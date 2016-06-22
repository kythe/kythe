<?php
/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * bazel wrapper.
 */
final class BazelTestEngine extends ArcanistUnitTestEngine {
  private $debug;
  private $useConfig;
  private $waitForBazel;

  private $project_root;

  public function run() {
    if (getenv("DEBUG")) {
      $this->debug = true;
    }
    $this->debugPrint("run");
    if (getenv("USE_BAZELRC")) {
      $this->useConfig = true;
      print("WARNING: using default bazelrc\n");
    }
    if (getenv("WAIT_FOR_BAZEL")) {
      $this->waitForBazel = true;
    }

    $this->project_root = $this->getWorkingCopy()->getProjectRoot();
    $targets = $this->getTargets();
    if (empty($targets)) {
      return array();
    }
    $targets[] = "//kythe/go/util/tools:print_test_status";
    return $this->runTests($targets);
  }

  protected function supportsRunAllTests() {
    return true;
  }

  private function getTargets() {
    $this->debugPrint("getTargets()");

    if ($this->getRunAllTests()) {
      return array('//...');
    }

    $files = $this->getFileTargets();
    if (empty($files)) {
      print("No files affected\n");
      return array();
    }
    $files = join($files, " ");
    $this->debugPrint("files: " . $files);

    $cmd = $this->bazelCommand("query", ["%s"]);
    $query = 'rdeps(//..., set('.$files.')) except attr(tags, "docker|arc-ignore", //...)';
    $this->debugPrint($query);
    $future = new ExecFuture($cmd, $query);
    $future->setCWD($this->project_root);
    $status = $future->resolve();
    if ($status[0] != 3 && $status[0] != 0) {
      throw new Exception("Bazel query error (".$status[0]."): ".$status[2]);
    }
    $output = trim($status[1]);
    if ($output === "") {
      print("No targets affected\n");
      return array();
    }
    return explode("\n", $output);
  }

  private function getFileTargets() {
    if (empty($this->getPaths())) {
      return array();
    }
    $files = join(" ",
                  array_map(array('BazelTestEngine', 'fileToTarget'), $this->getPaths()));
    $future = new ExecFuture($this->bazelCommand("query", ["-k", "%s"]), 'set('.$files.')');
    $future->setCWD($this->project_root);

    $status = $future->resolve();
    if ($status[0] != 3 && $status[0] != 0) {
      throw new Exception("Bazel query error (".$status[0]."): ".$status[2]);
    }

    $output = trim($status[1]);
    if ($output === "") {
      return array();
    }

    return explode("\n", $output);
  }

  private function runTests($targets) {
    $this->debugPrint("runTests(" . join($targets, ", ") . ")");

    $future = new ExecFuture($this->bazelCommand("test", array_merge([
        "--verbose_failures",
        "--test_tag_filters=-broken",
        "--noshow_loading_progress",
        "--noshow_progress"], $targets)));
    $future->setCWD($this->project_root);
    $status = $future->resolve();
    return $this->parseTestResults($targets, $status);
  }

  private function parseTestResults($targets, $status) {
    $this->debugPrint("parseTestResults");

    $code = $status[0];
    $output = $status[1];
    $lines = explode("\n", $output);

    if ($code == 4) {
      print("No tests affected...\n");
      return [];
    } else if ($code == 1) {
      throw new Exception($output . "\n" . $status[2]);
    }

    $cmd = $this->bazelCommand("query", ["-k", "%s"]);
    $query = 'tests(set('.join(" ", $targets).')) except attr(tags, "broken", //...)';
    $this->debugPrint($query);
    $future = new ExecFuture($cmd, $query);
    $future->setCWD($this->project_root);
    $testTargets = explode("\n", trim($future->resolvex()[0]));

    $results = array();
    foreach ($testTargets as $test) {
      $data = $this->parseTestResultFile($test);
      $result = new ArcanistUnitTestResult();
      $result->setName($test);
      if (property_exists($data, "test_case")) {
        $testCase = $data->{"test_case"};
        if (property_exists($testCase, "run_duration_millis")) {
          $result->setDuration($testCase->{"run_duration_millis"} / 1000);
        }
      }
      if ($data->{"test_passed"}) {
        $result->setResult(ArcanistUnitTestResult::RESULT_PASS);
      } else if ($data->{"status"} == 4) {
        $result->setResult(ArcanistUnitTestResult::RESULT_FAIL);
      } else {
        $result->setResult(ArcanistUnitTestResult::RESULT_BROKEN);
      }

      $results[] = $result;
    }

    return $results;
  }

  private function parseTestResultFile($target) {
    $path = "bazel-testlogs/".str_replace(":", "/", substr($target, 2))."/test.cache_status";
    $future = new ExecFuture("bazel-bin/kythe/go/util/tools/print_test_status %s", $path);
    $future->setCWD($this->project_root);
    return json_decode($future->resolvex()[0]);
  }

  private static function fileToTarget($file) {
    if (dirname($file) == ".") {
      return '//:' . $file;
    }
    return "'" . $file . "'";
  }

  private function bazelCommand($subcommand, $args) {
    $cmd = "bazel ";
    if (!$this->useConfig) {
      $cmd = $cmd . "--bazelrc=/dev/null ";
    }
    if (!$this->waitForBazel) {
      $cmd = $cmd . "--noblock_for_lock ";
    }
    $cmd = $cmd . $subcommand . " --tool_tag=arcanist ";
    $cmd = $cmd . join(" ", $args);
    $this->debugPrint($cmd);
    return $cmd;
  }

  private function debugPrint($msg) {
    if ($this->debug) {
      print("DEBUG: " . $msg . "\n");
    }
  }
}
