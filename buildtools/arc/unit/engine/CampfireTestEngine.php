<?php

/**
 * campfire wrapper.
 */
final class CampfireTestEngine extends ArcanistUnitTestEngine {
  public function run() {
    $this->project_root = $this->getWorkingCopy()->getProjectRoot();

    $results = array();
    $targets = $this->getTargets();
    foreach ($targets as $target) {
      $results[] = $this->runTest($target);
    }

    return $results;
  }

  protected function supportsRunAllTests() {
    return true;
  }

  private function getTargets() {
    if ($this->getRunAllTests()) {
      return array('//...');
    }

    $query_command = "./campfire query --require_lock_immediately --print_names %s";
    $files = json_encode($this->getExistingPaths());
    $future = new ExecFuture($query_command, 'dependsOn('.$files.')');
    $future->setCWD($this->project_root);
    $output = trim($future->resolvex()[0]);
    if ($output === "") {
      return array();
    }

    return explode("\n", $output);
  }

  private function getExistingPaths() {
    return array_values(array_filter($this->getPaths(), "file_exists"));
  }

  private function runTest($target) {
    $future = new ExecFuture('./campfire test --require_lock_immediately %s', $target);
    $future->setCWD($this->project_root);
    $status = $future->resolve();
    return $this->parseTestResult($target, $status);
  }

  private function parseTestResult($target, $status) {
    $code = $status[0];
    $output = $status[1];

    $result = new ArcanistUnitTestResult();
    $result->setName($target);
    if ($code == 0) {
      $result->setResult(ArcanistUnitTestResult::RESULT_PASS);
    } else {
      $result->setResult(strpos($output, 'Test failure code:') !== false
          ? ArcanistUnitTestResult::RESULT_FAIL
          : ArcanistUnitTestResult::RESULT_BROKEN);
      $result->setUserData($output);
    }
    return $result;
  }
}