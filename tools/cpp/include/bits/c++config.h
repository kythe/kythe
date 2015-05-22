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

#include_next <bits/c++config.h>
// libstdc++-4.8 <stdio.h> does not define ::gets() if __cplusplus > 201103L
// but <cstdio> unconditionally imports it into namespace std, which fails.
// If _GLIBCXX_HAVE_GETS is undefined, however, <cstdio> will declare ::gets().
#if defined __cplusplus && __cplusplus > 201103L
#undef _GLIBCXX_HAVE_GETS
#endif
