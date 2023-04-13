load("//kythe/docs:asciidoc.bzl", "AsciidocInfo")
load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//lib:shell.bzl", "shell")

_AsciidocHeaderInfo = provider(
    fields = {"header": "File with the asciidoc header."},
)

_SiteDocsInfo = provider()

def _header_impl(target, ctx):
    src = ctx.rule.file.src
    header = ctx.actions.declare_file(paths.replace_extension(src.path, "head." + src.extension))
    ctx.actions.run(
        inputs = [src],
        outputs = [header],
        tools = [ctx.executable._docheader],
        executable = ctx.executable._docheader,
        arguments = [src.path, header.path],
        mnemonic = "JekyllHeader",
    )

    return [
        _AsciidocHeaderInfo(header = header),
    ]

_header_aspect = aspect(
    implementation = _header_impl,
    attrs = {
        "_docheader": attr.label(
            default = Label("//kythe/web/site:doc_header"),
            executable = True,
            cfg = "exec",
        ),
    },
)

def _impl(ctx):
    outdir = ctx.actions.declare_directory(ctx.label.name)
    commands = []
    inputs = []
    for src in ctx.attr.srcs:
        header = src[_AsciidocHeaderInfo].header
        html = src[AsciidocInfo].primary_output_path
        resources = src[AsciidocInfo].resource_dir
        inputs += [resources, header]
        commands += [
            # Copy only the files from the resource dir, omitting the html file itself
            # or we will get subsequent permissions problems.
            "find {resource_dir} -mindepth 1 -maxdepth 1 -depth -not -path {html} -exec cp -L -r {{}} {outdir} \\;".format(
                resource_dir = shell.quote(resources.path),
                outdir = shell.quote(outdir.path),
                html = shell.quote(paths.join(resources.path, html)),
            ),
            "cat {header} {html} > {output}".format(
                header = shell.quote(header.path),
                html = shell.quote(paths.join(resources.path, html)),
                output = shell.quote(paths.join(outdir.path, html)),
            ),
        ]
    for dep in ctx.attr.deps:
        files = dep.files.to_list()
        inputs += files
        commands += [
            "cp -L -r {file} {outdir}".format(
                file = shell.quote(file.path),
                outdir = shell.quote(outdir.path),
            )
            for file in files
        ]

    commands.append("pushd {outdir}".format(outdir = shell.quote(outdir.path)))
    for src, dest in ctx.attr.rename_files.items():
        commands.append("mv {src} {dest}".format(
            src = shell.quote(src),
            dest = shell.quote(dest),
        ))
    commands.append("popd")

    ctx.actions.run_shell(
        mnemonic = "BuildDocs",
        inputs = inputs,
        outputs = [outdir],
        command = "\n".join([
            "set -e",
            "mkdir -p {outdir}".format(outdir = shell.quote(outdir.path)),
        ] + commands),
    )

    return [
        # Only include the root directory in our declared outputs.
        # This ensure that downstream rules don't see files listed twice if the expand tree artifacts.
        DefaultInfo(files = depset([outdir])),
        _SiteDocsInfo(),
    ]

site_docs = rule(
    implementation = _impl,
    attrs = {
        "srcs": attr.label_list(
            aspects = [_header_aspect],
            providers = [AsciidocInfo],
        ),
        "deps": attr.label_list(
            providers = [_SiteDocsInfo],
        ),
        "rename_files": attr.string_dict(),
    },
)

def _package_path(file):
    pkgroot = paths.join(file.root.path, file.owner.package)
    return paths.relativize(file.path, pkgroot)

def _jekyll_impl(ctx):
    input_root = ctx.label.name + ".staging.d"
    symlinks = []
    for src in ctx.files.srcs:
        declare_output = ctx.actions.declare_directory if src.is_directory else ctx.actions.declare_file
        symlink = declare_output(paths.join(input_root, _package_path(src)))
        ctx.actions.symlink(output = symlink, target_file = src)
        symlinks.append(symlink)
    input_dir = paths.join(symlinks[0].root.path, symlinks[0].owner.package, input_root)

    outdir = ctx.outputs.out
    if not outdir:
        outdir = ctx.actions.declare_directory("_site")

    # Dummy command for now.
    args = ctx.actions.args()
    args.add("build")
    args.add("-s", input_dir)
    args.add("-d", outdir.path)
    ctx.actions.run(
        outputs = [outdir],
        inputs = symlinks,
        arguments = [args],
        executable = ctx.executable._jekyll,
        # TODO(shahms): We don't currently have a Ruby toolchain in the RBE environment.
        execution_requirements = {"local": ""},
        mnemonic = "JekyllBuild",
    )

    return [
        DefaultInfo(files = depset([outdir])),
    ]

jekyll_build = rule(
    implementation = _jekyll_impl,
    attrs = {
        "out": attr.output(),
        "srcs": attr.label_list(
            allow_files = True,
        ),
        "_jekyll": attr.label(
            default = "@website_bundle//:bin/jekyll",
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
    },
)
