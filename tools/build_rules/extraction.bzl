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

# TODO(schroederc): move reusable chunks of verifier_test.bzl here

def _merge_kzips_impl(ctx):
    output = ctx.outputs.kzip
    ctx.actions.run(
        outputs = [output],
        inputs = ctx.files.srcs,
        executable = ctx.executable._kzip,
        mnemonic = "MergeKZips",
        arguments = ["merge", "--ignore_duplicate_cus", "--output", output.path] + [f.path for f in ctx.files.srcs],
    )
    return [DefaultInfo(files = depset([output]))]

merge_kzips = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
        ),
        "_kzip": attr.label(
            default = Label("//kythe/go/platform/tools/kzip"),
            executable = True,
            cfg = "exec",
        ),
    },
    outputs = {"kzip": "%{name}.kzip"},
    implementation = _merge_kzips_impl,
)

def _filter_kzip_impl(ctx):
    output = ctx.outputs.kzip
    args = ctx.actions.args()
    args.add("filter")
    args.add("--input", ctx.file.src.path)
    args.add_joined("--languages", ctx.attr.languages, join_with = ",")
    args.add("--output", output.path)
    ctx.actions.run(
        outputs = [output],
        inputs = [ctx.file.src],
        executable = ctx.executable._kzip,
        mnemonic = "FilterKZips",
        arguments = [args],
    )
    return [DefaultInfo(files = depset([output]))]

filter_kzip = rule(
    attrs = {
        "src": attr.label(
            allow_single_file = True,
            mandatory = True,
        ),
        "languages": attr.string_list(
            doc = "List of languages to retain from the input kzip.",
            mandatory = True,
        ),
        "_kzip": attr.label(
            default = Label("//kythe/go/platform/tools/kzip"),
            executable = True,
            cfg = "exec",
        ),
    },
    outputs = {"kzip": "%{name}.kzip"},
    implementation = _filter_kzip_impl,
)
