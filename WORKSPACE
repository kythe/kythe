workspace(name = "io_kythe_lang_proto")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_toolchains",
    sha256 = "d3da5e10483e2786452a3bdfe1bc2e3e4185f5292f96a52374a1f9aacf25d308",
    strip_prefix = "bazel-toolchains-4c1acb6eaf4a23580ac2edf56393a69614426399",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/4c1acb6eaf4a23580ac2edf56393a69614426399.tar.gz",
        "https://github.com/bazelbuild/bazel-toolchains/archive/4c1acb6eaf4a23580ac2edf56393a69614426399.tar.gz",
    ],
)

git_repository(
    name = "io_kythe",
    commit = "2f05e066e21f2d4d055aadd34e790b36334e72c8",
    remote = "https://github.com/kythe/kythe",
)

load("@io_kythe//:setup.bzl", "kythe_rule_repositories")

kythe_rule_repositories()

load("@io_kythe//:setup.bzl", "maybe")

# TODO(schroederc): remove this.  This needs to be loaded before loading the
# go_* rules.  Normally, this is done by go_rules_dependencies in external.bzl,
# but because we want to overload some of those dependencies, we need the go_*
# rules before go_rules_dependencies.  Likewise, we can't precisely control
# when loads occur within a Starlark file so we now need to load this
# manually... https://github.com/bazelbuild/rules_go/issues/1966
load("@io_bazel_rules_go//go/private:compat/compat_repo.bzl", "go_rules_compat")

maybe(
    go_rules_compat,
    name = "io_bazel_rules_go_compat",
)

load("@io_kythe//:external.bzl", "kythe_dependencies")

kythe_dependencies()
