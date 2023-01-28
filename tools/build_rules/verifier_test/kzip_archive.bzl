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
"""
Rule for creating generic .kzip compilation units using kzip create.
"""

def _key_pair(item):
    return "{}={}".format(*item)

def _kythe_uri(corpus, language, path = None, root = None):
    uri = "kythe://{}?lang={}".format(corpus, language)
    if path:
        uri += "?path=" + path
    if root:
        uri += "?root=" + root
    return uri

def _single_path(srcs):
    # If there is exactly one source, use that for the path.
    # This is primarily to be compatible with some existing uses of kzip create.
    if len(srcs) == 1:
        return srcs[0].path
    return None

def _kzip_archive(ctx):
    args = ctx.actions.args()
    args.add("create")
    args.add("--uri", _kythe_uri(
        corpus = ctx.attr.corpus,
        language = ctx.attr.lang,
        path = _single_path(ctx.files.srcs),
        root = ctx.expand_location(ctx.attr.root),
    ))
    args.add("--output", ctx.outputs.kzip)
    args.add("--output_key", str(ctx.label))
    args.add_joined("--source_file", ctx.files.srcs, join_with = ",")
    args.add_joined("--required_input", ctx.files.deps, join_with = ",")
    args.add_all(
        [(k, ctx.expand_location(v)) for k, v in ctx.attr.env.items()],
        before_each = "--env",
        map_each = _key_pair,
    )
    args.add_all([ctx.expand_location(d) for d in ctx.attr.details], before_each = "--details")
    args.add_all("--rules", ctx.files.vnames_config)
    if ctx.attr.has_compile_errors:
        args.add("--has_compile_errors")
    if ctx.attr.working_directory:
        args.add("--working_directory", ctx.expand_location(ctx.attr.working_directory))
    if ctx.attr.entry_context:
        args.add("--entry_context", ctx.expand_location(ctx.attr.entry_context))

    # All arguments after a plain "--" will be passed as arguments in the compilation unit.
    # This must come last.
    args.add_all("--", [ctx.expand_location(a) for a in ctx.attr.args])
    ctx.actions.run(
        outputs = [ctx.outputs.kzip],
        inputs = ctx.files.srcs + ctx.files.deps + ctx.files.vnames_config,
        executable = ctx.executable._kzip,
        arguments = [args],
        mnemonic = "KzipCreate",
    )

kzip_archive = rule(
    attrs = {
        "corpus": attr.string(
            doc = "The corpus to which this compilation belongs.",
            default = "kythe",
        ),
        "lang": attr.string(
            doc = "The language this compilation should be indexed as.",
            mandatory = True,
        ),
        "root": attr.string(
            doc = "The root to use, if any, in the compilation unit Vname.",
        ),
        "srcs": attr.label_list(
            doc = "List of source files.",
            allow_files = True,
            allow_empty = False,
        ),
        "deps": attr.label_list(
            doc = "List of non-source required_input files.",
            allow_files = True,
        ),
        "args": attr.string_list(
            doc = "List of command-line arguments for the compilation unit.",
        ),
        "working_directory": attr.string(
            doc = "Manually specify a working_directory to use in the compilation unit.",
        ),
        "env": attr.string_dict(
            doc = "Environment variables to specify in the compilation unit.",
        ),
        "entry_context": attr.string(
            doc = "Indexer-specific context to provide in the compilation unit.",
        ),
        "has_compile_errors": attr.bool(
            doc = "Whether to indicate compilation errors in the compilation unit.",
        ),
        "details": attr.string_list(
            doc = "JSON-encoded proto.Any details message to include.",
        ),
        "vnames_config": attr.label(
            allow_single_file = True,
            doc = "vnames.json file to use, if any.",
        ),
        "_kzip": attr.label(
            default = Label("//kythe/go/platform/tools/kzip"),
            allow_single_file = True,
            executable = True,
            cfg = "exec",
        ),
    },
    implementation = _kzip_archive,
    outputs = {
        "kzip": "%{name}.kzip",
    },
)
