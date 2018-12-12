load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "io_kythe",
    commit = "91da8b42354cd3b6818be5a9bf4389fd144ff6e5",
    remote = "https://github.com/kythe/kythe",
)

load("@io_kythe//:setup.bzl", "kythe_rule_repositories")

kythe_rule_repositories()

load("@io_kythe//:external.bzl", "kythe_dependencies")

kythe_dependencies()
