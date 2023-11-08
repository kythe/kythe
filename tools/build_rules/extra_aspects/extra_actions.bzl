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

"""Aspect-based re-implementation of extra_actions."""

load("@bazel_skylib//lib:sets.bzl", "sets")
load(":actions.bzl", "select_target_actions")

ExtraActionInfo = provider(
    doc = "Relevant information from extra_action rules.",
    fields = {
        "label": "The label of the captured extra action.",
        "cmd": "The action command template.",
        "out_templates": "A list of output templates.",
        "requires_action_output": "Boolean indicating whether or not the extra action requires the primary action output.",
        "tools": "A list of tools required by the action.",
        "inputs": "The list required inputs.",
        "input_manifests": "List of input_manifests as returned by resolve_command",
    },
)

ActionListenerInfo = provider(
    doc = "Collected information from extra_action and action_listener rules.",
    fields = {
        "mnemonics": "A Skylib set of action mnemonics on which to trigger.",
        "extra_actions": "A list of extra actions to trigger.",
        "write_extra_action": "A callable which will be used to write the extra action protobuf.",
    },
)

ActionListenerConfigInfo = provider(
    doc = "An aggregated ActionListenerInfo.",
    fields = {
        "action_listeners": "A depset of ActionListenerInfo instances.",
    },
)

ProtoWriterInfo = provider(
    doc = "Information and tools required for turning an action into a protobuf.",
    fields = {
        "write_extra_action": "A callable which will be used to write the extra action protobuf.",
    },
)

def _elems(value):
    """Either value.elems() or the value itself."""
    return getattr(value, "elems", lambda: value)()

_SAFE_CHARS = {
    ch: ch
    for ch in _elems("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-+")
}

# Output IDs longer than this will be hashed.
# As this currently uses Starlark's built-in hash(),
# which is specified to be Java's String.hashValue(),
# which is, in turn, a terrible hash function, we only do
# this as a last resort.
_MAX_ID_PATH_LENGTH = 200

def _shell_safe(value):
    """Replaces any non-safe char with +."""
    return "".join([_SAFE_CHARS.get(ch, "+") for ch in _elems(value)])

def _remove_prefix(haystack, needle):
    if haystack.startswith(needle):
        return True, haystack[len(needle):]
    return False, haystack

def _direct_label(target):
    # A dirty, dirty hack to deal with alias() and bind() targets.
    # target.label resolves the alias, but resolve_command needs the original label.
    found, target_str = _remove_prefix(str(target), "<alias target ")
    if found:
        return Label(target_str[:target_str.find(" ")])
    return target.label

def _extra_aspect_impl(target, ctx):
    for provider in [ActionListenerInfo, ActionListenerConfigInfo, ExtraActionInfo]:
        if provider in target:
            return []

    if ctx.rule.kind == "action_listener":
        return [
            ActionListenerInfo(
                mnemonics = sets.make(ctx.rule.attr.mnemonics),
                extra_actions = [
                    dep[ExtraActionInfo]
                    for dep in ctx.rule.attr.extra_actions
                    if ExtraActionInfo in dep
                ],
            ),
        ]
    if ctx.rule.kind == "extra_action":
        print([_direct_label(d) for d in ctx.rule.attr.data])
        print({
            d.label: d.files.to_list()
            for d in ctx.rule.attr.data
        })

        # cmd needs to be expanded in the context of the extra_action itself
        # so that relative $(location :target) paths resolve correctly.  We
        # need to use ctx.resolve_command() to do so as ctx.expand_location()
        # complains that java_binary targets export multiple files.
        # https://github.com/bazelbuild/bazel/issues/10067
        inputs, command, input_manifests = ctx.resolve_command(
            command = ctx.rule.attr.cmd,
            attribute = "cmd",
            expand_locations = True,
            tools = ctx.rule.attr.tools,
            label_dict = {
                _direct_label(d): d.files.to_list()
                for d in ctx.rule.attr.data
            },
        )

        return [
            ExtraActionInfo(
                label = target.label,
                cmd = command[-1],  # resolve_command returns a list invoking bash, we just want the script.
                out_templates = ctx.rule.attr.out_templates,
                requires_action_output = ctx.rule.attr.requires_action_output,
                tools = ctx.rule.files.tools,
                inputs = inputs + ctx.rule.files.data,
                input_manifests = input_manifests,
            ),
        ]
    return []

def _is_middleman_file(file):
    # This is a hack to exclude irrelevant and non-existent input files which
    # are otherwise present for some kinds of binary rules.
    return file.short_path.startswith("_middlemen/")

