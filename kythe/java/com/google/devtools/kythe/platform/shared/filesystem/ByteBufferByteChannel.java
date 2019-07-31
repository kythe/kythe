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

/** Read-only {@link SeekableByteChannel} backed by byte[] data. */
class ByteBufferByteChannel implements SeekableByteChannel {
  private byte[] buffer;
  private long position;

  public ByteBufferByteChannel(byte[] data) {
    buffer = data;
    position = 0;
  }

  @Override
  public boolean isOpen() {
    return buffer != null;
  }

  @Override
  public void close() throws IOException {
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
    dst.put(buffer, (int) position, wanted);
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
    return buffer.length;
  }

  @Override
  public SeekableByteChannel truncate(long size) throws IOException {
    throw new NonWritableChannelException();
  }
}
