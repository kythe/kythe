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

"""C++ extraction aspect definition."""

load("@rules_proto//proto:defs.bzl", "ProtoInfo")
load(
    "//tools/build_rules/extra_aspects:config.bzl",
    "KytheExtractorConfigInfo",
    "run_configured_extractor",
)

KytheCxxCompilationInfo = provider("C++ KytheCompilationInfo", fields = ["kzips"])

_KytheCxxProtoCompilationInfo = provider("", fields = ["kzips"])

_COMMON_ATTRS = {
    "_config": attr.label(
        providers = [KytheExtractorConfigInfo],
        default = Label("//tools/build_rules/extra_aspects/cxx:extractor-config"),
    ),
}

def _transitive_kzips(ctx, provider = _KytheCxxProtoCompilationInfo):
    return [
        dep[provider].kzips
        for dep in ctx.rule.attr.deps
        if provider in dep and dep[provider].kzips
    ]

def _extract_target(target, ctx, transitive):
    config = ctx.attr._config[KytheExtractorConfigInfo]
    kzips = [run_configured_extractor(target, ctx, config)]
    if transitive:
        kzips += _transitive_kzips(ctx)
    return depset(transitive = kzips)

def _extract_proto_impl(target, ctx):
    return [_KytheCxxProtoCompilationInfo(kzips = _extract_target(
        target,
        ctx,
        transitive = True,
    ))]

_extract_proto_aspect = aspect(
    implementation = _extract_proto_impl,
    attr_aspects = ["deps"],
    # Only call the aspect on proto_library-like targets.
    required_providers = [ProtoInfo],
    # We need the actions provided by the aspect, rather than the provider itself,
    # but CcProtoInfo aspect isn't exported from the rules.
    required_aspect_providers = [[CcInfo]],
    attrs = _COMMON_ATTRS,
)

def _extract_impl(target, ctx):
    config = ctx.attr._config[KytheExtractorConfigInfo].action_selection_config
    return [KytheCxxCompilationInfo(kzips = _extract_target(
        target,
        ctx,
        transitive = [] == config.aspect_rule_attrs.get(ctx.rule.kind),
    ))]

extract_cxx_aspect = aspect(
    implementation = _extract_impl,
    requires = [_extract_proto_aspect],
    attrs = _COMMON_ATTRS,
    provides = [KytheCxxCompilationInfo],
)
