#
# Copyright 2016 The Kythe Authors. All rights reserved.
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
"""Rules and macros related to Kythe verifier-based tests."""

load("@bazel_skylib//lib:shell.bzl", "shell")

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

def _atomize_entries_impl(ctx):
    zcat = ctx.executable._zcat
    entrystream = ctx.executable._entrystream
    postprocessor = ctx.executable._postprocessor
    atomizer = ctx.executable._atomizer

    inputs = depset(ctx.files.srcs, transitive = [
        dep.kythe_entries
        for dep in ctx.attr.deps
    ])

    sort_args = ctx.actions.args()
    sort_args.add_all([zcat, entrystream, sorted_entries])
    sort_args.add_all(inputs)
    sorted_entries = ctx.actions.declare_file("_sorted_entries", sibling = ctx.outputs.entries)
    ctx.actions.run_shell(
        outputs = [sorted_entries],
        inputs = [zcat, entrystream] + inputs.to_list(),
        mnemonic = "SortEntries",
        command = '("$1" "${@:4}" | "$2" --sort) > "$3" || rm -f "$3"',
        arguments = [sort_args],
    )

    process_args = ctx.actions.args()
    process_args.add_all(["--entries", sorted_entries, "--out", leveldb])
    leveldb = ctx.actions.declare_file("_serving_tables", sibling = ctx.outputs.entries)
    ctx.actions.run(
        outputs = [leveldb],
        inputs = [sorted_entries, postprocessor],
        executable = postprocessor,
        mnemonic = "PostProcessEntries",
        arguments = [process_args],
    )

    atomize_args = ctx.actions.args()
    atomize_args.add_all([atomizer, "--api", leveldb])
    atomize_args.add_all(ctx.attr.file_tickets)
    atomize_args.add(ctx.outputs.entries)
    ctx.actions.run_shell(
        outputs = [ctx.outputs.entries],
        inputs = [atomizer, leveldb],
        mnemonic = "AtomizeEntries",
        command = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") | gzip -c > "${@:${#@}}"',
        arguments = [atomize_args],
        execution_requirements = {
            # TODO(shahms): Remove this when we can use a non-LevelDB store.
            "local": "true",  # LevelDB is bad and should feel bad.
        },
    )
    return struct()

atomize_entries = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = [
                ".entries",
                ".entries.gz",
            ],
        ),
        "file_tickets": attr.string_list(
            mandatory = True,
            allow_empty = False,
        ),
        "deps": attr.label_list(
            providers = ["kythe_entries"],
        ),
        "_atomizer": attr.label(
            default = Label("//kythe/go/test/tools/xrefs_atomizer"),
            executable = True,
            cfg = "exec",
        ),
        "_entrystream": attr.label(
            default = Label("//kythe/go/platform/tools/entrystream"),
            executable = True,
            cfg = "exec",
        ),
        "_postprocessor": attr.label(
            default = Label("//kythe/go/serving/tools/write_tables"),
            executable = True,
            cfg = "exec",
        ),
        "_zcat": attr.label(
            default = Label("//tools:zcatext"),
            executable = True,
            cfg = "exec",
        ),
    },
    outputs = {
        "entries": "%{name}.entries.gz",
    },
    implementation = _atomize_entries_impl,
)

def extract(
        ctx,
        kzip,
        extractor,
        srcs,
        opts,
        deps = [],
        env = {},
        vnames_config = None,
        mnemonic = "ExtractCompilation"):
    """Create an extract action using the provided tool and inputs.

    Runs the extractor tool under an environment to produce the given kzip
    output file.  The extractor is passed each string from opts after expanding
    any build artifact locations and then each File's path from the srcs
    collection.

    Args:
      ctx: The Bazel rule context to use for actions.
      kzip: Declared .kzip output File
      extractor: Executable extractor tool to invoke
      srcs: Files passed to extractor tool; the compilation's source file inputs
      opts: List of options (or Args object) passed to the extractor tool before source files
      deps: Dependencies for the extractor's action (not passed to extractor on command-line)
      env: Dictionary of environment variables to provide.
      vnames_config: Optional path to a VName configuration file
      mnemonic: Mnemonic of the extractor's action

    Returns:
      The output file generated.
    """
    final_env = {
        "KYTHE_OUTPUT_FILE": kzip.path,
        "KYTHE_ROOT_DIRECTORY": ".",
    }
    final_env.update(env)

    if type(srcs) != "depset":
        srcs = depset(direct = srcs)
    if type(deps) != "depset":
        deps = depset(direct = deps)
    direct_inputs = []
    if vnames_config:
        final_env["KYTHE_VNAMES"] = vnames_config.path
        direct_inputs.append(vnames_config)
    inputs = depset(direct = direct_inputs, transitive = [srcs, deps])

    args = opts
    if type(args) != "Args":
        args = ctx.actions.args()
        args.add_all([ctx.expand_location(o) for o in opts])
    args.add_all(srcs)

    ctx.actions.run(
        inputs = inputs,
        tools = [extractor],
        outputs = [kzip],
        mnemonic = mnemonic,
        executable = extractor,
        arguments = [args],
        env = final_env,
        toolchain = None,
    )
    return kzip

def _index_compilation_impl(ctx):
    sources = []
    intermediates = []
    for dep in ctx.attr.deps:
        if KytheVerifierSources in dep:
            sources.append(dep[KytheVerifierSources].files)
        for input in dep.files.to_list():
            entries = ctx.actions.declare_file(
                ctx.label.name + input.basename + ".entries",
                sibling = ctx.outputs.entries,
            )
            intermediates.append(entries)

            args = ctx.actions.args()
            args.add(ctx.executable.indexer)
            args.add_all([ctx.expand_location(o) for o in ctx.attr.opts])
            args.add_all([input, entries])
            ctx.actions.run_shell(
                outputs = [entries],
                inputs = [input],
                tools = [ctx.executable.indexer] + ctx.files.tools,
                arguments = [args],
                command = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") > "${@:${#@}}"',
                mnemonic = "IndexCompilation",
                toolchain = None,
            )

    args = ctx.actions.args()
    args.add("cat")
    args.add_all(intermediates)
    args.add(ctx.outputs.entries)
    ctx.actions.run_shell(
        outputs = [ctx.outputs.entries],
        inputs = intermediates,
        command = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") | gzip -c > "${@:${#@}}"',
        mnemonic = "CompressEntries",
        arguments = [args],
    )
    return [
        KytheVerifierSources(files = depset(transitive = sources)),
        KytheEntries(compressed = depset([ctx.outputs.entries]), files = depset(intermediates)),
    ]

index_compilation = rule(
    attrs = {
        "indexer": attr.label(
            mandatory = True,
            executable = True,
            cfg = "exec",
        ),
        "opts": attr.string_list(),
        "tools": attr.label_list(
            cfg = "exec",
            allow_files = True,
        ),
        "deps": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = [".kzip"],
        ),
    },
    outputs = {
        "entries": "%{name}.entries.gz",
    },
    implementation = _index_compilation_impl,
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

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)

def kythe_integration_test(name, srcs, file_tickets, tags = [], size = "small"):
    entries = _invoke(
        atomize_entries,
        name = name + "_atomized_entries",
        testonly = True,
        srcs = [],
        file_tickets = file_tickets,
        tags = tags,
        deps = srcs,
    )
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        opts = ["--ignore_dups"],
        tags = tags,
        deps = [entries],
    )
