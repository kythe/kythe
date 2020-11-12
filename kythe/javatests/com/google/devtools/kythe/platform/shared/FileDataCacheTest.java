/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.platform.shared;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.util.concurrent.ListenableFuture;
import com.google.devtools.kythe.proto.Analysis;
import com.google.protobuf.ByteString;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
@SuppressWarnings("TestExceptionChecker")
public final class FileDataCacheTest {
  @Test(expected = IllegalArgumentException.class)
  public void testNullDigestLookupThrows() throws Throwable {
    List<Analysis.FileData> fileData = new ArrayList<>();

    Analysis.FileData.Builder fd1 = Analysis.FileData.newBuilder();
    fd1.getInfoBuilder().setDigest("digest1");
    fileData.add(fd1.build());
    Analysis.FileData.Builder fd2 = Analysis.FileData.newBuilder();
    fd2.getInfoBuilder().setDigest("anotherdigest");
    fileData.add(fd2.build());
    Analysis.FileData.Builder fd3 = Analysis.FileData.newBuilder();
    fd3.getInfoBuilder().setDigest("third");
    fileData.add(fd3.build());

    FileDataCache fileDataCache = new FileDataCache(fileData);

    ListenableFuture<byte[]> future = fileDataCache.startLookup(null, null);
    try {
      future.get();
    } catch (ExecutionException e) {
      throw e.getCause();
    }
  }

  @Test(expected = ExecutionException.class)
  public void testMissingDigestLookupThrows() throws ExecutionException, InterruptedException {
    List<Analysis.FileData> fileData = new ArrayList<>();

    Analysis.FileData.Builder fd1 = Analysis.FileData.newBuilder();
    fd1.getInfoBuilder().setDigest("digest1");
    fileData.add(fd1.build());
    Analysis.FileData.Builder fd2 = Analysis.FileData.newBuilder();
    fd2.getInfoBuilder().setDigest("anotherdigest");
    fileData.add(fd2.build());
    Analysis.FileData.Builder fd3 = Analysis.FileData.newBuilder();
    fd3.getInfoBuilder().setDigest("third");
    fileData.add(fd3.build());

    FileDataCache fileDataCache = new FileDataCache(fileData);

    ListenableFuture<byte[]> future = fileDataCache.startLookup(null, "missing");
    future.get();
  }

  @Test
  public void testCorrectDigestLookupReturnsMatch()
      throws ExecutionException, InterruptedException {
    List<Analysis.FileData> fileData = new ArrayList<>();

    Analysis.FileData.Builder fd1 = Analysis.FileData.newBuilder();
    fd1.getInfoBuilder().setDigest("digest1");
    fileData.add(fd1.build());
    Analysis.FileData.Builder fd2 = Analysis.FileData.newBuilder();
    fd2.getInfoBuilder().setDigest("anotherdigest");
    fileData.add(fd2.build());
    Analysis.FileData.Builder fd3 = Analysis.FileData.newBuilder();
    fd3.getInfoBuilder().setDigest("third");
    fd3.setContent(ByteString.copyFromUtf8("hello"));
    fileData.add(fd3.build());

    FileDataCache fileDataCache = new FileDataCache(fileData);

    ListenableFuture<byte[]> future = fileDataCache.startLookup(null, "third");
    byte[] bytes = future.get();
    assertThat(bytes).isEqualTo(ByteString.copyFromUtf8("hello").toByteArray());
  }

  @Test
  public void testPathIgnored() throws ExecutionException, InterruptedException {
    List<Analysis.FileData> fileData = new ArrayList<>();

    Analysis.FileData.Builder fd1 = Analysis.FileData.newBuilder();
    fd1.getInfoBuilder().setDigest("digest1");
    fileData.add(fd1.build());
    Analysis.FileData.Builder fd2 = Analysis.FileData.newBuilder();
    fd2.getInfoBuilder().setDigest("anotherdigest");
    fileData.add(fd2.build());
    Analysis.FileData.Builder fd3 = Analysis.FileData.newBuilder();
    fd3.getInfoBuilder().setDigest("third");
    fd3.setContent(ByteString.copyFromUtf8("hello"));
    fileData.add(fd3.build());

    FileDataCache fileDataCache = new FileDataCache(fileData);

    ListenableFuture<byte[]> fooFuture = fileDataCache.startLookup("foo", "third");
    byte[] fooBytes = fooFuture.get();
    assertThat(fooBytes).isEqualTo(ByteString.copyFromUtf8("hello").toByteArray());

    ListenableFuture<byte[]> barFuture = fileDataCache.startLookup("bar", "third");
    byte[] barBytes = barFuture.get();
    assertThat(barBytes).isEqualTo(ByteString.copyFromUtf8("hello").toByteArray());

    assertThat(barBytes).isEqualTo(fooBytes);
  }

  @Test
  public void testMultipleFilesWithTheSameDigestNotThrows()
      throws InterruptedException, ExecutionException {
    List<Analysis.FileData> fileData = new ArrayList<>();

    Analysis.FileData.Builder fd1 = Analysis.FileData.newBuilder();
    fd1.getInfoBuilder().setDigest("digest");
    fd1.setContent(ByteString.copyFromUtf8("first file"));
    fileData.add(fd1.build());
    Analysis.FileData.Builder fd2 = Analysis.FileData.newBuilder();
    fd2.getInfoBuilder().setDigest("digest");
    fd2.setContent(ByteString.copyFromUtf8("second file"));
    fileData.add(fd2.build());

    FileDataCache fileDataCache = new FileDataCache(fileData);

    ListenableFuture<byte[]> fooFuture = fileDataCache.startLookup("foo", "digest");
    byte[] fooBytes = fooFuture.get();
    assertThat(fooBytes).isEqualTo(ByteString.copyFromUtf8("first file").toByteArray());
  }
}
