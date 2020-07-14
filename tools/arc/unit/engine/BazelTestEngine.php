<?php
/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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
  private static $omit_tags = ["manual", "broken", "arc-ignore", "docker"];
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
    $targets[] = "//kythe/go/util/tools/print_test_status";
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
    // Quote each file to make it safe in case it has special characters in it.
    $files = array_map(function($s) { return '"'.$s.'"'; }, $files);
    $files = join($files, " ");
    $this->debugPrint("files: " . $files);

    $cmd = $this->bazelCommand("query", ["-k", "%s"]);
    $tag_filter = join("|", self::$omit_tags);
    $query = 'rdeps(//..., set('.$files.')) except attr(tags, "'.$tag_filter.'", //...)';
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
    // Quote each target to make it safe in case it has special characters in it.
    return array_map(function($s) { return '"'.$s.'"'; }, explode("\n", $output));
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

    $tag_filters = join(",", array_map(function($s) { return "-$s"; }, self::$omit_tags));
    $future = new ExecFuture($this->bazelCommand("test", array_merge([
        "--config=prepush",
        "--verbose_failures",
        "--test_tag_filters=$tag_filters",
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
    $tag_filter = join("|", self::$omit_tags);
    $query = 'tests(set('.join(" ", $targets).'))';
    $this->debugPrint($query);
    $future = new ExecFuture($cmd, $query);
    $future->setCWD($this->project_root);
    $testTargets = explode("\n", trim($future->resolvex()[0]));

    $results = array();
    foreach ($testTargets as $test) {
      $result = new ArcanistUnitTestResult();
      $result->setName($test);
      try {
        $data = $this->parseTestResultFile($test);
        if (array_key_exists("test_case", $data)) {
          $testCase = $data["test_case"];
          if (array_key_exists("run_duration_millis", $testCase)) {
            $result->setDuration($testCase["run_duration_millis"] / 1000);
          }
        }
        if ($data["test_passed"]) {
          $result->setResult(ArcanistUnitTestResult::RESULT_PASS);
        } else if ($data["status"] == 4) {
          $result->setResult(ArcanistUnitTestResult::RESULT_FAIL);
        } else {
          $result->setResult(ArcanistUnitTestResult::RESULT_BROKEN);
        }
      } catch (CommandException $exc) {
        if ($code == 0) {
          // bazel test exited successfully, therefore these are
          // skipped tests, rather than broken.
          $result->setResult(ArcanistUnitTestResult::RESULT_SKIP);
        } else {
          $result->setResult(ArcanistUnitTestResult::RESULT_BROKEN);
        }
      }

      $results[] = $result;
    }

    return $results;
  }

  private function parseTestResultFile($target) {
    $path = "bazel-testlogs/".str_replace(":", "/", substr($target, 2))."/test.cache_status";
    $future = new ExecFuture("bazel-bin/kythe/go/util/tools/print_test_status/print_test_status %s", $path);
    $future->setCWD($this->project_root);
    return $future->resolveJSON();
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
