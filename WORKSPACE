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
    ts_version_from = "//:package.json",
)

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

load("@aspect_rules_jasmine//jasmine:repositories.bzl", "jasmine_repositories")

jasmine_repositories(name = "jasmine")

load("@jasmine//:npm_repositories.bzl", jasmine_npm_repositories = "npm_repositories")

jasmine_npm_repositories()

# END rules_ts setup
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

load("@rules_rust//crate_universe:defs.bzl", "crate", "crates_repository", "render_config")

# Run `CARGO_BAZEL_REPIN=1 bazel sync --only=crate_index` after updating
crates_repository(
    name = "crate_index",
    cargo_lockfile = "//:Cargo.Bazel.lock",
    lockfile = "//:cargo-bazel-lock.json",
    packages = {
        "anyhow": crate.spec(
            version = "1.0.58",
        ),
        "base64": crate.spec(
            version = "0.13.0",
        ),
        "clap": crate.spec(
            features = ["derive"],
            version = "3.1.6",
        ),
        "colored": crate.spec(
            version = "2.0.0",
        ),
        "glob": crate.spec(
            version = "0.3.0",
        ),
        "hex": crate.spec(
            version = "0.4.3",
        ),
        "lazy_static": crate.spec(
            version = "1.4.0",
        ),
        "quick-error": crate.spec(
            version = "2.0.1",
        ),
        "path-clean": crate.spec(
            version = "0.1.0",
        ),
        "rayon": crate.spec(
            version = "1.5.3",
        ),
        "regex": crate.spec(
            version = "1.5.6",
        ),
        "rls-analysis": crate.spec(
            version = "0.18.3",
        ),
        "rls-data": crate.spec(
            version = "0.19.1",
        ),
        "serde": crate.spec(
            version = "1.0.137",
        ),
        "serde_json": crate.spec(
            version = "1.0.64",
        ),
        "sha2": crate.spec(
            version = "0.10.2",
        ),
        "tempfile": crate.spec(
            version = "3.3.0",
        ),
        "zip": crate.spec(
            version = "0.5.11",
        ),
        # Dev dependency for fuchsia extractor
        "serial_test": crate.spec(
            version = "0.6.0",
        ),
        # Dependencies for our Rust protobuf toolchain
        "protobuf": crate.spec(
            features = ["with-bytes"],
            version = "=2.27.1",
        ),
        "protobuf-codegen": crate.spec(
            version = "=2.27.1",
        ),
    },
    render_config = render_config(
        default_package_name = "",
    ),
)

load("@crate_index//:defs.bzl", "crate_repositories")

crate_repositories()

http_archive(
    name = "aspect_bazel_lib",
    sha256 = "e3151d87910f69cf1fc88755392d7c878034a69d6499b287bcfc00b1cf9bb415",
    strip_prefix = "bazel-lib-1.32.1",
    url = "https://github.com/aspect-build/bazel-lib/releases/download/v1.32.1/bazel-lib-v1.32.1.tar.gz",
)

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

aspect_bazel_lib_dependencies()
