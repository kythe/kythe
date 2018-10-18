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
        url = "https://github.com/bazelbuild/rules_go/releases/download/0.16.0/rules_go-0.16.0.tar.gz",
        sha256 = "ee5fe78fe417c685ecb77a0a725dc9f6040ae5beb44a0ba4ddb55453aad23a8a",
    )

    maybe(
        http_archive,
        name = "bazel_gazelle",
        strip_prefix = "bazel-gazelle-253128b77088080a348f54d79a28dcd47d99caf9",
        urls = ["https://github.com/bazelbuild/bazel-gazelle/archive/253128b77088080a348f54d79a28dcd47d99caf9.zip"],
    )
