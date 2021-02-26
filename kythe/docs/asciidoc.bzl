load("@bazel_skylib//lib:shell.bzl", "shell")
load("@bazel_skylib//lib:paths.bzl", "paths")

AsciidocInfo = provider(
    doc = "Information about the asciidoc-generated files.",
    fields = {
        "primary_output": "File indicating the primary output from the asciidoc command.",
        "resource_dir": "File for the directory containing all of the generated resources.",
    },
)

_toolchain_type = "//tools/build_rules/external_tools:external_tools_toolchain_type"

def _asciidoc_impl(ctx):
    resource_dir = ctx.actions.declare_directory(ctx.label.name + ".d")
    primary_output = ctx.actions.declare_file("{dir}/{name}.html".format(
        dir = resource_dir.basename,
        name = ctx.label.name,
    ))

    # Declared as an output, but not saved as part of the default output group.
    # Build with --output_groups=+asciidoc_logfile to retain.
    logfile = ctx.actions.declare_file(ctx.label.name + ".logfile")

    # Locate the asciidoc binary from the toolchain and construct its args.
    asciidoc = ctx.toolchains[_toolchain_type].asciidoc
    args = ["--backend", "html", "--no-header-footer"]
    for key, value in ctx.attr.attrs.items():
        if value:
            args.append("--attribute=%s=%s" % (key, value))
        else:
            args.append("--attribute=%s!" % (key,))
    if ctx.attr.example_script:
        args.append("--attribute=example_script=" + ctx.file.example_script.path)
    args += ["--conf-file=%s" % c.path for c in ctx.files.confs]
    args += ["-o", primary_output.path]
    args.append(ctx.file.src.path)

    # Get the path where all our necessary tools are located so it can be set
    # to PATH in our run_shell command.
    tool_path = ctx.toolchains[_toolchain_type].path

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
        outputs = [primary_output, resource_dir, logfile],
        arguments = args,
        command = "\n".join([
            "set -e",
            # so we can locate the binaries asciidoc needs
            'export PATH="$PATH:{tool_path}"'.format(tool_path = tool_path),
            # Run asciidoc itself, and fail if it returns nonzero.
            "{asciidoc} \"$@\" 2> >(tee -a {logfile} >&2)".format(
                logfile = shell.quote(logfile.path),
                asciidoc = shell.quote(asciidoc),
            ),
            # The tool succeeded, but now check for error diagnostics.
            'if grep -q -e "filter non-zero exit code" -e "no output from filter" {logfile}; then'.format(
                logfile = shell.quote(logfile.path),
            ),
            "exit 1",
            "fi",
            # Move SVGs to the appropriate directory.
            "find . -name '*.svg' -maxdepth 1 -exec mv '{{}}' {out}/ \\;".format(out = shell.quote(resource_dir.path)),
        ]),
        mnemonic = "RunAsciidoc",
    )
    return [
        DefaultInfo(files = depset([primary_output, resource_dir])),
        OutputGroupInfo(asciidoc_logfile = depset([logfile])),
        AsciidocInfo(primary_output = primary_output, resource_dir = resource_dir),
    ]

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
)
