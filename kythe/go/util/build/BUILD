load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "build",
    srcs = ["build.go"],
    importpath = "kythe.io/kythe/go/util/build",
    x_defs = {
        "kythe.io/kythe/go/util/build._BUILD_SCM_REVISION": "{STABLE_BUILD_SCM_REVISION}",
        "kythe.io/kythe/go/util/build._BUILD_SCM_STATUS": "{STABLE_BUILD_SCM_STATUS}",
        "kythe.io/kythe/go/util/build._KYTHE_RELEASE_VERSION": "{STABLE_KYTHE_RELEASE_VERSION}",
    },
)
