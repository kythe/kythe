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
        sha256 = "6776d68ebb897625dead17ae510eac3d5f6342367327875210df44dbe2aeeb19",
        urls = ["https://github.com/bazelbuild/rules_go/releases/download/0.17.1/rules_go-0.17.1.tar.gz"],
    )

    maybe(
        http_archive,
        name = "bazel_gazelle",
        sha256 = "3c681998538231a2d24d0c07ed5a7658cb72bfb5fd4bf9911157c0e9ac6a2687",
        urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.17.0/bazel-gazelle-0.17.0.tar.gz"],
    )

    maybe(
        git_repository,
        name = "build_bazel_rules_nodejs",
        remote = "https://github.com/bazelbuild/rules_nodejs.git",
        tag = "0.16.2",
    )
