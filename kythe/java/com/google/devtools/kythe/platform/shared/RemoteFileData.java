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
import com.google.common.util.concurrent.SettableFuture;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.FileDataServiceGrpc;
import com.google.devtools.kythe.proto.FileDataServiceGrpc.FileDataServiceStub;

import io.grpc.ChannelImpl;
import io.grpc.stub.StreamObserver;
import io.grpc.transport.netty.NettyChannelBuilder;

import java.net.InetSocketAddress;
import java.util.concurrent.Future;

/** {@link FileDataProvider} backed by an external {@link FileDataServiceGrpc.FileDataService}. */
public class RemoteFileData implements FileDataProvider {
  private final FileDataServiceStub stub;

  public RemoteFileData(String addr) {
    HostAndPort hp = HostAndPort.fromString(addr);
    ChannelImpl channel = NettyChannelBuilder.forAddress(new InetSocketAddress(hp.getHostText(), hp.getPort()))
        .build();
    stub = FileDataServiceGrpc.newStub(channel);
  }

  @Override
  public Future<byte[]> startLookup(String path, String digest) {
    SettableFuture<byte[]> future = SettableFuture.create();
    stub.get(new SingletonLookup(future))
        .onValue(FileInfo.newBuilder()
            .setPath(path)
            .setDigest(digest)
            .build());
    return future;
  }

  private static class SingletonLookup implements StreamObserver<FileData> {
    private final SettableFuture<byte[]> future;

    public SingletonLookup(SettableFuture<byte[]> future) {
      this.future = future;
    }

    @Override
    public void onValue(FileData data) {
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