def normalize_action(action):
    """Returns a struct with the same attributes as the action, but with normalized values.

    Sorts action.env, sorts and filters out "middleman" artifacts from inputs and outputs.

    Args:
     action: The action to inspect and transform.

    Returns:
     A struct with the same attributes as the input action and a
     normalized attribute indicating the transformation.
    """
    if getattr(action, "normalized", False):
        return action
    values = {}
    for key in dir(action):
        if key in ("to_proto", "to_json"):
            continue
        elif key in ("inputs", "outputs"):
            # convert back to a depset to preserve API.
            value = depset(sorted([
                file
                for file in getattr(action, key).to_list()
                if not _is_middleman_file(file)
            ]))
        elif key == "env":
            # dicts maintain insertion order.
            value = {k: v for k, v in sorted(getattr(action, key).items())}
        else:
            value = getattr(action, key)
        values[key] = value
    return struct(normalized = True, original_action = action, **values)

def _original_action(action):
    if type(action) == "Action":
        return action
    return getattr(action, "original_action", None)

def as_spawn_info_struct(target, action):
    """Returns a Starlark struct which, when converted to proto, will be a valid SpawnInfo extra action."""
    action = normalize_action(action)
    return struct(**{
        "owner": str(target.label),
        "mnemonic": action.mnemonic,
        "[blaze.SpawnInfo.spawn_info]": struct(
            argument = action.argv,
            variable = [struct(name = k, value = v) for k, v in action.env.items()],
            input_file = [f.path for f in action.inputs.to_list()],
            output_file = [f.path for f in action.outputs.to_list()],
        ),
    })

action_listener_aspect = aspect(
    implementation = _extra_aspect_impl,
    attr_aspects = ["extra_actions"],
)

def write_extra_action_struct(ctx, executable, action_id, info_struct, mnemonic):
    """Writes given ExtraActionInfo struct to a file in binary form.

    Additionally creates intermediate .xa.textproto file with the proto in textproto form.
    The latter file is useful for local debugging.

    Args:
        ctx: The Bazel-provided aspect rule context.
        executable: Label to //kythe/go/util/tools/write_extra_action executable.
        action_id: Action id used to generate unique file names.
        info_struct: struct representing ExtraActionInfo proto.
        mnemonic: Mnemonic to use for the action that writes ExtraActionInfo proto.

    Returns:
      A file containing binary ExtraActionInfo proto.
    """
    txtpb = ctx.actions.declare_file(action_id + ".xa.textproto")
    ctx.actions.write(
        output = txtpb,
        content = proto.encode_text(info_struct),
    )
    output = ctx.actions.declare_file(action_id + ".xa")
    args = ctx.actions.args()
    args.add(txtpb)
    args.add(output)
    ctx.actions.run(
        outputs = [output],
        inputs = [txtpb],
        arguments = [args],
        executable = executable,
        mnemonic = mnemonic,
        toolchain = None,
    )
    return output

def _common_prefix_length(a, b):
    for i in range(min(len(a), len(b))):
        if a[i] != b[i]:
            return i
    return min(len(a), len(b))

def _output_id(label, action, default):
    """Returns a identifier based on the action outputs."""
    prefixes = [
        (_common_prefix_length(label.package, output.short_path), output.short_path)
        for output in action.outputs.to_list()
    ]
    if not prefixes:
        return default

    # Use the output with the longest common shared prefix with the package.
    n, path = max(prefixes)

    # Replace uses of the label in the output name with its hash as we
    # already use the label in the output name elsewhere and sometimes it's really long.
    # Additionally replace semantic characters with safer alternatives.
    id = path[n:].strip("/").replace(label.name, str(hash(label.name))).replace(".", "+").replace("/", "+")
    if len(id) + len(label.package) + len(label.name) > _MAX_ID_PATH_LENGTH:
        hashed = str(hash(id))
        truncate = _MAX_ID_PATH_LENGTH - (len(hashed) + len(label.package) + len(label.name))
        if 0 < truncate and truncate < len(id):
            return id[:truncate] + hashed
        return hashed
    return id

def _action_id_map(target, actions):
    """Returns a dict mapping from an action id string to the action."""
    return {
        # b/178768424: Prior to cl/354550518, salvation_worker splats everything after the
        # *first* dot into the "extension" field, which causes Monarch quota issues.
        # Replace any errant dots with "+" to avoid this behavior in the meantime.
        _shell_safe("{}-{}-{}".format(
            target.label.name.replace(".", "+"),
            action.mnemonic,
            _output_id(target.label, action, default = str(i)),
        )): action
        for i, action in _action_items(actions)
    }

