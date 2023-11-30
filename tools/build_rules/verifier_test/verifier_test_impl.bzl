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
"""Private implementation details for verifier_test rules.

Public API available in verifier_test.bzl.
"""

load("@bazel_skylib//lib:shell.bzl", "shell")

visibility("private")

KytheVerifierSources = provider(
    doc = "Input files which the verifier should inspect for assertions.",
    fields = {
        "files": "Depset of files which should be considered.",
    },
)

KytheEntries = provider(
    doc = "Kythe indexer entry facts.",
    fields = {
        "compressed": "Depset of combined, compressed index entries.",
        "files": "Depset of files which combine to make an index.",
    },
)

KytheEntryProducerInfo = provider(
    doc = "Provider indicating an executable to be called which will produce Kythe entries on stdout.",
    fields = {
        "executables": "A list of File objects to run which should produce Kythe entries on stdout.",
        "runfiles": "Required runfiles.",
    },
)

def _verifier_test_impl(ctx):
    entries = []
    entries_gz = []
    sources = []
    for src in ctx.attr.srcs:
        if KytheVerifierSources in src:
            sources.append(src[KytheVerifierSources].files)
        else:
            sources.append(src.files)

    indexers = []
    runfiles = []
    for dep in ctx.attr.deps:
        if KytheEntryProducerInfo in dep:
            indexers.extend(dep[KytheEntryProducerInfo].executables)
            runfiles.append(dep[KytheEntryProducerInfo].runfiles)
        if KytheEntries in dep:
            if dep[KytheEntries].files:
                entries.append(dep[KytheEntries].files)
            else:
                entries_gz.append(dep[KytheEntries].compressed)

    # Flatten input lists
    entries = depset(transitive = entries).to_list()
    entries_gz = depset(transitive = entries_gz).to_list()
    sources = depset(transitive = sources).to_list()

    if not (entries or entries_gz or indexers):
        fail("Missing required entry stream input (check your deps!)")
    args = ctx.attr.opts + [shell.quote(src.short_path) for src in sources]

    # If no dependency specifies KytheVerifierSources and
    # we aren't provided explicit sources, assume `--use_file_nodes`.
    if not sources and "--use_file_nodes" not in args:
        args.append("--use_file_nodes")
    ctx.actions.expand_template(
        template = ctx.file._template,
        output = ctx.outputs.executable,
        is_executable = True,
        substitutions = {
            "@ARGS@": " ".join(args),
            "@INDEXERS@": "\n".join([shell.quote(i.short_path) for i in indexers]),
            "@ENTRIES@": " ".join([shell.quote(e.short_path) for e in entries]),
            "@ENTRIES_GZ@": " ".join([shell.quote(e.short_path) for e in entries_gz]),
            # If failure is expected, invert the sense of the verifier return.
            "@INVERT@": "!" if not ctx.attr.expect_success else "",
            "@VERIFIER@": shell.quote(ctx.executable._verifier.short_path),
            "@REWRITE@": "1" if not ctx.attr.resolve_code_facts else "",
            "@MARKEDSOURCE@": shell.quote(ctx.executable._markedsource.short_path),
            "@WORKSPACE_NAME@": ctx.workspace_name,
        },
    )
    tools = [
        ctx.outputs.executable,
        ctx.executable._verifier,
    ]
    if ctx.attr.resolve_code_facts:
        tools.append(ctx.executable._markedsource)
    return [
        DefaultInfo(
            runfiles = ctx.runfiles(files = sources + entries + entries_gz + tools).merge_all(runfiles),
            executable = ctx.outputs.executable,
        ),
    ]

verifier_test = rule(
    attrs = {
        "srcs": attr.label_list(
            doc = "Targets or files containing verifier goals.",
            allow_files = True,
            providers = [KytheVerifierSources],
        ),
        "resolve_code_facts": attr.bool(default = False),
        # Arguably, "expect_failure" is more natural, but that
        # attribute is used by Skylark.
        "expect_success": attr.bool(default = True),
        "opts": attr.string_list(),
        "deps": attr.label_list(
            doc = "Targets which produce graph entries to verify.",
            providers = [[KytheEntries], [KytheEntryProducerInfo]],
        ),
        "_template": attr.label(
            default = Label("//tools/build_rules/verifier_test:verifier_test.sh.in"),
            allow_single_file = True,
        ),
        "_verifier": attr.label(
            default = Label("//kythe/cxx/verifier"),
            executable = True,
            cfg = "target",
        ),
        "_markedsource": attr.label(
            default = Label("//kythe/go/util/tools/markedsource"),
            executable = True,
            cfg = "target",
        ),
    },
    test = True,
    implementation = _verifier_test_impl,
)
