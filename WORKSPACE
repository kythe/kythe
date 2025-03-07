workspace(name = "io_kythe")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load("//:version.bzl", "MAX_VERSION", "MIN_VERSION", "check_version")

http_archive(
    name = "bazel_features",
    sha256 = "cec7fbc7bce6597cf2e83e01ddd9328a1bb057dc1a3092745238f49d3301ab5a",
    strip_prefix = "bazel_features-1.12.0",
    url = "https://github.com/bazel-contrib/bazel_features/releases/download/v1.12.0/bazel_features-v1.12.0.tar.gz",
)

http_archive(
    name = "rules_shell",
    sha256 = "d8cd4a3a91fc1dc68d4c7d6b655f09def109f7186437e3f50a9b60ab436a0c53",
    strip_prefix = "rules_shell-0.3.0",
    url = "https://github.com/bazelbuild/rules_shell/releases/download/v0.3.0/rules_shell-v0.3.0.tar.gz",
)

http_archive(
    name = "rules_cc",
    urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.17/rules_cc-0.0.17.tar.gz"],
    sha256 = "abc605dd850f813bb37004b77db20106a19311a96b2da1c92b789da529d28fe1",
    strip_prefix = "rules_cc-0.0.17",
)

http_archive(
    name = "com_google_protobuf",
    integrity = "sha256-6bmsGRCxBBBlg5hQYDyvNuKdPT0jDd9SvRN3jdMbkEY=",
    strip_prefix = "protobuf-29.3",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v29.3/protobuf-29.3.zip"],
)

http_archive(
    name = "rules_java",
    urls = [
        "https://github.com/bazelbuild/rules_java/releases/download/8.10.0/rules_java-8.10.0.tar.gz",
    ],
    sha256 = "476bd403f284e5080037f1910a29ce4c482ac798a3560c2e0df6d6f1857011b6",
)

http_archive(
    name = "rules_python",
    sha256 = "2ef40fdcd797e07f0b6abda446d1d84e2d9570d234fddf8fcd2aa262da852d1c",
    strip_prefix = "rules_python-1.2.0",
    url = "https://github.com/bazelbuild/rules_python/releases/download/1.2.0/rules_python-1.2.0.tar.gz",
)

http_archive(
    name = "rules_proto",
    sha256 = "14a225870ab4e91869652cfd69ef2028277fc1dc4910d65d353b62d6e0ae21f4",
    strip_prefix = "rules_proto-7.1.0",
    url = "https://github.com/bazelbuild/rules_proto/releases/download/7.1.0/rules_proto-7.1.0.tar.gz",
)

# gazelle:repository go_repository name=org_golang_x_tools importpath=golang.org/x/tools
http_archive(
    name = "org_golang_x_tools",
    # Must be kept in sync with rules_go or the patches may fail.
    # v0.7.0, latest as of 2023-03-27
    urls = [
        "https://mirror.bazel.build/github.com/golang/tools/archive/refs/tags/v0.30.0.zip",
        "https://github.com/golang/tools/archive/refs/tags/v0.30.0.zip",
    ],
    # sha256 = "9f20a20f29f4008d797a8be882ef82b69cf8f7f2b96dbdfe3814c57d8280fa4b",
    strip_prefix = "tools-0.30.0",
    patches = [
        "@io_kythe//third_party/go:add_export_license.patch",
        # deletegopls removes the gopls subdirectory. It contains a nested
        # module with additional dependencies. It's not needed by rules_go.
        # releaser:patch-cmd rm -rf gopls
        "@io_bazel_rules_go//third_party:org_golang_x_tools-deletegopls.patch",
        # releaser:patch-cmd gazelle -repo_root . -go_prefix golang.org/x/tools -go_naming_convention import_alias
        "@io_bazel_rules_go//third_party:org_golang_x_tools-gazelle.patch",
    ],
    patch_args = ["-p1"],
    integrity = "sha256-BzaxoKoo9IB0iRoPk871OWV129c7m1zcTeVLKjv6S0s=",
)

