#
# Copyright 2016 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Bazel rules to extract Go compilations from library targets for testing the
# Go cross-reference indexer.

# A convenience macro to generate a test library, pass it to the Go indexer,
# and feed the output of indexing to the Kythe schema verifier.
def go_indexer_test(name, srcs, deps=[], import_path='', size = 'small',
                    log_entries=False, data=None,
                    has_marked_source=False,
                    allow_duplicates=False,
                    metadata_suffix=''):
  pass

# A convenience macro to generate a test library, pass it to the Go indexer,
# and feed the output of indexing to the Kythe integration test pipeline.
def go_integration_test(name, srcs, deps=[], data=None,
                        file_tickets=[],
                        import_path='', size='small',
                        has_marked_source=False,
                        metadata_suffix=''):
  pass
