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
        name = "bazel_skylib",
        sha256 = "2ea8a5ed2b448baf4a6855d3ce049c4c452a6470b1efd1504fdb7c1c134d220a",
        strip_prefix = "bazel-skylib-0.8.0",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/0.8.0.tar.gz"],
    )

    maybe(
        http_archive,
        name = "io_bazel_rules_go",
        url = "https://github.com/bazelbuild/rules_go/releases/download/0.19.4/rules_go-0.19.4.tar.gz",
        sha256 = "ae8c36ff6e565f674c7a3692d6a9ea1096e4c1ade497272c2108a810fb39acd2",
    )

    maybe(
        http_archive,
        name = "rules_java",
        url = "https://github.com/bazelbuild/rules_java/archive/973a93dd2d698929264d1028836f6b9cc60ff817.zip",
        sha256 = "a6cb0dbe343b67c7d4f3f11a68e327acdfc71fee1e17affa4e605129fc56bb15",
        strip_prefix = "rules_java-973a93dd2d698929264d1028836f6b9cc60ff817",
    )

    maybe(
        http_archive,
        name = "rules_proto",
        sha256 = "e4fe70af52135d2ee592a07f916e6e1fc7c94cf8786c15e8c0d0f08b1fe5ea16",
        strip_prefix = "rules_proto-97d8af4dc474595af3900dd85cb3a29ad28cc313",
        url = "https://github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.zip",
    )

    maybe(
        http_archive,
        name = "bazel_gazelle",
        sha256 = "7fc87f4170011201b1690326e8c16c5d802836e3a0d617d8f75c3af2b23180c4",
        urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.18.2/bazel-gazelle-0.18.2.tar.gz"],
    )

    maybe(
        http_archive,
        name = "build_bazel_rules_nodejs",
        sha256 = "9901bc17138a79135048fb0c107ee7a56e91815ec6594c08cb9a17b80276d62b",
        urls = ["https://github.com/bazelbuild/rules_nodejs/releases/download/0.40.0/rules_nodejs-0.40.0.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_jvm_external",
        sha256 = "e5b97a31a3e8feed91636f42e19b11c49487b85e5de2f387c999ea14d77c7f45",
        strip_prefix = "rules_jvm_external-2.9",
        urls = ["https://github.com/bazelbuild/rules_jvm_external/archive/2.9.zip"],
    )
