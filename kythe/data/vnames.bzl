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

def _construct_vnames_config_impl(ctx):
    corpus = ctx.attr.corpus
    if "kythe_corpus" in ctx.var:
        # Use --define kythe_corpus=... override
        corpus = ctx.var["kythe_corpus"]
    elif corpus == "":
        # Use workspace name as default
        corpus = ctx.workspace_name
    srcs = ctx.files.srcs
    merged = ctx.actions.declare_file(ctx.label.name + "_merged.json")
    ctx.actions.run_shell(
        outputs = [merged],
        inputs = srcs,
        command = "\n".join([
            "set -e -o pipefail",
            "cat " + " ".join([src.path for src in srcs]) + " | " +
            "tr -d '\n' | sed 's/\]\[/,/g' > " + merged.path,
        ]),
    )
    ctx.actions.expand_template(
        template = merged,
        output = ctx.outputs.vnames,
        substitutions = {"CORPUS": corpus},
    )
    return [DefaultInfo(
        data_runfiles = ctx.runfiles(files = [ctx.outputs.vnames]),
    )]

construct_vnames_config = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
        ),
        "corpus": attr.string(),
    },
    outputs = {"vnames": "%{name}.json"},
    implementation = _construct_vnames_config_impl,
)
