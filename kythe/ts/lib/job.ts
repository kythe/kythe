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

import kythe = require('./kythe');
import ts = require('typescript');

export interface Job {
  // Files required for compliation that we are indexing.
  files : string[];
  // Files required for compilation but that we aren't indexing.
  implicits : string[];
  lib : string;
  options : ts.CompilerOptions;
  dontIndexDefaultLib : boolean;
  // Maps path names to VNames.
  classifier : kythe.PathClassifier;
}
