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
        sha256 = "7519e9e1c716ae3c05bd2d984a42c3b02e690c5df728dc0a84b23f90c355c5a1",
        urls = ["https://github.com/bazelbuild/rules_go/releases/download/0.15.4/rules_go-0.15.4.tar.gz"],
    )

    maybe(
        http_archive,
        name = "bazel_gazelle",
        sha256 = "c0a5739d12c6d05b6c1ad56f2200cb0b57c5a70e03ebd2f7b87ce88cabf09c7b",
        urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.14.0/bazel-gazelle-0.14.0.tar.gz"],
    )
