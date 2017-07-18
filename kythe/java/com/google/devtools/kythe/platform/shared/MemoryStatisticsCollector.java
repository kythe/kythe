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

import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/** {@link StatisticsCollector} backed by an in-memory map that can be printed. */
public class MemoryStatisticsCollector implements StatisticsCollector {
  private final Map<String, AtomicLong> counters = new HashMap<>();

  /** Prints the current statistics to the given {@link PrintStream}. */
  public synchronized void printStatistics(PrintStream out) {
    for (Map.Entry<String, AtomicLong> entry : counters.entrySet()) {
      out.printf("%s:\t%d\n", entry.getKey(), entry.getValue().get());
    }
  }

  /** Returns the value of the given counter. */
  public long getValue(String name) {
    return getCounter(name).get();
  }

  @Override
  public void incrementCounter(String name) {
    getCounter(name).getAndIncrement();
  }

  @Override
  public void incrementCounter(String name, int amount) {
    getCounter(name).getAndAdd(amount);
  }

  private synchronized AtomicLong getCounter(String name) {
    if (counters.containsKey(name)) {
      return counters.get(name);
    } else {
      AtomicLong counter = new AtomicLong();
      counters.put(name, counter);
      return counter;
    }
  }
}
