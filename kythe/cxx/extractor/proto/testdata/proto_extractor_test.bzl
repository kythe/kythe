"""Rules for testing the proto extractor"""

# Copyright 2018 The Kythe Authors. All rights reserved.
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

def _proto_extract_kzip_impl(ctx):
    cmd = [ctx.executable._extractor.path] + [p.path for p in ctx.files.protos]
    if ctx.attr.protoc_args:
        cmd += ["--"] + ctx.attr.protoc_args
    ctx.actions.run_shell(
        mnemonic = "ProtoExtract",
        command = " ".join(cmd),
        env = ctx.attr.extra_env + {"KYTHE_OUTPUT_FILE": ctx.outputs.kzip.path},
        outputs = [ctx.outputs.kzip],
        tools = [ctx.executable._extractor],
        inputs = ctx.files.protos + ctx.files.extra_files,
    )
    return [DefaultInfo(runfiles = ctx.runfiles(files = [ctx.outputs.kzip]))]

proto_extract_kzip = rule(
    implementation = _proto_extract_kzip_impl,
    attrs = {
        "protos": attr.label_list(allow_files = True, mandatory = True),
        # Protos needed as dependencies for @protos and any other files needed
        # to run the extractor.
        "extra_files": attr.label_list(allow_files = True),
        "_extractor": attr.label(
            cfg = "host",
            executable = True,
            default = Label("//kythe/cxx/extractor/proto:proto_extractor"),
        ),
        "protoc_args": attr.string_list(),
        "extra_env": attr.string_dict(),
    },
    outputs = {"kzip": "%{name}.kzip"},
)

def proto_extraction_golden_test(
        name,
        protos,
        extra_files = [],
        protoc_args = [],
        extra_env = {}):
    """Runs the extractor with the given parameters, then compares the generated
    kzip's compilation unit to a golden file.
    """
    kzip = name + "_kzip"
    proto_extract_kzip(
        name = kzip,
        protoc_args = protoc_args,
        extra_files = extra_files,
        protos = protos,
        extra_env = extra_env,
    )

    golden_file = name + ".UNIT"
    kindex_tool = "@io_kythe//kythe/cxx/tools:kindex_tool"
    native.sh_test(
        name = name + "_test",
        srcs = ["kzip_unit_diff_test.sh"],
        args = [
            "$(location %s)" % kindex_tool,
            "$(location %s)" % kzip,
            "$(location %s)" % golden_file,
        ],
        data = [
            golden_file,
            "skip_functions.sh",
            kzip,
            kindex_tool,
        ],
    )
