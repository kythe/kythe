/*
 * Copyright 2022 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util;

import com.google.common.collect.Streams;
import java.util.Comparator;
import java.util.Optional;
import java.util.ServiceLoader;

/**
 * OrderedCompatibilityService defines shared functionality between between JdkCompatibilityShim
 * interfaces.
 */
public interface OrderedCompatibilityService {

  /** The compatibility level of the provider. */
  public enum CompatibilityLevel {
    /** This provider is incompatible with the current runtime. */
    INCOMPATIBLE,
    /**
     * This provider is compatible with the current runtime, but should only be used if a COMPATIBLE
     * provider can't be found.
     */
    FALLBACK,
    /**
     * This provider is compatible with the current runtime and should be preferred over a fallback,
     * if any.
     */
    COMPATIBLE
  }

  /** Indicates the range of runtime versions with which these shims are compatible. */
  public static class CompatibilityRange {
    private final Runtime.Version minVersion;
    private final Optional<Runtime.Version> maxVersion;
    private final CompatibilityLevel level;

    public CompatibilityRange(Runtime.Version minVersion) {
      this(minVersion, Optional.empty(), CompatibilityLevel.COMPATIBLE);
    }

    public CompatibilityRange(Runtime.Version minVersion, Runtime.Version maxVersion) {
      this(minVersion, Optional.of(maxVersion), CompatibilityLevel.COMPATIBLE);
    }

    public CompatibilityRange(
        Runtime.Version minVersion,
        Optional<Runtime.Version> maxVersion,
        CompatibilityLevel level) {
      this.minVersion = minVersion;
      this.maxVersion = maxVersion;
      this.level = level;
    }

    /** Returns the minimum known compatible runtime version. */
    public Runtime.Version getMinVersion() {
      return minVersion;
    }

    /** Returns the maximum known compatible runtime version, if any. */
    public Optional<Runtime.Version> getMaxVersion() {
      return maxVersion;
    }

    /** Returns the compatibility level. */
    public CompatibilityLevel getCompatibilityLevel() {
      return level;
    }
  }

  /** Returns the compatibility level of the service provider. */
  CompatibilityRange getCompatibleRange();

  public static <S extends OrderedCompatibilityService> Optional<S> loadBest(Class<S> klass) {
    Runtime.Version version = Runtime.version();
    return Streams.stream(ServiceLoader.load(klass))
        // Filter out incompatible providers.
        .filter(p -> isCompatible(version, p.getCompatibleRange()))
        // Then find the best.
        .max(OrderedCompatibilityService::compareProviders);
  }

  private static boolean isCompatible(Runtime.Version version, CompatibilityRange range) {
    return range.getCompatibilityLevel() != CompatibilityLevel.INCOMPATIBLE
        && version.compareToIgnoreOptional(range.getMinVersion()) >= 0
        && range.getMaxVersion().map(max -> version.compareToIgnoreOptional(max) < 0).orElse(true);
  }

  private static int compareCompatibilityRanges(CompatibilityRange lhs, CompatibilityRange rhs) {
    return Comparator.comparing(CompatibilityRange::getCompatibilityLevel)
        .thenComparing(CompatibilityRange::getMinVersion, Runtime.Version::compareToIgnoreOptional)
        .compare(lhs, rhs);
  }

  private static <S extends OrderedCompatibilityService> int compareProviders(S lhs, S rhs) {
    return Comparator.comparing(
            OrderedCompatibilityService::getCompatibleRange,
            OrderedCompatibilityService::compareCompatibilityRanges)
        .compare(lhs, rhs);
  }
}
