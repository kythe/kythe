load("@bazel_skylib//:lib.bzl", "shell")

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

    # Get the path where all our necessary tools are located so it can be set
    # to PATH in our run_shell command.
    tool_path = ctx.toolchains["//tools/build_rules/external_tools:external_tools_toolchain_type"].path

    logfile = ctx.actions.declare_file(ctx.attr.name + ".logfile")

    # Run asciidoc, capture stderr, look in stderr for error messages and fail if we find any.
    ctx.actions.run_shell(
        inputs = [ctx.file.src] + ctx.files.confs + ([ctx.file.example_script] if ctx.file.example_script else []) + ctx.files.data,
        outputs = [ctx.outputs.out, logfile],
        env = {
            "PATH": tool_path,
        },
        command = "\n".join([
            '%s %s 2> %s' % (
                shell.quote(asciidoc),
                " ".join([shell.quote(arg) for arg in args]),
                shell.quote(logfile.path),
            ),
            "if [[ $? -ne 0 ]]; then",
            "exit 1",
            "fi",
            'cat %s' % (shell.quote(logfile.path)),
            'grep -q -e "filter non-zero exit code" -e "no output from filter" %s' % (
                shell.quote(logfile.path)
            ),
            "if [[ $? -ne 1 ]]; then",
            "exit 1",
            "fi",
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
    },
)
