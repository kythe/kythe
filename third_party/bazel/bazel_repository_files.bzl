"""Repository rule for downloading selected files from Bazel's github repository."""

_URL_TEMPLATE = "https://raw.githubusercontent.com/bazelbuild/bazel/{commit}/{path}"

def _impl(repository_ctx):
    commit = repository_ctx.attr.commit
    for path in repository_ctx.attr.files:
        repository_ctx.download(
            url = _URL_TEMPLATE.format(commit = commit, path = path),
            output = path,
            canonical_id = "{commit}/{path}".format(commit = commit, path = path),
        )

    for src, dest in repository_ctx.attr.overlay.items():
        repository_ctx.symlink(src, dest)

bazel_repository_files = repository_rule(
    implementation = _impl,
    attrs = {
        "commit": attr.string(mandatory = True),
        "files": attr.string_list(mandatory = True),
        "overlay": attr.label_keyed_string_dict(),
    },
)
