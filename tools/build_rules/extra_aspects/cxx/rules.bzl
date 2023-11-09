# Copyright 2023 The Kythe Authors. All rights reserved.
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

"""C++ extraction rules."""

load(":aspect.bzl", "KytheCxxCompilationInfo", "extract_cxx_aspect")

_EXTRACTION_SETTINGS = {
    # We don't want and can't use module maps for extraction/indexing.
    "//command_line_option:features": ["-use_module_maps", "-module_maps"],
}

def _cc_extract_kzip_impl(ctx):
    return [
        DefaultInfo(files = depset(transitive = [
            d[KytheCxxCompilationInfo].kzips
            for d in ctx.attr.deps
            if KytheCxxCompilationInfo in d and d[KytheCxxCompilationInfo].kzips
        ])),
    ]

def _transition_impl(settings, attr):
    if not attr.disable_modules:
        return None
    return _EXTRACTION_SETTINGS

_transition = transition(
    implementation = _transition_impl,
    inputs = [],
    outputs = _EXTRACTION_SETTINGS.keys(),
)

cc_extract_kzip = rule(
    implementation = _cc_extract_kzip_impl,
    attrs = {
        "deps": attr.label_list(
            mandatory = True,
            aspects = [extract_cxx_aspect],
        ),
        "disable_modules": attr.bool(
            default = True,
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    cfg = _transition,
)
