load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def maybe(repo_rule, name, **kwargs):
    """Defines a repository if it does not already exist.
    """
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

def kythe_rule_repositories():
    """Defines external repositories for Kythe Bazel rules.

    These repositories must be loaded before calling external.bzl%kythe_dependencies.
    """
    maybe(
        http_archive,
        name = "io_bazel_rules_go",
        url = "https://github.com/bazelbuild/rules_go/releases/download/0.16.1/rules_go-0.16.1.tar.gz",
        sha256 = "f5127a8f911468cd0b2d7a141f17253db81177523e4429796e14d429f5444f5f",
    )

    maybe(
        http_archive,
        name = "bazel_gazelle",
        strip_prefix = "bazel-gazelle-253128b77088080a348f54d79a28dcd47d99caf9",
        sha256 = "6e48a5f804ee1f0df84b546aa5c2eb15b3b2e2bcfc75f2bf305323343c2e8b94",
        urls = ["https://github.com/bazelbuild/bazel-gazelle/archive/253128b77088080a348f54d79a28dcd47d99caf9.zip"],
    )

    maybe(
        git_repository,
        name = "build_bazel_rules_nodejs",
        remote = "https://github.com/bazelbuild/rules_nodejs.git",
        tag = "0.16.0",
    )
