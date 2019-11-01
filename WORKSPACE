workspace(
    name = "io_kythe",
    managed_directories = {"@npm": ["node_modules"]},
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("//:version.bzl", "check_version", "MAX_VERSION", "MIN_VERSION")

# Check that the user has a version between our minimum supported version of
# Bazel and our maximum supported version of Bazel.
check_version(MIN_VERSION, MAX_VERSION)

http_archive(
    name = "bazel_toolchains",
    sha256 = "56e75f7c9bb074f35b71a9950917fbd036bd1433f9f5be7c04bace0e68eb804a",
    strip_prefix = "bazel-toolchains-9bd2748ec99d72bec41c88eecc3b7bd19d91a0c7",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/9bd2748ec99d72bec41c88eecc3b7bd19d91a0c7.tar.gz",
        "https://github.com/bazelbuild/bazel-toolchains/archive/9bd2748ec99d72bec41c88eecc3b7bd19d91a0c7.tar.gz",
    ],
)

load("//:setup.bzl", "kythe_rule_repositories", "maybe")

kythe_rule_repositories()

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

RULES_JVM_EXTERNAL_TAG = "2.9"
RULES_JVM_EXTERNAL_SHA = "e5b97a31a3e8feed91636f42e19b11c49487b85e5de2f387c999ea14d77c7f45"

http_archive(
    name = "rules_jvm_external",
    strip_prefix = "rules_jvm_external-%s" % RULES_JVM_EXTERNAL_TAG,
    sha256 = RULES_JVM_EXTERNAL_SHA,
    url = "https://github.com/bazelbuild/rules_jvm_external/archive/%s.zip" % RULES_JVM_EXTERNAL_TAG,
)

load("@rules_jvm_external//:defs.bzl", "maven_install")

maven_install(
    name = "maven",
    artifacts = [
        "com.beust:jcommander:1.48",
        "com.google.auto.service:auto-service:1.0-rc4",
        "com.google.auto.value:auto-value:1.5.4",
        "com.google.auto:auto-common:0.10",
        "com.google.code.findbugs:jsr305:3.0.1",
        "com.google.code.gson:gson:2.8.5",
        "com.google.common.html.types:types:1.0.8",
        "com.google.errorprone:error_prone_annotations:2.3.1",
        "com.google.guava:guava:26.0-jre",
        "com.google.re2j:re2j:1.2",
        "com.google.truth:truth:1.0",
        "com.googlecode.java-diff-utils:diffutils:1.3.0",
        "javax.annotation:jsr250-api:1.0",
        "junit:junit:4.12",
        "org.checkerframework:checker-qual:2.9.0",
        "org.ow2.asm:asm:7.0",
    ],
    repositories = [
        "https://jcenter.bintray.com",
        "https://maven.google.com",
        "https://repo1.maven.org/maven2",
    ],
    fetch_sources = True,
    generate_compat_repositories = True, # Required by bazel-common's dependencies
    version_conflict_policy = "pinned",
)
load("@maven//:compat.bzl", "compat_repositories")
compat_repositories()
