#
# Copyright 2018 Google Inc. All rights reserved.
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

def find_cpp_toolchain(ctx):
    """Returns the appropriate C++ toolchain.

    If the c++ toolchain is in use, returns it.  Otherwise, returns a c++
    toolchain derived from legacy toolchain selection.

    Args:
      ctx: The rule context for which to find a toolchain.

    Returns:
      A CcToolchainProvider.
    """
    if Label("@bazel_tools//tools/cpp:toolchain_type") in ctx.fragments.platform.enabled_toolchain_types:
        return ctx.toolchains["@bazel_tools//tools/cpp:toolchain_type"]
    else:
        return ctx.attr._cc_toolchain[cc_common.CcToolchainInfo]
