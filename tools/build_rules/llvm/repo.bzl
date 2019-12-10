def _git(repository_ctx):
    commit = repository_ctx.attr._commit
    url = "https://github.com/llvm/llvm-project/archive/%s.zip" % (commit,)
    prefix = "llvm-project-" + commit
    sha256 = repository_ctx.download_and_extract(
        url,
        sha256 = repository_ctx.attr._sha256,
        canonical_id = commit,
    ).sha256

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

    return {"_commit": commit, "name": repository_ctx.name, "_sha256": sha256}

git_llvm_repository = repository_rule(
    implementation = _git,
    attrs = {
        "_commit": attr.string(
            default = "9c6b7f68b807250e7c3aa01938339fdbd239c4ea",
        ),
        "_sha256": attr.string(
            # Make sure to update this along with the commit as its presence will cache the download,
            # even if the rules or commit change.
            default = "640dfe8129802f46e07958bc8590e1fe3df1d91d9c8f9be677b1a7e6204d0c45",
        ),
    },
)

def local_llvm_repository(name, path):
    native.new_local_repository(
        name = name,
        path = path,
        build_file = "//tools/build_rules/llvm:llvm.BUILD",
    )
