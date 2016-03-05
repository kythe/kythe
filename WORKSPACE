load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

load("//tools/go:go_configure.bzl", "go_configure")

go_configure()

new_git_repository(
    name = "gtest",
    build_file = "third_party/googletest.BUILD",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.7.0",
)

bind(
    name = "googletest",
    actual = "@gtest//:googletest",
)

new_git_repository(
    name = "googleflags",
    build_file = "third_party/googleflags.BUILD",
    commit = "58345b18d92892a170d61a76c5dd2d290413bdd7",
    remote = "https://github.com/gflags/gflags.git",
)

bind(
    name = "gflags",
    actual = "@googleflags//:gflags",
)

git_repository(
    name = "re2repo",
    commit = "63e4dbd1996d2cf723f740e08521a67ad66a09de",
    remote = "https://code.googlesource.com/re2",
)

bind(
    name = "re2",
    actual = "@re2repo//:re2",
)

bind(
    name = "go_package_prefix",
    actual = "//:go_package_prefix",
)

bind(
    name = "android/sdk",
    actual = "//:nothing",
)