http_archive(
    name = "aspect_bazel_lib",
    sha256 = "40ba9d0f62deac87195723f0f891a9803a7b720d7b89206981ca5570ef9df15b",
    strip_prefix = "bazel-lib-2.14.0",
    url = "https://github.com/aspect-build/bazel-lib/releases/download/v2.14.0/bazel-lib-v2.14.0.tar.gz",
)

load("@rules_shell//shell:repositories.bzl", "rules_shell_dependencies", "rules_shell_toolchains")

rules_shell_dependencies()

rules_shell_toolchains()

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps", "PROTOBUF_MAVEN_ARTIFACTS")

protobuf_deps()

load("@rules_java//java:rules_java_deps.bzl", "compatibility_proxy_repo")
compatibility_proxy_repo()

load("@rules_jvm_external//:defs.bzl", "maven_install")
maven_install(
    name = "protobuf_maven",
    artifacts = PROTOBUF_MAVEN_ARTIFACTS,
    repositories = [
        "https://repo1.maven.org/maven2",
        "https://repo.maven.apache.org/maven2",
    ],
)

load("@com_google_protobuf//bazel/private:proto_bazel_features.bzl", "proto_bazel_features")  # buildifier: disable=bzl-visibility
proto_bazel_features(name = "proto_bazel_features")

load("@rules_java//java:repositories.bzl", "rules_java_toolchains")
rules_java_toolchains()

load("@rules_python//python:repositories.bzl", "py_repositories")

py_repositories()

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")

rules_proto_dependencies()

load("@rules_proto//proto:toolchains.bzl", "rules_proto_toolchains")

rules_proto_toolchains()

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies", "aspect_bazel_lib_register_toolchains")

aspect_bazel_lib_dependencies()

aspect_bazel_lib_register_toolchains()

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

load("@aspect_rules_js//npm:repositories.bzl", "npm_translate_lock")

npm_translate_lock(
    name = "npm",
    pnpm_lock = "//:pnpm-lock.yaml",
)

load("@npm//:repositories.bzl", "npm_repositories")

npm_repositories()

# END rules_ts setup
###

load("@io_bazel_rules_go//go:deps.bzl", "go_download_sdk", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

GO_SDK_VERSION = "1.24.1"
# Register multiple Go SDKs so that we can perform cross-compilation remotely.
# i.e. We might want to trigger a Linux AMD64 Go build remotely from a MacOS ARM64 laptop.
#
# Reference: https://github.com/bazelbuild/rules_go/issues/3540.
go_download_sdk(
    name = "go_sdk_linux",
    goarch = "amd64",
    goos = "linux",
    version = GO_SDK_VERSION,
)
go_download_sdk(
    name = "go_sdk_linux_arm64",
    goarch = "arm64",
    goos = "linux",
    version = GO_SDK_VERSION,
)
go_download_sdk(
    name = "go_sdk_darwin",
    goarch = "amd64",
    goos = "darwin",
    version = GO_SDK_VERSION,
)
go_download_sdk(
    name = "go_sdk_darwin_arm64",
    goarch = "arm64",
    goos = "darwin",
    version = GO_SDK_VERSION,
)
go_download_sdk(
    name = "go_sdk_windows",
    goarch = "amd64",
    goos = "windows",
    version = GO_SDK_VERSION,
)
go_download_sdk(
    name = "go_sdk_windows_arm64",
    goarch = "arm64",
    goos = "windows",
    version = GO_SDK_VERSION,
)

go_register_toolchains()

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

http_archive(
    name = "musl_toolchains",
    sha256 = "26cacffab74e10f0840c83b0be9193dc6deccfbc8ec1cf6f8362dc61d0057fa1",
    url = "https://github.com/bazel-contrib/musl-toolchain/releases/download/v0.1.15/musl_toolchain-v0.1.15.tar.gz",
)

load("@musl_toolchains//:repositories.bzl", "load_musl_toolchains")

load_musl_toolchains(
    extra_target_compatible_with = [
        "@//toolchains:musl_on",
    ],
)

load("@musl_toolchains//:toolchains.bzl", "register_musl_toolchains")

register_musl_toolchains()
