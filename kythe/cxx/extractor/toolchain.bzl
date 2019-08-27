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
"""C++ Verifier toolchain support rules and macros."""

load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain")

CxxExtractorToolchainInfo = provider(
    doc = "Provides required information for C++/ObjectiveC extractors.",
    fields = ["cc_toolchain", "extractor_binary", "compiler_executable"],
)

CXX_EXTRACTOR_TOOLCHAINS = ["@io_kythe//kythe/cxx/extractor:toolchain_type"]

def _cxx_extractor_toolchain_impl(ctx):
    cc_toolchain = find_cpp_toolchain(ctx)
    if ctx.attr.compiler_executable:
        compiler_executable = ctx.attr.compiler_executable
    else:
        compiler_executable = cc_toolchain.compiler_executable
    cxx_extractor = CxxExtractorToolchainInfo(
        extractor_binary = ctx.attr.extractor.files_to_run,
        compiler_executable = compiler_executable,
        cc_toolchain = cc_toolchain,
    )
    return [
        platform_common.ToolchainInfo(
            cxx_extractor_info = cxx_extractor,
        ),
        cxx_extractor,
    ]

cxx_extractor_toolchain = rule(
    implementation = _cxx_extractor_toolchain_impl,
    attrs = {
        "extractor": attr.label(
            executable = True,
            default = Label("//kythe/cxx/extractor:cxx_extractor"),
            cfg = "host",
        ),
        "compiler_executable": attr.string(),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    provides = [CxxExtractorToolchainInfo, platform_common.ToolchainInfo],
    fragments = ["cpp"],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
)

def register_toolchains():
    native.register_toolchains(
        "@io_kythe//kythe/cxx/extractor:linux_toolchain",
        "@io_kythe//kythe/cxx/extractor:macos_toolchain",
    )

def find_extractor_toolchain(ctx):
    if "@io_kythe//kythe/cxx/extractor:toolchain_type" in ctx.toolchains:
        return ctx.toolchains["@io_kythe//kythe/cxx/extractor:toolchain_type"].cxx_extractor_info
    return ctx.attr._cxx_extractor_toolchain[CxxExtractorToolchainInfo]
