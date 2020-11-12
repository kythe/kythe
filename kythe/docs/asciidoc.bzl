load("@bazel_skylib//lib:shell.bzl", "shell")

_toolchain_type = "//tools/build_rules/external_tools:external_tools_toolchain_type"

def _asciidoc_impl(ctx):
    # Locate the asciidoc binary from the toolchain and construct its args.
    # Because an asciidoc run produces an unknown number of files from the
    # execution of filters, the output is staged to a temporary directory
    # and packaged into a .zip file.
    asciidoc = ctx.toolchains[_toolchain_type].asciidoc
    args = ["--backend", "html", "--no-header-footer"]
    for key, value in ctx.attr.attrs.items():
        if value:
            args += ["--attribute=%s=%s" % (key, value)]
        else:
            args += ["--attribute=%s!" % (key,)]
    if ctx.attr.example_script:
        args += ["--attribute=example_script=" + ctx.file.example_script.path]
    args += ["--conf-file=%s" % c.path for c in ctx.files.confs]
    args += ["-o", "out/" + ctx.attr.name + ".html"]
    args += [ctx.file.src.path]

    # Get the path where all our necessary tools are located so it can be set
    # to PATH in our run_shell command.
    tool_path = ctx.toolchains[_toolchain_type].path

    # Declare the logfile as an output so that it can be read if something goes
    # awry (otherwise Bazel will clean it up).
    logfile = ctx.actions.declare_file(ctx.attr.name + ".logfile")

    # Resolve data targets to get input files and runfiles manifests.
    data, _, manifests = ctx.resolve_command(tools = ctx.attr.data)

    # Run asciidoc and capture stderr to logfile. If it succeeds, look in the
    # captured log for error messages and fail if we find any.
    ctx.actions.run_shell(
        inputs = ([ctx.file.src] +
                  ctx.files.confs +
                  ([ctx.file.example_script] if ctx.file.example_script else []) +
                  data),
        input_manifests = manifests,
        outputs = [ctx.outputs.out, logfile],
        command = "\n".join([
            # so we can locate the binaries asciidoc needs
            'export PATH="$PATH:' + tool_path + '"',

            # Create the temporary staging directory.
            "mkdir out",

            # Run asciidoc itself, and fail if it returns nonzero.
            "%s %s 2> %s" % (
                shell.quote(asciidoc),
                " ".join([shell.quote(arg) for arg in args]),
                shell.quote(logfile.path),
            ),
            "if [[ $? -ne 0 ]]; then",
            "exit 1",
            "fi",

            # The tool succeeded, but now check for error diagnostics.
            "cat %s" % (shell.quote(logfile.path)),
            'grep -q -e "filter non-zero exit code" -e "no output from filter" %s' % (
                shell.quote(logfile.path)
            ),
            "if [[ $? -ne 1 ]]; then",
            "exit 1",
            "fi",

            # Move any generated images to the out directory.
            "find . -name '*.svg' -maxdepth 1 -exec mv '{}' out/ \\;",

            # Package up the outputs into the zip file.
            "(cd out; zip -9qr ../%s *)" % ctx.outputs.out.path,
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
        "out": "%{name}.zip",
    },
)
