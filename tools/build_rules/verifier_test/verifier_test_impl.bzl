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
        "log": "Log file from the indexer execution. The file will be stored in test outputs.",
        "name": "Name that uniquely identifies these entries within a test. For example it can be language name if test is multi-language.",
    },
)

KytheEntryProducerInfo = provider(
    doc = "Provider indicating an executable to be called which will produce Kythe entries on stdout.",
    fields = {
        "executables": "A list of File objects to run which should produce Kythe entries on stdout.",
        "runfiles": "Required runfiles.",
        "log": "Log file from the indexer execution. The file will be stored in test outputs.",
        "name": "Name that uniquely identifies these entries within a test. For example it can be language name if test is multi-language.",
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

    # Check that all deps have unique names.
    unique_names = {}
    for dep in ctx.attr.deps:
        info = None
        if KytheEntryProducerInfo in dep:
            info = dep[KytheEntryProducerInfo]
        elif KytheEntries in dep:
            info = dep[KytheEntries]
        if info and hasattr(info, "name"):
            if info.name in unique_names:
                fail("Multiple deps producers with the same name: %s. Deps: %s, %s" % (info.name, unique_names[info.name], dep.label.name))
            unique_names[info.name] = dep.label.name

    # List of '$indexer:$name' strings to execute during the test. 'name' is used in log file.
    indexers = []
    runfiles = []

    # List of '$logfile:$name_indexer.log' strings.
    logs = []

    # List of '$entryfile:$name_entries.json' strings.
    entries_to_log = []
    for dep in ctx.attr.deps:
        if KytheEntryProducerInfo in dep:
            runfiles.append(dep[KytheEntryProducerInfo].runfiles)
            for executable in dep[KytheEntryProducerInfo].executables:
                name = dep[KytheEntryProducerInfo].name if hasattr(dep[KytheEntryProducerInfo], "name") else dep.label.name.replace("/", "_")
                indexers.append("%s:%s" % (
                    shell.quote(executable.short_path),
                    name,
                ))
        if KytheEntries in dep:
            if dep[KytheEntries].files:
                entries.append(dep[KytheEntries].files)
                if hasattr(dep[KytheEntries], "name"):
                    for file in dep[KytheEntries].files.to_list():
                        entries_to_log.append("%s:%s_entries.json" % (
                            shell.quote(file.short_path),
                            dep[KytheEntries].name,
                        ))
            else:
                entries_gz.append(dep[KytheEntries].compressed)

            if hasattr(dep[KytheEntries], "log"):
                runfiles.append(ctx.runfiles(files = [dep[KytheEntries].log]))
                logs.append("%s:%s_indexer.log" % (
                    shell.quote(dep[KytheEntries].log.short_path),
                    dep[KytheEntries].name,
                ))

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
            "@INDEXERS@": " ".join(indexers),
            "@INDEXER_LOGS@": " ".join(logs),
            "@ENTRIES@": " ".join([shell.quote(e.short_path) for e in entries]),
            "@ENTRIES_TO_LOG@": " ".join(entries_to_log),
            "@ENTRIES_GZ@": " ".join([shell.quote(e.short_path) for e in entries_gz]),
            # If failure is expected, invert the sense of the verifier return.
            "@INVERT@": "!" if not ctx.attr.expect_success else "",
            "@VERIFIER@": shell.quote(ctx.executable._verifier.short_path),
            "@REWRITE@": "1" if not ctx.attr.resolve_code_facts else "",
            "@MARKEDSOURCE@": shell.quote(ctx.executable._markedsource.short_path),
            "@WORKSPACE_NAME@": ctx.workspace_name,
            "@ENTRYSTREAM@": shell.quote(ctx.executable._entrystream.short_path),
        },
    )
    tools = [
        ctx.outputs.executable,
        ctx.executable._verifier,
        ctx.executable._markedsource,
        ctx.executable._entrystream,
    ]
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
        "_entrystream": attr.label(
            default = Label("//kythe/go/platform/tools/entrystream"),
            executable = True,
            cfg = "target",
        ),
    },
    test = True,
    implementation = _verifier_test_impl,
)
