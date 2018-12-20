load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "io_kythe",
    commit = "01617d9cc0753550964c57af57c64644a0808798",
    remote = "https://github.com/kythe/kythe",
)

load("@io_kythe//:setup.bzl", "kythe_rule_repositories")

kythe_rule_repositories()

load("@io_kythe//:external.bzl", "kythe_dependencies")

kythe_dependencies()
