/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.platform.shared.filesystem;

import static com.google.common.base.Preconditions.checkArgument;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.NonWritableChannelException;
import java.nio.channels.SeekableByteChannel;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

/** Read-only {@link SeekableByteChannel} lazily backed by byte[] data. */
class ByteBufferByteChannel implements SeekableByteChannel {
  private Future<byte[]> buffer;
  private long position;

  public ByteBufferByteChannel(Future<byte[]> data) {
    buffer = data;
    position = 0;
  }

  @Override
  public boolean isOpen() {
    return buffer != null;
  }

  @Override
  public void close() throws IOException {
    if (buffer != null) {
      buffer.cancel(true);
    }
    buffer = null;
  }

  @Override
  public int read(ByteBuffer dst) throws IOException {
    if (!isOpen()) {
      throw new ClosedChannelException();
    }
    int wanted = dst.remaining();
    long possible = size() - position;
    if (possible <= 0) {
      return -1;
    }
    wanted = (int) Math.min(possible, wanted);
    try {
      dst.put(buffer.get(), (int) position, wanted);
    } catch (InterruptedException | ExecutionException exc) {
      throw new IOException(exc);
    }
    position += wanted;
    return wanted;
  }

  @Override
  public int write(ByteBuffer src) throws IOException {
    throw new NonWritableChannelException();
  }

  @Override
  public long position() throws IOException {
    return position;
  }

  @Override
  public SeekableByteChannel position(long newPosition) throws IOException {
    checkArgument(newPosition >= 0);
    position = newPosition;
    return this;
  }

  @Override
  public long size() throws IOException {
    try {
      return buffer.get().length;
    } catch (InterruptedException | ExecutionException exc) {
      throw new IOException(exc);
    }
  }

  @Override
  public SeekableByteChannel truncate(long size) throws IOException {
    throw new NonWritableChannelException();
  }
}
