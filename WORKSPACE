workspace(name = "io_kythe")

load("//:version.bzl", "check_version")

check_version("0.2.0-2016-03-21 (@19b5675)")

load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

load("//tools/go:go_configure.bzl", "go_configure")

go_configure()

load("//tools:node_configure.bzl", "node_configure")

node_configure()

load("//tools:kythe_pkg_config.bzl", "kythe_pkg_config")

kythe_pkg_config(
    name = "libcrypto",
    darwin = ("OPENSSL_HOME", "/usr/local/opt/openssl"),
)

kythe_pkg_config(
    name = "libuuid",
    darwin = ("UUID_HOME", "/usr/local/opt/ossp-uuid"),
    modname = "uuid",
)

kythe_pkg_config(
    name = "libmemcached",
    darwin = ("MEMCACHED_HOME", "/usr/local/opt/libmemcached"),
)

new_git_repository(
    name = "com_github_google_googletest",
    build_file = "third_party/googletest.BUILD",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.7.0",
)

bind(
    name = "googletest",
    actual = "@com_github_google_googletest//:googletest",
)

bind(
    name = "googletest/license",
    actual = "@com_github_google_googletest//:license",
)

new_git_repository(
    name = "com_github_gflags_gflags",
    build_file = "third_party/googleflags.BUILD",
    commit = "58345b18d92892a170d61a76c5dd2d290413bdd7",
    remote = "https://github.com/gflags/gflags.git",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

bind(
    name = "gflags/license",
    actual = "@com_github_gflags_gflags//:license",
)

git_repository(
    name = "com_googlesource_code_re2",
    commit = "63e4dbd1996d2cf723f740e08521a67ad66a09de",
    remote = "https://code.googlesource.com/re2",
)

bind(
    name = "re2",
    actual = "@com_googlesource_code_re2//:re2",
)

bind(
    name = "re2/license",
    actual = "@com_googlesource_code_re2//:LICENSE",
)

new_git_repository(
    name = "com_github_google_glog",
    build_file = "third_party/googlelog.BUILD",
    commit = "1b0b08c8dda1659027677966b03a3ff3c488e549",
    remote = "https://github.com/google/glog.git",
)

bind(
    name = "glog",
    actual = "@com_github_google_glog//:glog",
)

bind(
    name = "glog/license",
    actual = "@com_github_google_glog//:license",
)

bind(
    name = "go_package_prefix",
    actual = "//:go_package_prefix",
)

bind(
    name = "android/sdk",
    actual = "//:nothing",
)
