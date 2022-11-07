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

import java.util.Optional;
import java.util.ServiceLoader;

/**
 * OrderedCompatibilityService defines shared functionality between between JdkCompatibilityShim
 * interfaces.
 */
public interface OrderedCompatibilityService {

  /** The compatibilty level of the provider. */
  public enum CompatibilityClass {
    /** This provider is incompatible with the current runtime. */
    INCOMPATIBLE,
    /**
     * This provider is compatible with the current runtime and should be preferred over a fallback,
     * if any.
     */
    COMPATIBLE,
    /**
     * This provider is compatible with the current runtime, but should only be used if a COMPATIBLE
     * provider can't be found.
     */
    FALLBACK
  };

  /** Returns the compatibility level of the service provider. */
  CompatibilityClass getCompatibility();

  public static <S extends OrderedCompatibilityService> Optional<S> loadBest(Class<S> klass) {
    S fallback = null;
    for (S provider : ServiceLoader.load(klass)) {
      if (provider == null) continue;
      switch (provider.getCompatibility()) {
        case INCOMPATIBLE:
          continue;
        case COMPATIBLE:
          // Use the first compatible provider, if any.
          return Optional.of(provider);
        case FALLBACK:
          if (fallback == null) fallback = provider;
          break;
      }
    }
    return Optional.ofNullable(fallback);
  }
}
