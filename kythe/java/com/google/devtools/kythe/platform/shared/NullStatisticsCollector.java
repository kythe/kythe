/*
 * Copyright 2014 Google Inc. All rights reserved.
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

import java.io.Serializable;

/** {@link StatisticsCollector} that ignores all statistics. */
public class NullStatisticsCollector implements StatisticsCollector, Serializable {
  private static final long serialVersionUID = 7642617128987532613L;

  private static final NullStatisticsCollector instance = new NullStatisticsCollector();

  /** Returns the single instance of the {@link StatisticsCollector} that ignores all statistics. */
  public static NullStatisticsCollector getInstance() {
    return instance;
  }

  private NullStatisticsCollector() {}

  @Override
  public void incrementCounter(String name) {}

  @Override
  public void incrementCounter(String name, int amount) {}
}
