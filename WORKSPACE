workspace(name = "io_kythe")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

load("//:version.bzl", "MAX_VERSION", "MIN_VERSION", "check_version")

# Check that the user has a version between our minimum supported version of
# Bazel and our maximum supported version of Bazel.
check_version(MIN_VERSION, MAX_VERSION)

load("//:setup.bzl", "kythe_rule_repositories")

kythe_rule_repositories()

###
# BEGIN rules_ts setup
# loads are sensitive to intervening calls, so they need to happen at the
# top-level and not in e.g. a _ts_dependencies() function.
load("@aspect_rules_ts//ts:repositories.bzl", "rules_ts_dependencies")

rules_ts_dependencies(
    ts_integrity = "sha512-6l+RyNy7oAHDfxC4FzSJcz9vnjTKxrLpDG5M2Vu4SHRVNg6xzqZp6LYSR9zjqQTu8DU/f5xwxUdADOkbrIX2gQ==",
    ts_version_from = "//:package.json",
)

load("@aspect_rules_js//js:repositories.bzl", "rules_js_dependencies")

rules_js_dependencies()

load("@bazel_features//:deps.bzl", "bazel_features_deps")

bazel_features_deps()

load("@aspect_rules_jasmine//jasmine:dependencies.bzl", "rules_jasmine_dependencies")

# Fetch dependencies which users need as well
rules_jasmine_dependencies()

# Fetch and register node, if you haven't already
load("@rules_nodejs//nodejs:repositories.bzl", "DEFAULT_NODE_VERSION", "nodejs_register_toolchains")

nodejs_register_toolchains(
    name = "node",
    node_version = DEFAULT_NODE_VERSION,
)

load("@aspect_rules_js//npm:npm_import.bzl", "npm_translate_lock")

npm_translate_lock(
    name = "npm",
    pnpm_lock = "//:pnpm-lock.yaml",
)

load("@npm//:repositories.bzl", "npm_repositories")

npm_repositories()

# END rules_ts setup
##

# gazelle:repository_macro external.bzl%_go_dependencies
load("//:external.bzl", "kythe_dependencies")

kythe_dependencies()

load("//tools/build_rules/external_tools:external_tools_configure.bzl", "external_tools_configure")

external_tools_configure()

load("@maven//:compat.bzl", "compat_repositories")

compat_repositories()

load("@maven//:defs.bzl", "pinned_maven_install")

pinned_maven_install()

load(
    "@bazelruby_rules_ruby//ruby:defs.bzl",
    "ruby_bundle",
)

ruby_bundle(
    name = "website_bundle",
    bundler_version = "2.1.4",
    gemfile = "//kythe/web/site:Gemfile",
    gemfile_lock = "//kythe/web/site:Gemfile.lock",
)

http_archive(
    name = "aspect_bazel_lib",
    sha256 = "d488d8ecca98a4042442a4ae5f1ab0b614f896c0ebf6e3eafff363bcc51c6e62",
    strip_prefix = "bazel-lib-1.33.0",
    url = "https://github.com/aspect-build/bazel-lib/releases/download/v1.33.0/bazel-lib-v1.33.0.tar.gz",
)

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

aspect_bazel_lib_dependencies()

# clang-tidy aspect wrapper
load(
    "@bazel_tools//tools/build_defs/repo:git.bzl",
    "git_repository",
)

git_repository(
    name = "bazel_clang_tidy",
    commit = "133d89a6069ce253a92d32a93fdb7db9ef100e9d",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)

# Tweag Nix Support
######################

# Replace with http_archive: https://github.com/tweag/rules_nixpkgs/#setup
http_archive(
    name = "io_tweag_rules_nixpkgs",
    strip_prefix = "rules_nixpkgs-126e9f66b833337be2c35103ce46ab66b4e44799",
    urls = ["https://github.com/tweag/rules_nixpkgs/archive/126e9f66b833337be2c35103ce46ab66b4e44799.tar.gz"],
    sha256 = "480df4a7777a5e3ee7a755ab38d18ecb3ddb7b2e2435f24ad2037c1b084faa65",
)

load("@io_tweag_rules_nixpkgs//nixpkgs:repositories.bzl", "rules_nixpkgs_dependencies")
rules_nixpkgs_dependencies()

# Define nixpkgs version using the flakes lock file.
load("@io_tweag_rules_nixpkgs//nixpkgs:nixpkgs.bzl", "nixpkgs_local_repository")
nixpkgs_local_repository(
    name = "nixpkgs",
    nix_flake_lock_file = "//:flake.lock",
    nix_file_deps = ["//:flake.lock"],
)

# Configure the C++ toolchain
load("@io_tweag_rules_nixpkgs//nixpkgs:nixpkgs.bzl", "nixpkgs_cc_configure")
nixpkgs_cc_configure(
    name = "nixpkgs_config_cc",
    repository = "@nixpkgs",
    #    attribute_path = "clang_13",
)

load("@rules_cc//cc:repositories.bzl", "rules_cc_dependencies", "rules_cc_toolchains")

rules_cc_dependencies()

rules_cc_toolchains()


## Libraries brought in via nixpkgs.
load("@io_tweag_rules_nixpkgs//nixpkgs:nixpkgs.bzl", "nixpkgs_package")

#nixpkgs_package(
    #name = "uuid",
    #attribute_path = "libossp_uuid",
    #repository = "@nixpkgs",
#)

nixpkgs_package(
    name = "libossp_uuid.dev",
    attribute_path = "libossp_uuid",
    build_file_content = """\
load("@rules_cc//cc:defs.bzl", "cc_library")
filegroup(
    name = "include",
    srcs = glob(["include/**/*.h"]),
    visibility = ["//visibility:public"],
)
filegroup(
    name = "libs",
    srcs = glob(["lib/**/*"]),
    visibility = ["//visibility:public"],
)
cc_library(
    name = "lib",
    srcs = [":libs"],
    hdrs = [":include"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
""",
    repository = "@nixpkgs",
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "bazel_local_nix",
	remote = "https://github.com/filmil/bazel_local_nix",
	commit = "d2daf82dfa7dc1ff7eafe12fa91c19b8fa417f15",
)

# Installation.
load("@bazel_local_nix//:repositories.bzl", "bazel_local_nix_dependencies")
bazel_local_nix_dependencies()