def _action_items(actions):
    """Returns either actions.items() or enumerate(actions)."""
    return getattr(actions, "items", lambda: enumerate(actions))()

def _run_extra_action(target, ctx, write_extra_action, xainfo, action_id, action):
    if type(xainfo) == "Target":
        xainfo = xainfo[ExtraActionInfo]

    xa_file, extra_inputs = write_extra_action(
        target,
        ctx,
        action_id,
        action,
    )

    if not xa_file:
        return []

    output_map = {
        template: ctx.actions.declare_file(template.replace("$(ACTION_ID)", action_id))
        for template in xainfo.out_templates
    }

    inputs = [action.inputs]
    if xainfo.requires_action_output:
        inputs.append(action.outputs)
    if xainfo.inputs != None:
        inputs.append(depset(xainfo.inputs))

    inputs.append(extra_inputs)

    cmd = xainfo.cmd.replace("$(EXTRA_ACTION_FILE)", xa_file.path)
    for template, output in output_map.items():
        cmd = cmd.replace("$(output {})".format(template), output.path)

    kwargs = {}
    shadowed_action = _original_action(action)
    if shadowed_action != None:
        kwargs["shadowed_action"] = shadowed_action

    ctx.actions.run_shell(
        inputs = depset(direct = [xa_file], transitive = inputs),
        outputs = output_map.values(),
        mnemonic = "KytheExtraAction" + action.mnemonic,
        progress_message = "Executing extra_action {} on {}".format(xainfo.label, target.label),
        tools = xainfo.tools,
        command = cmd,
        input_manifests = xainfo.input_manifests,
        toolchain = None,
        **kwargs
    )
    return output_map.values()

def _run_extra_actions(target, ctx, write_extra_action, extra_actions, actions):
    """Run the list of ExtraActionInfo over the provided actions.

    Args:
      target: The Bazel Target on which to run extra actions.
      ctx: The rule context provided to the aspect.
      write_extra_action: A callable used to write the .xa proto file.
      extra_actions: The list of ExtraActionInfo providers to use.
      actions: The list of rule actions for which to run the extra actions.

    Returns:
      A list of output File objects produced by the extra actions.
    """
    outputs = []
    for xainfo in extra_actions:
        for id, action in actions.items():
            outputs.extend(_run_extra_action(target, ctx, write_extra_action, xainfo, id, action))
    return outputs

def run_action_listener(target, ctx, listener, actions):
    """Run the action listener over matching actions.

    Args:
      target: The Bazel Target on which to run extra actions.
      ctx: The rule context provided to the aspect.
      listener: The ActionListenerInfo providers to use.
      actions: The list or dict of rule actions for which to run the extra actions.

    Returns:
      A list of output File objects produced by the extra actions.
    """
    if type(listener) == "Target":
        listener = listener[ActionListenerInfo]

    actions = _action_id_map(target, actions)
    return _run_extra_actions(
        target,
        ctx,
        listener.write_extra_action,
        listener.extra_actions,
        {
            id: action
            for id, action in actions.items()
            if sets.contains(listener.mnemonics, action.mnemonic)
        },
    )

def extract_target(target, ctx, config):
    """Extracts the provided target with the given config.

    Args:
      target: The Bazel-provided Target object to extract.
      ctx: The Bazel-provided aspect rule context.
      config: The KytheExtractorConfigInfo to use for extraction.

    Returns:
      A depset of kzip files.
    """
    actions = select_target_actions(
        target,
        ctx.rule,
        config.action_selection_config,
    )
    kzips = []
    for listener in config.action_listeners:
        kzips.extend(run_action_listener(
            target,
            ctx,
            listener,
            actions,
        ))
    return depset(kzips)

def _spawn_info_impl(ctx):
    executable = ctx.executable._write_extra_action

    def _write_spawn_info_proto(target, ctx, action_id, action):
        return write_extra_action_struct(
            ctx = ctx,
            executable = executable,
            action_id = action_id,
            info_struct = as_spawn_info_struct(target, action),
            mnemonic = "KytheWriteSpawnInfo",
        ), depset()

    return [
        ProtoWriterInfo(
            write_extra_action = _write_spawn_info_proto,
        ),
    ]

spawn_info_proto_writer = rule(
    implementation = _spawn_info_impl,
    attrs = {
        "_write_extra_action": attr.label(
            default = Label("//kythe/go/util/tools/write_extra_action"),
            executable = True,
            cfg = "exec",
        ),
    },
    provides = [ProtoWriterInfo],
)
