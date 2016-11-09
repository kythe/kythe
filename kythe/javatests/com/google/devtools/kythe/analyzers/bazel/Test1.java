package com.google.devtools.kythe.analyzers.bazel;


import static org.junit.Assert.*;

import com.google.devtools.build.lib.vfs.FileSystem;
import com.google.devtools.build.lib.vfs.Path;
import com.google.devtools.build.lib.vfs.UnixFileSystem;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class Test1 {

  protected FileSystem getFreshFileSystem() {
    return new UnixFileSystem();
  }

  @Test
  public void testFileNotExists() throws Exception {
    FileSystem unixFileSystem = getFreshFileSystem();
    //Path nonExistantFilePath = absolutize("file_does_not_exists.foo");
    Path nonExistantFilePath = unixFileSystem.getPath("/usr/local/google/home/craigbarber/.cache/bazel/_bazel_craigbarber/e3e9059079267da4a6f5d33cdd0e5a2f/bazel-sandbox/b9299431-f65a-47e0-9ff1-4dd93f0624a7-1/execroot/kythe/_tmp/tickets_test_1/junit905983271379793721/kythe/foo/NonExistingFile");
    assertFalse(nonExistantFilePath.isFile());
  }
}