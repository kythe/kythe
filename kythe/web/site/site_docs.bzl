load("//kythe/docs:asciidoc.bzl", "AsciidocInfo")
load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//lib:shell.bzl", "shell")

_AsciidocHeaderInfo = provider(
    fields = {"header": "File with the asciidoc header."},
)

def _header_impl(target, ctx):
    src = ctx.rule.file.src
    header = ctx.actions.declare_file(paths.replace_extension(src.path, "head." + src.extension))
    ctx.actions.run(
        inputs = [src],
        outputs = [header],
        tools = [ctx.executable._docheader],
        executable = ctx.executable._docheader,
        arguments = [src.path, header.path],
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
    intermediates = []
    for src in ctx.attr.srcs:
        header = src[_AsciidocHeaderInfo].header
        html = src[AsciidocInfo].primary_output
        resources = src[AsciidocInfo].resource_dir
        tmpdir = ctx.actions.declare_directory(src.label.name + ".tmp.d")
        ctx.actions.run_shell(
            inputs = [resources, html, header],
            outputs = [tmpdir],
            command = "\n".join([
                "set -e",
                # Copy only the files from the resource dir, omitting the html file itself
                # or we will get subsequent permissions problems.
                "find {resource_dir} -mindepth 1 -not -name {html} -exec cp {{}} {outdir} \\;".format(
                    resource_dir = shell.quote(resources.path),
                    outdir = shell.quote(tmpdir.path),
                    html = shell.quote(html.basename),
                ),
                "cat {header} {html} > {output}".format(
                    header = shell.quote(header.path),
                    html = shell.quote(html.path),
                    output = shell.quote(paths.join(tmpdir.path, html.basename)),
                ),
            ]),
        )
        intermediates.append(tmpdir)
    for dep in ctx.attr.deps:
        staging = ctx.actions.declare_directory("{root}.staging.d/{name}".format(root = ctx.label.name, name = dep.label.name))
        args = ctx.actions.args()
        args.add_all(dep.files)

        ctx.actions.run_shell(
            # We use an additional directory so it is retained when we copy the staging
            # directory below.
            command = "mkdir -p {destdir} && cp \"$@\" {destdir}".format(
                destdir = shell.quote("{root}/{name}".format(
                    root = staging.path,
                    name = dep.label.name,
                )),
            ),
            arguments = [args],
            inputs = dep.files,
            outputs = [staging],
        )
        intermediates.append(staging)

    outdir = None
    if intermediates:
        outdir = ctx.actions.declare_directory(ctx.label.name)
        outputs = [outdir]
        renames = []
        for src, dest in ctx.attr.rename_files.items():
            # Declaring this file ensures that intermediate directories are created, if necessary.
            # It also means Bazel will issue a warning if we fail to create it.
            outputs.append(ctx.actions.declare_file(paths.join(ctx.label.name, dest)))
            renames.append("mv {src} {dest}".format(
                src = shell.quote(src),
                dest = shell.quote(dest),
            ))

        # Outputs which are a prefix of another output can only be output by a single action,
        # so bundle all of the results into intermediate directories above and copy them into place here.
        ctx.actions.run_shell(
            inputs = intermediates,
            outputs = outputs,
            command = "\n".join([
                "set -e",
                "find \"$@\" -mindepth 1 -maxdepth 1 -depth -exec cp -L -r {{}} {outdir} \\;".format(outdir = shell.quote(outdir.path)),
                # Change to the output directory so the subsequent renames remain local.
                "cd {outdir}".format(outdir = shell.quote(outdir.path)),
            ] + renames),
            arguments = [d.path for d in intermediates],
        )

    return [
        # Only include the root directory in our declared outputs.
        # This ensure that downstream rules don't see files listed twice if the expand tree artifacts.
        DefaultInfo(files = depset([outdir] if outdir else [])),
    ]

site_docs = rule(
    implementation = _impl,
    attrs = {
        "srcs": attr.label_list(
            aspects = [_header_aspect],
            providers = [AsciidocInfo],
        ),
        "deps": attr.label_list(),
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
        symlink = ctx.actions.declare_file(paths.join(input_root, _package_path(src)))
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
