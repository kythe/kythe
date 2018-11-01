# Kythe extraction setup
bind(
    name = "vnames_config",
    actual = "@io_kythe//kythe/examples/bazel:kythe_config.json",
)

# Build against Kythe master.  Run `bazel sync` to update to the latest commit.
http_archive(
    name = "io_kythe",
    strip_prefix = "kythe-master",
    urls = ["https://github.com/google/kythe/archive/master.zip"],
)

load("@io_kythe//:setup.bzl", "kythe_rule_repositories")

kythe_rule_repositories()

load("@io_kythe//:external.bzl", "kythe_dependencies")

kythe_dependencies()
