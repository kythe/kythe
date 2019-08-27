#
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
#
load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain")

CxxExtractorToolchainInfo = provider(
    doc = "Provides required information for C++/ObjectiveC extractors.",
    fields = ["cc_toolchain", "extractor_binary", "system_includes"],
)

def _cxx_extractor_toolchain_impl(ctx):
    cc_toolchain = find_cpp_toolchain(ctx)
    if ctx.attr.include_cc_builtin_directories:
        system_includes = depset(cc_toolchain.built_in_include_directories)
    else:
        system_includes = depset()

    return [
        platform_common.ToolchainInfo(
            cxx_extractor_info = CxxExtractorToolchainInfo(
                extractor_binary = ctx.executable.extractor,
                system_includes = system_includes,
                cc_toolchain = cc_toolchain,
            ),
        ),
    ]

cxx_extractor_toolchain = rule(
    implementation = _cxx_extractor_toolchain_impl,
    attrs = {
        "extractor": attr.label(
            executable = True,
            default = Label("//kythe/cxx/extractor:cxx_extractor"),
            cfg = "host",
        ),
        "include_cc_builtin_directories": attr.bool(
            default = False,
        ),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    fragments = ["cpp"],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
)
