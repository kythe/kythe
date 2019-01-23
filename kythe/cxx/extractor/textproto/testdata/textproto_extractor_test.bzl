"""Rules for testing the textproto extractor"""

# Copyright 2019 The Kythe Authors. All rights reserved.
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

load("//kythe/cxx/extractor/proto/testdata:proto_extractor_test.bzl", "extractor_golden_test")

def textproto_extractor_golden_test(**kwargs):
    """Alias for extractor_golden_test, with the textproto extractor swapped in.
    """
    extractor_golden_test(
        extractor = "//kythe/cxx/extractor/textproto:textproto_extractor",
        **kwargs
    )
