workspace(
    name = "io_kythe",
    managed_directories = {"@npm": ["node_modules"]},
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//:version.bzl", "MAX_VERSION", "MIN_VERSION", "check_version")

# Check that the user has a version between our minimum supported version of
# Bazel and our maximum supported version of Bazel.
check_version(MIN_VERSION, MAX_VERSION)

http_archive(
    name = "bazel_toolchains",
    sha256 = "1342f84d4324987f63307eb6a5aac2dff6d27967860a129f5cd40f8f9b6fd7dd",
    strip_prefix = "bazel-toolchains-2.2.0",
    urls = [
        "https://github.com/bazelbuild/bazel-toolchains/releases/download/2.2.0/bazel-toolchains-2.2.0.tar.gz",
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/releases/download/2.2.0/bazel-toolchains-2.2.0.tar.gz",
    ],
)

load("//:setup.bzl", "kythe_rule_repositories", "maybe")

kythe_rule_repositories()

# gazelle:repository_macro external.bzl%_go_dependencies
load("//:external.bzl", "kythe_dependencies")

kythe_dependencies()

load("//tools/build_rules/external_tools:external_tools_configure.bzl", "external_tools_configure")

external_tools_configure()

load("@build_bazel_rules_nodejs//:index.bzl", "npm_install")

npm_install(
    name = "npm",
    package_json = "//:package.json",
    package_lock_json = "//:package-lock.json",
)

load("@npm//:install_bazel_dependencies.bzl", "install_bazel_dependencies")

install_bazel_dependencies()

load("@npm_bazel_typescript//:index.bzl", "ts_setup_workspace")

ts_setup_workspace()

# This binding is needed for protobuf. See https://github.com/protocolbuffers/protobuf/pull/5811
bind(
    name = "error_prone_annotations",
    actual = "@maven//:com_google_errorprone_error_prone_annotations",
)

load("@maven//:compat.bzl", "compat_repositories")

compat_repositories()

# If the configuration here changes, run tools/platforms/configs/rebuild.sh
load("@bazel_toolchains//rules:environments.bzl", "clang_env")
load("@bazel_toolchains//rules:rbe_repo.bzl", "rbe_autoconfig")
load("//tools/platforms:toolchain_config_suite_spec.bzl", "DEFAULT_TOOLCHAIN_CONFIG_SUITE_SPEC")

rbe_autoconfig(
    name = "rbe_default",
    env = clang_env(),
    export_configs = True,
    toolchain_config_suite_spec = DEFAULT_TOOLCHAIN_CONFIG_SUITE_SPEC,
    use_legacy_platform_definition = False,
)

rbe_autoconfig(
    name = "rbe_bazel_minversion",
    bazel_version = MIN_VERSION,
    env = clang_env(),
    export_configs = True,
    toolchain_config_suite_spec = DEFAULT_TOOLCHAIN_CONFIG_SUITE_SPEC,
    use_legacy_platform_definition = False,
)

rbe_autoconfig(
    name = "rbe_bazel_maxversion",
    bazel_version = MAX_VERSION,
    env = clang_env(),
    export_configs = True,
    toolchain_config_suite_spec = DEFAULT_TOOLCHAIN_CONFIG_SUITE_SPEC,
    use_legacy_platform_definition = False,
)
