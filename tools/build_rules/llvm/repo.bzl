def _git(repository_ctx):
    commit = repository_ctx.attr._commit
    url = "https://github.com/llvm/llvm-project/archive/%s.zip" % (commit,)
    prefix = "llvm-project-" + commit
    repository_ctx.download_and_extract(url, sha256 = repository_ctx.attr._sha256)

    # Move clang into place.
    repository_ctx.execute(["mv", prefix + "/clang", prefix + "/llvm/tools/"])

    # Re-parent llvm as the top-level directory.
    repository_ctx.execute([
        "find",
        prefix + "/llvm",
        "-mindepth",
        "1",
        "-maxdepth",
        "1",
        "-exec",
        "mv",
        "{}",
        ".",
        ";",
    ])

    # Remove the detritus.
    repository_ctx.execute(["rm", "-rf", prefix])
    repository_ctx.execute(["rmdir", "llvm"])

    # Add workspace files.
    repository_ctx.symlink(Label("@io_kythe//tools/build_rules/llvm:llvm.BUILD"), "BUILD.bazel")
    repository_ctx.file(
        "WORKSPACE",
        "workspace(name = \"%s\")\n" % (repository_ctx.name,),
    )

    return {"_commit": commit, "name": repository_ctx.name}

git_llvm_repository = repository_rule(
    implementation = _git,
    attrs = {
        "_commit": attr.string(
            default = "a10fe832fdf742d3d9631900ef173af4222d3c9f",
        ),
        "_sha256": attr.string(
            # Make sure to update this along with the commit as its presence will cache the download,
            # even if the rules or commit change.
            default = "5f3cbc5f3eac12995937ce6bc1e67e568ad19ca9c55468894b4375203990d1a9",
        ),
    },
)

def local_llvm_repository(name, path):
    native.new_local_repository(
        name = name,
        path = path,
        build_file = "//tools/build_rules/llvm:llvm.BUILD",
    )
