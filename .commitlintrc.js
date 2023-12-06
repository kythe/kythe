/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

module.exports = {
  // https://www.conventionalcommits.org/en/v1.0.0-beta.2/
  extends : [ '@commitlint/config-conventional' ],
  rules : {
    'scope-case' : [ 2, 'always', 'snake-case' ],
    'scope-empty' : [ 1, 'never' ],
    'scope-enum' :
                 [
                   1, 'always',
                   [
                     'api',        // user-facing API (e.g. XRefService)
                     'dev',        // developer tooling/scripting (e.g. linting)
                     'example',    // example tools or docs (e.g. sample web ui)
                     'extraction', // compilation extraction
                     'indexing',   // semantic analysis over compilations
                     'post_processing', // serving data construction
                     'schema',          // graph schema design
                     'serving',         // server logic or data formats
                     'tooling',         // CLI utilities (e.g. entrystream)

                     // Language-specific scopes
                     'cxx_common',
                     'cxx_extractor',
                     'cxx_indexer',
                     'go_common',
                     'go_extractor',
                     'go_indexer',
                     'java_common',
                     'java_extractor',
                     'java_indexer',
                     'jvm_common',
                     'jvm_extractor',
                     'jvm_indexer',
                     'rust_common',
                     'rust_extractor',
                     'rust_indexer',
                     'typescript_common',
                     'typescript_extractor',
                     'typescript_indexer',
                   ]
                 ],
    'type-enum' :
                [
                  2, 'always',
                  [
                    'build',
                    'chore',
                    'ci',
                    'deprecation',
                    'docs',
                    'feat',
                    'fix',
                    'perf',
                    'refactor',
                    'release',
                    'revert',
                    'style',
                    'test',
                  ]
                ],
  },
}
