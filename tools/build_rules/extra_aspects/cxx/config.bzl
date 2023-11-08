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

"""C++ Extractor configuration."""

load(
    "//tools/build_rules/extra_aspects:extra_actions.bzl",
    "ProtoWriterInfo",
    "normalize_action",
)

def _has_assembly_source(action, inputs):
    for file in inputs:
        if action.argv and file.path not in action.argv:
            # If we have a usable argv, check that the input is present
            # to ignore non-source input files.
            # Prior to Starlarkification, argv was not useful.
            # When extracting via the test transition, `inputs` contains
            # some toolchain-specific assembly files that aren't actually
            # used by the compilation, which will trigger false-positives below.
            continue
        if file.extension in ("s", "S"):
            return True
    return False

def _as_cpp_compile_info(target, action):
    action = normalize_action(action)
    inputs = action.inputs.to_list()

    # Entirely skip compilations containing assembly.
    if _has_assembly_source(action, inputs):
        return None

    return struct(**{
        "owner": str(target.label),
        "mnemonic": action.mnemonic,
        "[blaze.CppCompileInfo.cpp_compile_info]": struct(
            variable = [struct(name = k, value = v) for k, v in action.env.items()],
            sources_and_headers = [f.path for f in inputs],
            output_file = [f.path for f in action.outputs.to_list()][0],
        ),
    })

def _write_cpp_compile_info(executable, target, ctx, action_id, action):
    cpp_info = _as_cpp_compile_info(target, action)
    if not cpp_info:
        return None
    txtpb = ctx.actions.declare_file(action_id + ".xa.textproto")
    ctx.actions.write(
        output = txtpb,
        content = proto.encode_text(cpp_info),
    )
    output = ctx.actions.declare_file(action_id + ".xa")
    args = ctx.actions.args()
    args.add(txtpb)
    args.add(output)
    args.add("--")

    ctx.actions.run(
        outputs = [output],
        inputs = depset([txtpb]),
        arguments = [args] + action.args,
        executable = executable,
        mnemonic = "KytheWriteCppCompileInfo",
        # In theory, the exact environment and inputs can change during execution.
        # In order to record this, we shadow the original action when writing the proto.
        shadowed_action = action,
        toolchain = None,
    )
    return output

def _cpp_compile_info_impl(ctx):
    executable = ctx.executable._write_extra_action

    def write_extra_action(target, ctx, action_id, action):
        return (
            _write_cpp_compile_info(executable, target, ctx, action_id, action),
            depset(),
        )

    return [
        ProtoWriterInfo(
            write_extra_action = write_extra_action,
        ),
    ]

cpp_compile_info_proto_writer = rule(
    implementation = _cpp_compile_info_impl,
    attrs = {
        "_write_extra_action": attr.label(
            default = Label("//kythe/go/util/tools/write_extra_action"),
            executable = True,
            cfg = "exec",
        ),
    },
    provides = [ProtoWriterInfo],
)
