def _asciidoc_impl(ctx):
    asciidoc = ctx.toolchains["//tools/build_rules/external_tools:external_tools_toolchain_type"].asciidoc
    args = ["--backend", "html", "--no-header-footer"]
    for key, value in ctx.attr.attrs.items():
        if value:
            args += ["--attribute=%s=%s" % (key, value)]
        else:
            args += ["--attribute=%s!" % (key,)]
    if ctx.attr.example_script:
        args += ["--attribute=example_script=" + ctx.file.example_script.path]
    args += ["--conf-file=%s" % c.path for c in ctx.files.confs]
    args += ["-o", ctx.outputs.out.path]
    args += [ctx.file.src.path]

    # Since asciidoc shells out to dot and dot may not be in the standard bazel
    # path, have to find all the tools we use and construct a bespoke PATH for
    # our run_shell command.
    dot = ctx.toolchains["//tools/build_rules/external_tools:external_tools_toolchain_type"].dot
    dot = dot[0:dot.rfind('/')]
    python = ctx.toolchains["//tools/build_rules/external_tools:external_tools_toolchain_type"].python
    python = python[0:python.rfind('/')]
    cat = ctx.toolchains["//tools/build_rules/external_tools:external_tools_toolchain_type"].cat
    cat = cat[0:cat.rfind('/')]

    paths = dict([(dot, ""), (python, ""), (cat, "")]).keys()

    # Run asciidoc, capture stderr, look in stderr for error messages and fail if we find any.
    ctx.actions.run_shell(
        inputs = [ctx.file.src] + ctx.files.confs + ([ctx.file.example_script] if ctx.file.example_script else []) + ctx.files.data,
        outputs = [ctx.outputs.out, ctx.outputs.logfile],
        env = {
            "PATH": ":".join(paths),
        },
        command = "\n".join([
            "set -eo pipefail",
            '"%s" %s 2> "%s"' % (
                asciidoc,
                " ".join(args),
                ctx.outputs.logfile.path,
            ),
            'cat "%s"' % (ctx.outputs.logfile.path),
            '! grep -q -e "filter non-zero exit code" -e "no output from filter" "%s"' % (
                ctx.outputs.logfile.path
            ),
        ]),
        mnemonic = "RunAsciidoc",
    )

asciidoc = rule(
    implementation = _asciidoc_impl,
    toolchains = ["//tools/build_rules/external_tools:external_tools_toolchain_type"],
    attrs = {
        "src": attr.label(
            doc = "asciidoc file to process",
            allow_single_file = True,
        ),
        "attrs": attr.string_dict(
            doc = "Dict of attributes to pass to asciidoc as --attribute=KEY=VALUE",
        ),
        "confs": attr.label_list(
            doc = "`conf-file`s to pass to asciidoc",
            allow_files = True,
        ),
        "data": attr.label_list(
            doc = "Files/targets used during asciidoc generation. Only needed for tools used in example_script.",
            allow_files = True,
        ),
        "example_script": attr.label(
            doc = "Script to pass to asciidoc as --attribute=example_script=VALUE.",
            allow_single_file = True,
        ),
    },
    doc = "Generate asciidoc",
    outputs = {
        "out": "%{name}.html",
        "logfile": "%{name}.logfile",
    },
)
