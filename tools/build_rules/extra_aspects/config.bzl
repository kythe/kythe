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

"""Kythe extractor configuration rules and providers."""

load("@bazel_skylib//lib:sets.bzl", "sets")
load("@bazel_skylib//lib:structs.bzl", "structs")
load(":actions.bzl", _action_selection_config = "config")
load(
    ":extra_actions.bzl",
    "ActionListenerConfigInfo",
    "ActionListenerInfo",
    "ProtoWriterInfo",
    "action_listener_aspect",
    "extract_target",
)

KytheExtractorConfigInfo = provider(
    doc = "Configuration options for Kythe extraction.",
    fields = {
        "allowed_rule_kinds": "A Skylib set of allowed rule kinds (or None to allow everything).",
        "action_listeners": "A depset of ActionListenerInfo from the provided target.",
        "action_selection_config": "A KytheActionSelectionInfo instance.",
    },
)

def _use_default_proto_writer(action_listener, proto_writer):
    if not getattr(action_listener, "write_extra_action", None):
        keys = structs.to_dict(action_listener)
        keys["write_extra_action"] = proto_writer.write_extra_action
        return ActionListenerInfo(**keys)
    return action_listener

def _get_transitive_action_listeners(action_listeners, proto_writer):
    return depset(direct = [
        _use_default_proto_writer(t[ActionListenerInfo], proto_writer[ProtoWriterInfo])
        for t in action_listeners
        if ActionListenerInfo in t
    ], transitive = [
        t[ActionListenerConfigInfo].action_listeners
        for t in action_listeners
        if ActionListenerConfigInfo in t
    ])

def _config_impl(ctx):
    action_listeners = _get_transitive_action_listeners(
        ctx.attr.action_listeners,
        ctx.attr.proto_writer,
    ).to_list()

    # TODO(b/182403136): Workaround for error applying aspects to jsunit_test rules.
    # They complain about a "non-executable dependency" so we provide a do-nothing executable
    # file to silence this.
    dummy_executable = ctx.actions.declare_file(ctx.label.name + ".dummy_exec")
    ctx.actions.write(
        output = dummy_executable,
        content = "",
        is_executable = True,
    )

    return [
        KytheExtractorConfigInfo(
            action_listeners = action_listeners,
            allowed_rule_kinds = _allowed_rule_kinds(ctx.attr.rules, ctx.attr.aspect_rule_attrs),
            action_selection_config = _action_selection_config(
                rules = sets.make(ctx.attr.rules),
                aspect_rule_attrs = ctx.attr.aspect_rule_attrs,
                mnemonics = sets.union(*[a.mnemonics for a in action_listeners]),
                exclude_input_extensions = sets.make(ctx.attr.exclude_input_extensions),
            ),
        ),
        DefaultInfo(
            executable = dummy_executable,
        ),
    ]

kythe_extractor_config = rule(
    implementation = _config_impl,
    attrs = {
        "rules": attr.string_list(mandatory = True),
        "aspect_rule_attrs": attr.string_list_dict(),
        "action_listeners": attr.label_list(
            aspects = [action_listener_aspect],
            providers = [
                [ActionListenerInfo],
                [ActionListenerConfigInfo],
            ],
            allow_rules = ["action_listener"],
        ),
        "proto_writer": attr.label(
            default = Label("//tools/build_rules/extra_aspects:spawn-info-writer"),
            providers = [ProtoWriterInfo],
        ),
        "exclude_input_extensions": attr.string_list(),
    },
    provides = [KytheExtractorConfigInfo],
)

def _action_listener_config_impl(ctx):
    return [
        ActionListenerConfigInfo(
            action_listeners = _get_transitive_action_listeners(ctx.attr.action_listeners, ctx.attr.proto_writer),
        ),
    ]

action_listener_config = rule(
    implementation = _action_listener_config_impl,
    attrs = {
        "action_listeners": attr.label_list(
            aspects = [action_listener_aspect],
            providers = [
                [ActionListenerInfo],
                [ActionListenerConfigInfo],
            ],
            allow_rules = ["action_listener"],
        ),
        "proto_writer": attr.label(
            mandatory = True,
            providers = [ProtoWriterInfo],
        ),
    },
    provides = [ActionListenerConfigInfo],
)

def _allowed_rule_kinds(rules, aspect_rule_attrs):
    if not rules:
        return None  # An empty `rules` attribute means allow all kinds.
    return sets.union(sets.make(rules), sets.make(aspect_rule_attrs.keys()))

def run_configured_extractor(target, ctx, config):
    # If there is an explicit allowlist of rule kinds, only extract those.
    # Attempting to extract an unknown rule kind is not an error.
    # As command-line aspects are called for all top-level targets,
    # they must be able to quietly handle this case.
    if config.allowed_rule_kinds and not sets.contains(config.allowed_rule_kinds, ctx.rule.kind):
        return depset()
    return extract_target(target, ctx, config)
