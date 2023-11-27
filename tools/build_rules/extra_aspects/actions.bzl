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

"""Starlark utilities for selecting actions."""

load("@bazel_skylib//lib:sets.bzl", "sets")

def select_target_actions(target, rule, config):
    """Given a target and a rule, finds actions matching the specified config.

    Args:
      target: The Target object provided to an aspect implementation.
      rule: The rule_attributes from an aspect's ctx.rule parameter.
      config: A KytheActionSelectionInfo instance, configuring action selection.

    Returns:
      A list of actions selected by the provided configuration.
    """
    actions = []
    if sets.length(config.rules) == 0 or sets.contains(config.rules, rule.kind):
        actions.extend(_filter_actions(target.actions, config.mnemonics, config.exclude_input_extensions))
    for attr in config.aspect_rule_attrs.get(rule.kind, []):
        for dep in getattr(rule.attr, attr, []):
            # {lang}_proto_library targets are implemented as aspects
            # with a single "dep" pointing at original proto_library target.
            # All of the actions come from the aspect as applied to the dep,
            # which is not a top-level dep, so we need to inspect those as well.
            # TODO(shahms): It would be nice if there was some way to tell whether or
            # not an action came from an aspect or the underlying rule. We could then
            # more generally inspect deps to find actions rather than limiting it to
            # allowlisted kinds.
            # This could be accomplished by having `aspect_ids` populated on deps.
            # We could look specifically for "<merged target" in the string-ified dep,
            # but that's pretty ugly.
            # It looks like adding an "aspect_ids" list attribute should be straightforward.
            # https://github.com/bazelbuild/bazel/blob/master/src/main/java/com/google/devtools/build/lib/analysis/configuredtargets/MergedConfiguredTarget.java#L54
            # It could also just use ActionOwner information.
            if str(dep).startswith("<merged target") and hasattr(dep, "actions"):
                actions.extend(_filter_actions(dep.actions, config.mnemonics, config.exclude_input_extensions))
    return actions

def config(rules, aspect_rule_attrs, mnemonics, exclude_input_extensions):
    """Create an action selection config.

    Args:
     rules: A Skylib set of rule kinds from which to select actions.
     aspect_rule_attrs: A dict of {kind: [attrs]} of rule kind attributes from which to select actions.
     mnemonics: A Skylib set of action mnemonics to select.
     exclude_input_extensions: A Skylib set of file extensions to whose actions to exclude.

    Returns:
      A struct with corresponding attributes.
    """
    return struct(
        rules = rules,
        aspect_rule_attrs = aspect_rule_attrs,
        mnemonics = mnemonics,
        exclude_input_extensions = exclude_input_extensions,
    )

def _has_extension(inputs, extensions):
    if not extensions or sets.length(extensions) == 0:
        return False
    for input in inputs.to_list():
        if sets.contains(extensions, input.extension):
            return True
    return False

def _filter_actions(actions, mnemonics, exclude_extensions = None):
    return [
        a
        for a in actions
        if sets.contains(mnemonics, a.mnemonic) and
           not _has_extension(a.inputs, exclude_extensions)
    ]
