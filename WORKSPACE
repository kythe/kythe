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
###

###
# BEGIN rules_go setup
# must happen before any go_repository calls, otherwise after rules_go 0.44.0
# this error will be thrown:
#
#     ERROR: Failed to load Starlark extension '@@io_bazel_rules_nogo//:scope.bzl'.
#     Cycle in the workspace file detected. This indicates that a repository is used prior to being defined.
#     The following chain of repository dependencies lead to the missing definition.
#      - @@io_bazel_rules_nogo
#     This could either mean you have to add the '@@io_bazel_rules_nogo' repository with a statement like
#     `http_archive` in your WORKSPACE file (note that transitive dependencies are not added automatically),
#     or move an existing definition earlier in your WORKSPACE file.
#     ERROR: Error computing the main repository mapping: cycles detected during computation of main repo mapping
#
# also note that go modules that rules_go requires are imported here first so
# that they take precedence over the version rules_go registers. this is for
# kythe-specific patches to work.
#
# this is fairly brittle, version bumps of rules_go need to be simultaneously
# performed with bumping versions of packages here.
# migrating kythe to bzlmod should make this much easier:
# https://github.com/bazel-contrib/rules_go/blob/42a97e95154170f6be04cbbdbe87808016e4470f/docs/go/core/bzlmod.md?plain=1#L312-L322
# however would be a separate undertaking.
load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

# gazelle:repository go_repository name=org_golang_x_sys importpath=golang.org/x/sys
http_archive(
    name = "org_golang_x_sys",
    urls = [
        "https://mirror.bazel.build/github.com/golang/sys/archive/refs/tags/v0.18.0.zip",
        "https://github.com/golang/sys/archive/refs/tags/v0.18.0.zip",
    ],
    sha256 = "d5ffb367cf0b672a6b97e2e0cf775dbae36276986485cf665cef5fd677563651",
    strip_prefix = "sys-0.18.0",
    patches = [
        "@io_kythe//third_party/go:add_export_license.patch",
        # releaser:patch-cmd gazelle -repo_root . -go_prefix golang.org/x/sys -go_naming_convention import_alias
        "@io_bazel_rules_go//third_party:org_golang_x_sys-gazelle.patch",
    ],
    patch_args = ["-p1"],
)

# gazelle:repository go_repository name=org_golang_x_tools importpath=golang.org/x/tools
http_archive(
    name = "org_golang_x_tools",
    # Must be kept in sync with rules_go or the patches may fail.
    # v0.15.0, latest as of 2024-02-27
    urls = [
        "https://mirror.bazel.build/github.com/golang/tools/archive/refs/tags/v0.15.0.zip",
        "https://github.com/golang/tools/archive/refs/tags/v0.15.0.zip",
    ],
    sha256 = "e76a03b11719138502c7fef44d5e1dc4469f8c2fcb2ee4a1d96fb09aaea13362",
    strip_prefix = "tools-0.15.0",
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
)

# gazelle:repository go_repository name=com_github_golang_protobuf importpath=github.com/golang/protobuf
http_archive(
    name = "com_github_golang_protobuf",
    # Must be kept in sync with rules_go or the patches may fail.
    # v1.5.3, latest as of 2023-12-15
    urls = [
        "https://mirror.bazel.build/github.com/golang/protobuf/archive/refs/tags/v1.5.3.zip",
        "https://github.com/golang/protobuf/archive/refs/tags/v1.5.3.zip",
    ],
    sha256 = "2dced4544ae5372281e20f1e48ca76368355a01b31353724718c4d6e3dcbb430",
    strip_prefix = "protobuf-1.5.3",
    patches = [
        "@io_kythe//third_party/go:new_export_license.patch",
        # releaser:patch-cmd gazelle -repo_root . -go_prefix github.com/golang/protobuf -go_naming_convention import_alias -proto disable_global
        "@io_bazel_rules_go//third_party:com_github_golang_protobuf-gazelle.patch",
    ],
    patch_args = ["-p1"],
)

go_rules_dependencies()
go_register_toolchains(version = "1.21.6")

# END rules_go setup
###

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
