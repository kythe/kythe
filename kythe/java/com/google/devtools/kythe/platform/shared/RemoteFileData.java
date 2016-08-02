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

package com.google.devtools.kythe.platform.shared;

import com.google.common.net.HostAndPort;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.SettableFuture;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Analysis.FilesRequest;
import com.google.devtools.kythe.proto.FileDataServiceGrpc;
import com.google.devtools.kythe.proto.FileDataServiceGrpc.FileDataServiceStub;
import io.grpc.ManagedChannel;
import io.grpc.netty.NegotiationType;
import io.grpc.netty.NettyChannelBuilder;
import io.grpc.stub.StreamObserver;
import java.net.InetSocketAddress;

/** {@link FileDataProvider} backed by an external {@link FileDataServiceGrpc} service. */
public class RemoteFileData implements FileDataProvider {
  private final FileDataServiceStub stub;

  public RemoteFileData(String addr) {
    HostAndPort hp = HostAndPort.fromString(addr);
    ManagedChannel channel =
        NettyChannelBuilder.forAddress(new InetSocketAddress(hp.getHostText(), hp.getPort()))
            .negotiationType(NegotiationType.PLAINTEXT)
            .build();
    stub = FileDataServiceGrpc.newStub(channel);
  }

  @Override
  public ListenableFuture<byte[]> startLookup(String path, String digest) {
    SettableFuture<byte[]> future = SettableFuture.create();

    FilesRequest.Builder req = FilesRequest.newBuilder();
    req.addFilesBuilder().setPath(path).setDigest(digest);
    stub.get(req.build(), new SingletonLookup(future));
    return future;
  }

  @Override
  public void close() {}

  private static class SingletonLookup implements StreamObserver<FileData> {
    private final SettableFuture<byte[]> future;

    public SingletonLookup(SettableFuture<byte[]> future) {
      this.future = future;
    }

    @Override
    public void onNext(FileData data) {
      if (data.getMissing()) {
        throw new IllegalStateException("no FileData returned");
      }
      future.set(data.getContent().toByteArray());
    }

    @Override
    public void onError(Throwable t) {
      future.setException(t);
    }

    @Override
    public void onCompleted() {
      if (!future.isDone()) {
        future.setException(new IllegalStateException("no FileData returned"));
      }
    }
  }
}
