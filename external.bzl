load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load(
    "@bazelruby_rules_ruby//ruby:deps.bzl",
    "rules_ruby_dependencies",
    "rules_ruby_select_sdk",
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")
load("@io_bazel_rules_go//go:deps.bzl", "go_download_sdk", "go_register_toolchains", "go_rules_dependencies")
load("@io_kythe//:setup.bzl", "github_archive")
load("@io_kythe//kythe/cxx/extractor:toolchain.bzl", cxx_extractor_register_toolchains = "register_toolchains")
load("@io_kythe//third_party/bazel:bazel_repository_files.bzl", "bazel_repository_files")
load("@io_kythe//tools:build_rules/shims.bzl", "go_repository")
load("@io_kythe//tools/build_rules/lexyacc:lexyacc.bzl", "lexyacc_configure")
load("@llvm-raw//utils/bazel:configure.bzl", "llvm_configure")
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
load(
    "@rules_java//java:repositories.bzl",
    "remote_jdk17_repos",
    "remote_jdk21_repos",
)
load("@rules_java//toolchains:remote_java_repository.bzl", "remote_java_repository")
load("@rules_jvm_external//:defs.bzl", "maven_install")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@rules_python//python:repositories.bzl", "py_repositories")

def _rule_dependencies():
    gazelle_dependencies()

    # Specifically define and register only Java toolchains we intend to support.
    remote_jdk17_repos()
    remote_jdk19_repos()
    remote_jdk21_repos()
    native.register_toolchains("@local_jdk//:runtime_toolchain_definition")
    for version in ("11", "17", "19", "21"):
        for platform in ("linux", "macos", "win"):
            native.register_toolchains("@remotejdk{version}_{platform}_toolchain_config_repo//:toolchain".format(
                version = version,
                platform = platform,
            ))
            if platform == "macos":
                native.register_toolchains("@remotejdk{version}_{platform}_aarch64_toolchain_config_repo//:toolchain".format(
                    version = version,
                    platform = platform,
                ))
    native.register_toolchains("//buildenv/java:all")

    protobuf_deps()
    rules_proto_dependencies()
    py_repositories()
    rules_ruby_dependencies()
    rules_ruby_select_sdk(version = "host")
    rules_foreign_cc_dependencies(register_built_tools = False)

def _gazelle_ignore(**kwargs):
    """Dummy macro which causes gazelle to see a repository as already defined."""

def _common_dependencies():
    # Rather than pull down the entire Bazel source repository for a single file,
    # just grab the files we need and use them locally.
    maybe(
        bazel_repository_files,
        name = "io_bazel_files",
        commit = "7fa5796de6094ff529ceb17ee73c7eac9e42eb15",
        files = [
            "src/java_tools/buildjar/java/com/google/devtools/build/buildjar/javac/JavacOptions.java",
            "src/java_tools/buildjar/java/com/google/devtools/build/buildjar/javac/WerrorCustomOption.java",
            "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.proto",
            "src/main/java/com/google/devtools/build/lib/packages/metrics/package_load_metrics.proto",
            "src/main/protobuf/command_line.proto",
            "src/main/protobuf/extra_actions_base.proto",
            "src/main/protobuf/failure_details.proto",
            "src/main/protobuf/invocation_policy.proto",
            "src/main/protobuf/option_filters.proto",
            "src/main/protobuf/test_status.proto",
        ],
        overlay = {
            "@io_kythe//third_party/bazel:root.BUILD": "BUILD",
            "@io_kythe//third_party/bazel:javac_options.BUILD": "src/java_tools/buildjar/java/com/google/devtools/build/buildjar/BUILD",
        },
    )

def _cc_dependencies():
    maybe(
        http_archive,
        name = "llvm_zstd",
        build_file = "@llvm-raw//utils/bazel/third_party_build:zstd.BUILD",
        sha256 = "7c42d56fac126929a6a85dbc73ff1db2411d04f104fae9bdea51305663a83fd0",
        strip_prefix = "zstd-1.5.2",
        urls = [
            "https://github.com/facebook/zstd/releases/download/v1.5.2/zstd-1.5.2.tar.gz",
        ],
    )

    maybe(
        llvm_configure,
        name = "llvm-project",
        repo_mapping = {
            "@llvm_zlib": "@net_zlib",
        },
    )

    maybe(
        http_archive,
        name = "org_sourceware_libffi",
        build_file = "@io_kythe//third_party:libffi.BUILD",
        sha256 = "653ffdfc67fbb865f39c7e5df2a071c0beb17206ebfb0a9ecb18a18f63f6b263",  # 2019-11-02
        strip_prefix = "libffi-3.3-rc2",
        urls = ["https://github.com/libffi/libffi/releases/download/v3.3-rc2/libffi-3.3-rc2.tar.gz"],
    )

    maybe(
        http_archive,
        name = "souffle",
        urls = ["https://github.com/souffle-lang/souffle/archive/cc8ea091721fcfb60bed45a0edf571ad9d0c58a5.zip"],
        build_file = "@io_kythe//third_party:souffle.BUILD",
        sha256 = "8e914fc7ccd7d846d9e3073ecfb185fcb4a85ada51cd8ab04283052e48ebfd62",
        strip_prefix = "souffle-cc8ea091721fcfb60bed45a0edf571ad9d0c58a5",
        patch_args = ["-p0"],
        patches = [
            "@io_kythe//third_party:souffle_remove_config.patch",
            "@io_kythe//third_party:souffle_filesystem.patch",
        ],
    )

    maybe(
        http_archive,
        name = "net_zlib",
        build_file = "@io_kythe//third_party:zlib.BUILD",
        sha256 = "d14c38e313afc35a9a8760dadf26042f51ea0f5d154b0630a31da0540107fb98",
        strip_prefix = "zlib-1.2.13",
        urls = [
            "https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.xz",
            "https://zlib.net/zlib-1.2.13.tar.xz",
        ],
    )

    maybe(
        http_archive,
        name = "org_libzip",
        build_file = "@io_kythe//third_party:libzip.BUILD",
        strip_prefix = "libzip-1.7.3",
        sha256 = "0e2276c550c5a310d4ebf3a2c3dfc43fb3b4602a072ff625842ad4f3238cb9cc",
        urls = [
            "https://mirror.bazel.build/github.com/nih-at/libzip/releases/download/v1.7.3/libzip-1.7.3.tar.gz",
            "https://github.com/nih-at/libzip/releases/download/v1.7.3/libzip-1.7.3.tar.gz",
        ],
    )

    maybe(
        git_repository,
        name = "boringssl",
        # Use the GitHub mirror because the official source at
        # https://boringssl.googlesource.com/boringssl does not allow
        # unauthenticated git clone and the archives suffer from
        # https://github.com/google/gitiles/issues/84 preventing the use of
        # sha256sum on archives.
        remote = "https://github.com/google/boringssl",
        # Commits must come from the master-with-bazel branch.
        # branch = "master-with-bazel",
        commit = "3ef9a6b03503ae25f9267473073fea9c39d9cdac",
        shallow_since = "1603819042 +0000",
    )

    maybe(
        http_archive,
        name = "com_github_tencent_rapidjson",
        build_file = "@io_kythe//third_party:rapidjson.BUILD",
        sha256 = "8e00c38829d6785a2dfb951bb87c6974fa07dfe488aa5b25deec4b8bc0f6a3ab",
        strip_prefix = "rapidjson-1.1.0",
        urls = [
            "https://mirror.bazel.build/github.com/Tencent/rapidjson/archive/v1.1.0.zip",
            "https://github.com/Tencent/rapidjson/archive/v1.1.0.zip",
        ],
    )

    # Make sure to update regularly in accordance with Abseil's principle of live at HEAD
    maybe(
        github_archive,
        name = "com_google_absl",
        repo_name = "abseil/abseil-cpp",
        commit = "71d553b12397ef81e9111b4fa21c68af3c0bf8b9",
    )

    maybe(
        http_archive,
        name = "com_google_googletest",
        sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
        strip_prefix = "googletest-1.14.0",
        urls = [
            "https://mirror.bazel.build/github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz",
            "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "org_brotli",
        sha256 = "f9e8d81d0405ba66d181529af42a3354f838c939095ff99930da6aa9cdf6fe46",
        strip_prefix = "brotli-1.0.9",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party:brotli/brotli-1.0.7-int-float-conversion.patch",
        ],
        urls = [
            "https://mirror.bazel.build/github.com/google/brotli/archive/v1.0.9.tar.gz",
            "https://github.com/google/brotli/archive/v1.0.9.tar.gz",
        ],
    )

    maybe(
        github_archive,
        name = "com_google_riegeli",
        repo_name = "google/riegeli",
        commit = "e68237a48ad60896e18d7899b01293751960c1d2",
    )

    maybe(
        http_archive,
        name = "org_libmemcached_libmemcached",
        build_file = "@io_kythe//third_party:libmemcached.BUILD",
        sha256 = "e22c0bb032fde08f53de9ffbc5a128233041d9f33b5de022c0978a2149885f82",
        strip_prefix = "libmemcached-1.0.18",
        urls = [
            "https://mirror.bazel.build/launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz",
            "https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "se_haxx_curl",
        build_file = "@io_kythe//third_party:curl.BUILD",
        sha256 = "ff3e80c1ca6a068428726cd7dd19037a47cc538ce58ef61c59587191039b2ca6",
        strip_prefix = "curl-7.49.1",
        urls = [
            "https://mirror.bazel.build/curl.haxx.se/download/curl-7.49.1.tar.gz",
            "https://curl.haxx.se/download/curl-7.49.1.tar.gz",
        ],
    )

    maybe(
        github_archive,
        name = "com_googlesource_code_re2",
        repo_name = "google/re2",
        commit = "1e44e72d31ddc66b783a545e9d9fcaa876a146b7",
        integrity = "sha256-ihSWrTM50oDsQKaXFBUoQjQMtQr62pdYQy+JOch+48E=",
    )

    maybe(
        http_archive,
        name = "com_github_stedolan_jq",
        build_file = "@io_kythe//third_party:jq.BUILD",
        sha256 = "998c41babeb57b4304e65b4eb73094279b3ab1e63801b6b4bddd487ce009b39d",
        strip_prefix = "jq-1.4",
        urls = [
            "https://mirror.bazel.build/github.com/stedolan/jq/releases/download/jq-1.4/jq-1.4.tar.gz",
            "https://github.com/stedolan/jq/releases/download/jq-1.4/jq-1.4.tar.gz",
        ],
    )

    maybe(
        github_archive,
        name = "com_github_google_snappy",
        repo_name = "google/snappy",
        build_file = "@io_kythe//third_party:snappy.BUILD",
        commit = "1.1.8",
        integrity = "sha256-OLSqv4jrSAEx7UW/uJwZyj4qYtrrCBvfABz7F+xM0wM=",
    )

    maybe(
        http_archive,
        name = "com_github_google_leveldb",
        build_file = "@io_kythe//third_party:leveldb.BUILD",
        sha256 = "5b2bd7a91489095ad54bb81ca6544561025b48ec6d19cc955325f96755d88414",
        strip_prefix = "leveldb-1.20",
        urls = [
            "https://mirror.bazel.build/github.com/google/leveldb/archive/v1.20.zip",
            "https://github.com/google/leveldb/archive/v1.20.zip",
        ],
    )

    maybe(
        github_archive,
        name = "com_github_inazarenko_protobuf_matchers",
        repo_name = "inazarenko/protobuf-matchers",
        commit = "2aeaf99c0f3e81e4fa493bc91f99616d1d7608b9",
        integrity = "sha256-OFRBZzcHjcAXXDv2uFk1zDUywqtlc+M1DimY33++HxM=",
    )

    lexyacc_configure()
    cxx_extractor_register_toolchains()

def _java_dependencies():
    maven_install(
        name = "maven",
        artifacts = [
            "com.google.flogger:flogger:0.7.3",
            "com.google.flogger:flogger-system-backend:0.7.3",
            "com.beust:jcommander:1.82",
            "com.google.auto.service:auto-service:1.0",
            "com.google.auto.service:auto-service-annotations:1.0",
            "com.google.auto.value:auto-value:1.8",
            "com.google.auto.value:auto-value-annotations:1.8",
            "com.google.auto:auto-common:1.0",
            "com.google.code.findbugs:jsr305:3.0.2",
            "com.google.code.gson:gson:2.8.9",
            "com.google.common.html.types:types:1.0.8",
            "com.google.errorprone:error_prone_annotations:2.22.0",
            "com.google.guava:guava:31.1-jre",
            "com.google.jimfs:jimfs:1.2",
            "com.google.re2j:re2j:1.6",
            "com.google.truth:truth:1.1.2",
            "com.googlecode.java-diff-utils:diffutils:1.3.0",
            "org.apache.tomcat:tomcat-annotations-api:9.0.45",
            "junit:junit:4.13.2",
            "org.checkerframework:checker-qual:3.12.0",
            "org.ow2.asm:asm:9.1",
        ],
        repositories = [
            "https://jcenter.bintray.com",
            "https://maven.google.com",
            "https://repo1.maven.org/maven2",
        ],
        fetch_sources = True,
        generate_compat_repositories = True,  # Required by bazel-common's dependencies
        version_conflict_policy = "pinned",
        maven_install_json = "//:maven_install.json",
    )

def _go_dependencies():
    _gazelle_ignore(
        name = "com_github_bazelbuild_rules_go",
        actual = "io_bazel_rules_go",
        importpath = "github.com/bazelbuild/rules_go",
    )

    _gazelle_ignore(name = "com_github_chzyer_logex")
    _gazelle_ignore(name = "com_github_chzyer_readline")
    _gazelle_ignore(name = "com_github_chzyer_test")

    _gazelle_ignore(name = "com_github_cncf_udpa_go")

    _gazelle_ignore(name = "com_github_ianlancetaylor_demangle")

    _gazelle_ignore(name = "com_github_yuin_goldmark")

    _gazelle_ignore(name = "io_rsc_quote_v3")
    _gazelle_ignore(name = "io_rsc_sampler")
    go_repository(
        name = "co_honnef_go_tools",
        importpath = "honnef.co/go/tools",
        sum = "h1:UoveltGrhghAA7ePc+e+QYDHXrBps2PqFZiHkGR/xK8=",
        version = "v0.0.1-2020.1.4",
    )
    go_repository(
        name = "com_github_aclements_go_moremath",
        importpath = "github.com/aclements/go-moremath",
        sum = "h1:xlwdaKcTNVW4PtpQb8aKA4Pjy0CdJHEqvFbAnvR5m2g=",
        version = "v0.0.0-20210112150236-f10218a38794",
    )
    go_repository(
        name = "com_github_alecthomas_template",
        importpath = "github.com/alecthomas/template",
        sum = "h1:JYp7IbQjafoB+tBA3gMyHYHrpOtNuDiK/uB5uXxq5wM=",
        version = "v0.0.0-20190718012654-fb15b899a751",
    )
    go_repository(
        name = "com_github_alecthomas_units",
        importpath = "github.com/alecthomas/units",
        sum = "h1:UQZhZ2O0vMHr2cI+DC1Mbh0TJxzA3RcLoMsFw+aXw7E=",
        version = "v0.0.0-20190924025748-f65c72e2690d",
    )
    go_repository(
        name = "com_github_andybalholm_brotli",
        importpath = "github.com/andybalholm/brotli",
        sum = "h1:V7DdXeJtZscaqfNuAdSRuRFzuiKlHSC/Zh3zl9qY3JY=",
        version = "v1.0.4",
    )
    go_repository(
        name = "com_github_andybalholm_cascadia",
        importpath = "github.com/andybalholm/cascadia",
        sum = "h1:nhxRkql1kdYCc8Snf7D5/D3spOX+dBgjA6u8x004T2c=",
        version = "v1.3.1",
    )
    go_repository(
        name = "com_github_apache_beam",
        build_extra_args = ["-known_import=beam.apache.org"],
        build_file_proto_mode = "disable",
        importpath = "github.com/apache/beam",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:7C2/JDa+fiRJs8kAcfCHxVTf0xxwKsCFQYDMoRdr/dk=",
        version = "v2.31.0+incompatible",
    )
    go_repository(
        name = "com_github_aymerick_douceur",
        importpath = "github.com/aymerick/douceur",
        sum = "h1:Mv+mAeH1Q+n9Fr+oyamOlAkUNPWPlA8PPGR0QAaYuPk=",
        version = "v0.2.0",
    )
    go_repository(
        name = "com_github_bazelbuild_bazel_gazelle",
        importpath = "github.com/bazelbuild/bazel-gazelle",
        sum = "h1:BpkUzE3H2l6buJYFTKgzVMecJimQgWwYud25qVIx0SQ=",
        version = "v0.42.0",
    )
    go_repository(
        name = "com_github_bazelbuild_buildtools",
        importpath = "github.com/bazelbuild/buildtools",
        sum = "h1:FGzENZi+SX9I7h9xvMtRA3rel8hCEfyzSixteBgn7MU=",
        version = "v0.0.0-20240918101019-be1c24cc9a44",
    )
    go_repository(
        name = "com_github_beevik_etree",
        importpath = "github.com/beevik/etree",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:l7WETslUG/T+xOPs47dtd6jov2Ii/8/OjCldk5fYfQw=",
        version = "v1.2.0",
    )
    go_repository(
        name = "com_github_beorn7_perks",
        importpath = "github.com/beorn7/perks",
        sum = "h1:VlbKKnNfV8bJzeqoa4cOKqO6bYr3WgKZxO8Z16+hsOM=",
        version = "v1.0.1",
    )
    go_repository(
        name = "com_github_bmatcuk_doublestar_v4",
        importpath = "github.com/bmatcuk/doublestar/v4",
        sum = "h1:54Bopc5c2cAvhLRAzqOGCYHYyhcDHsFF4wWIR5wKP38=",
        version = "v4.8.1",
    )
    go_repository(
        name = "com_github_burntsushi_toml",
        importpath = "github.com/BurntSushi/toml",
        sum = "h1:Rt8g24XnyGTyglgET/PRUNlrUeu9F5L+7FilkXfZgs0=",
        version = "v1.2.0",
    )
    go_repository(
        name = "com_github_burntsushi_xgb",
        importpath = "github.com/BurntSushi/xgb",
        sum = "h1:1BDTz0u9nC3//pOCMdNH+CiXJVYJh5UQNCOBG7jbELc=",
        version = "v0.0.0-20160522181843-27f122750802",
    )
    go_repository(
        name = "com_github_census_instrumentation_opencensus_proto",
        importpath = "github.com/census-instrumentation/opencensus-proto",
        sum = "h1:iKLQ0xPNFxR/2hzXZMrBo8f1j86j5WHzznCCQxV/b8g=",
        version = "v0.4.1",
    )
    go_repository(
        name = "com_github_cespare_xxhash_v2",
        importpath = "github.com/cespare/xxhash/v2",
        sum = "h1:UL815xU9SqsFlibzuggzjXhog7bL6oX9BbNZnL2UFvs=",
        version = "v2.3.0",
    )
    go_repository(
        name = "com_github_client9_misspell",
        importpath = "github.com/client9/misspell",
        sum = "h1:ta993UF76GwbvJcIo3Y68y/M3WxlpEHPWIGDkJYwzJI=",
        version = "v0.3.4",
    )
    go_repository(
        name = "com_github_cloudykit_fastprinter",
        importpath = "github.com/CloudyKit/fastprinter",
        sum = "h1:sR+/8Yb4slttB4vD+b9btVEnWgL3Q00OBTzVT8B9C0c=",
        version = "v0.0.0-20200109182630-33d98a066a53",
    )
    go_repository(
        name = "com_github_cloudykit_jet_v6",
        importpath = "github.com/CloudyKit/jet/v6",
        sum = "h1:hvO96X345XagdH1fAoBjpBYG4a1ghhL/QzalkduPuXk=",
        version = "v6.1.0",
    )
    go_repository(
        name = "com_github_cncf_xds_go",
        importpath = "github.com/cncf/xds/go",
        sum = "h1:N+3sFI5GUjRKBi+i0TxYVST9h4Ie192jJWpHvthBBgg=",
        version = "v0.0.0-20240723142845-024c85f92f20",
    )
    go_repository(
        name = "com_github_cockroachdb_datadriven",
        importpath = "github.com/cockroachdb/datadriven",
        sum = "h1:otljaYPt5hWxV3MUfO5dFPFiOXg9CyG5/kCfayTqsJ4=",
        version = "v1.0.3-0.20230413201302-be42291fc80f",
    )
    go_repository(
        name = "com_github_cockroachdb_errors",
        build_file_proto_mode = "disable_global",
        importpath = "github.com/cockroachdb/errors",
        sum = "h1:xSEW75zKaKCWzR3OfxXUxgrk/NtT4G1MiOv5lWZazG8=",
        version = "v1.11.1",
    )
    go_repository(
        name = "com_github_cockroachdb_logtags",
        importpath = "github.com/cockroachdb/logtags",
        sum = "h1:r6VH0faHjZeQy818SGhaone5OnYfxFR/+AzdY3sf5aE=",
        version = "v0.0.0-20230118201751-21c54148d20b",
    )
    go_repository(
        name = "com_github_cockroachdb_pebble",
        importpath = "github.com/cockroachdb/pebble",
        sum = "h1:pcFh8CdCIt2kmEpK0OIatq67Ln9uGDYY3d5XnE0LJG4=",
        version = "v1.1.0",
    )
    go_repository(
        name = "com_github_cockroachdb_redact",
        importpath = "github.com/cockroachdb/redact",
        sum = "h1:u1PMllDkdFfPWaNGMyLD1+so+aq3uUItthCFqzwPJ30=",
        version = "v1.1.5",
    )
    go_repository(
        name = "com_github_cockroachdb_tokenbucket",
        importpath = "github.com/cockroachdb/tokenbucket",
        sum = "h1:zuQyyAKVxetITBuuhv3BI9cMrmStnpT18zmgmTxunpo=",
        version = "v0.0.0-20230807174530-cc333fc44b06",
    )
    go_repository(
        name = "com_github_codegangsta_inject",
        importpath = "github.com/codegangsta/inject",
        sum = "h1:sDMmm+q/3+BukdIpxwO365v/Rbspp2Nt5XntgQRXq8Q=",
        version = "v0.0.0-20150114235600-33e0aa1cb7c0",
    )
    go_repository(
        name = "com_github_creack_pty",
        importpath = "github.com/creack/pty",
        sum = "h1:uDmaGzcdjhF4i/plgjmEsriH11Y0o7RKapEf/LDaM3w=",
        version = "v1.1.9",
    )
    go_repository(
        name = "com_github_datadog_zstd",
        importpath = "github.com/DataDog/zstd",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
            "@io_kythe//third_party/go:datadog-zstd.patch",
        ],
        sum = "h1:oWf5W7GtOLgp6bciQYDmhHHjdhYkALu6S/5Ni9ZgSvQ=",
        version = "v1.5.5",
    )
    go_repository(
        name = "com_github_davecgh_go_spew",
        importpath = "github.com/davecgh/go-spew",
        sum = "h1:vj9j/u1bqnvCEfJOwUhtlOARqs3+rkHYY13jYWTU97c=",
        version = "v1.1.1",
    )
    go_repository(
        name = "com_github_eknkc_amber",
        importpath = "github.com/eknkc/amber",
        sum = "h1:clC1lXBpe2kTj2VHdaIu9ajZQe4kcEY9j0NsnDDBZ3o=",
        version = "v0.0.0-20171010120322-cdade1c07385",
    )
    go_repository(
        name = "com_github_envoyproxy_go_control_plane",
        importpath = "github.com/envoyproxy/go-control-plane",
        sum = "h1:HzkeUz1Knt+3bK+8LG1bxOO/jzWZmdxpwC51i202les=",
        version = "v0.13.0",
    )
    go_repository(
        name = "com_github_envoyproxy_protoc_gen_validate",
        importpath = "github.com/envoyproxy/protoc-gen-validate",
        sum = "h1:tntQDh69XqOCOZsDz0lVJQez/2L6Uu2PdjCQwWCJ3bM=",
        version = "v1.1.0",
    )
    go_repository(
        name = "com_github_fatih_structs",
        importpath = "github.com/fatih/structs",
        sum = "h1:Q7juDM0QtcnhCpeyLGQKyg4TOIghuNXrkL32pHAUMxo=",
        version = "v1.1.0",
    )
    go_repository(
        name = "com_github_felixge_httpsnoop",
        importpath = "github.com/felixge/httpsnoop",
        sum = "h1:NFTV2Zj1bL4mc9sqWACXbQFVBBg2W3GPvqp8/ESS2Wg=",
        version = "v1.0.4",
    )
    go_repository(
        name = "com_github_flosch_pongo2_v4",
        importpath = "github.com/flosch/pongo2/v4",
        sum = "h1:gv+5Pe3vaSVmiJvh/BZa82b7/00YUGm0PIyVVLop0Hw=",
        version = "v4.0.2",
    )
    go_repository(
        name = "com_github_fsnotify_fsnotify",
        importpath = "github.com/fsnotify/fsnotify",
        sum = "h1:8JEhPFa5W2WU7YfeZzPNqzMP6Lwt7L2715Ggo0nosvA=",
        version = "v1.7.0",
    )
    go_repository(
        name = "com_github_getsentry_sentry_go",
        importpath = "github.com/getsentry/sentry-go",
        sum = "h1:MtBW5H9QgdcJabtZcuJG80BMOwaBpkRDZkxRkNC1sN0=",
        version = "v0.18.0",
    )
    go_repository(
        name = "com_github_ghemawat_stream",
        importpath = "github.com/ghemawat/stream",
        sum = "h1:r5GgOLGbza2wVHRzK7aAj6lWZjfbAwiu/RDCVOKjRyM=",
        version = "v0.0.0-20171120220530-696b145b53b9",
    )
    go_repository(
        name = "com_github_gin_contrib_sse",
        importpath = "github.com/gin-contrib/sse",
        sum = "h1:Y/yl/+YNO8GZSjAhjMsSuLt29uWRFHdHYUb5lYOV9qE=",
        version = "v0.1.0",
    )
    go_repository(
        name = "com_github_gin_gonic_gin",
        importpath = "github.com/gin-gonic/gin",
        sum = "h1:4+fr/el88TOO3ewCmQr8cx/CtZ/umlIRIs5M4NTNjf8=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_github_go_errors_errors",
        importpath = "github.com/go-errors/errors",
        sum = "h1:J6MZopCL4uSllY1OfXM374weqZFFItUbrImctkmUxIA=",
        version = "v1.4.2",
    )
    go_repository(
        name = "com_github_go_gl_glfw",
        importpath = "github.com/go-gl/glfw",
        sum = "h1:QbL/5oDUmRBzO9/Z7Seo6zf912W/a6Sr4Eu0G/3Jho0=",
        version = "v0.0.0-20190409004039-e6da0acd62b1",
    )
    go_repository(
        name = "com_github_go_gl_glfw_v3_3_glfw",
        importpath = "github.com/go-gl/glfw/v3.3/glfw",
        sum = "h1:WtGNWLvXpe6ZudgnXrq0barxBImvnnJoMEhXAzcbM0I=",
        version = "v0.0.0-20200222043503-6f7a984d4dc4",
    )
    go_repository(
        name = "com_github_go_kit_kit",
        importpath = "github.com/go-kit/kit",
        sum = "h1:wDJmvq38kDhkVxi50ni9ykkdUr1PKgqKOoi01fa0Mdk=",
        version = "v0.9.0",
    )
    go_repository(
        name = "com_github_go_kit_log",
        importpath = "github.com/go-kit/log",
        sum = "h1:DGJh0Sm43HbOeYDNnVZFl8BvcYVvjD5bqYJvp0REbwQ=",
        version = "v0.1.0",
    )
    go_repository(
        name = "com_github_go_logfmt_logfmt",
        importpath = "github.com/go-logfmt/logfmt",
        sum = "h1:TrB8swr/68K7m9CcGut2g3UOihhbcbiMAYiuTXdEih4=",
        version = "v0.5.0",
    )
    go_repository(
        name = "com_github_go_logr_logr",
        importpath = "github.com/go-logr/logr",
        sum = "h1:6pFjapn8bFcIbiKo3XT4j/BhANplGihG6tvd+8rYgrY=",
        version = "v1.4.2",
    )
    go_repository(
        name = "com_github_go_logr_stdr",
        importpath = "github.com/go-logr/stdr",
        sum = "h1:hSWxHoqTgW2S2qGc0LTAI563KZ5YKYRhT3MFKZMbjag=",
        version = "v1.2.2",
    )
    go_repository(
        name = "com_github_go_martini_martini",
        importpath = "github.com/go-martini/martini",
        sum = "h1:xveKWz2iaueeTaUgdetzel+U7exyigDYBryyVfV/rZk=",
        version = "v0.0.0-20170121215854-22fa46961aab",
    )
    go_repository(
        name = "com_github_go_playground_locales",
        importpath = "github.com/go-playground/locales",
        sum = "h1:u50s323jtVGugKlcYeyzC0etD1HifMjqmJqb8WugfUU=",
        version = "v0.14.0",
    )
    go_repository(
        name = "com_github_go_playground_universal_translator",
        importpath = "github.com/go-playground/universal-translator",
        sum = "h1:82dyy6p4OuJq4/CByFNOn/jYrnRPArHwAcmLoJZxyho=",
        version = "v0.18.0",
    )
    go_repository(
        name = "com_github_go_playground_validator_v10",
        importpath = "github.com/go-playground/validator/v10",
        sum = "h1:prmOlTVv+YjZjmRmNSF3VmspqJIxJWXmqUsHwfTRRkQ=",
        version = "v10.11.1",
    )
    go_repository(
        name = "com_github_go_stack_stack",
        importpath = "github.com/go-stack/stack",
        sum = "h1:5SgMzNM5HxrEjV0ww2lTmX6E2Izsfxas4+YHWRs3Lsk=",
        version = "v1.8.0",
    )
    go_repository(
        name = "com_github_goccy_go_json",
        importpath = "github.com/goccy/go-json",
        sum = "h1:/pAaQDLHEoCq/5FFmSKBswWmK6H0e8g4159Kc/X/nqk=",
        version = "v0.9.11",
    )
    go_repository(
        name = "com_github_gogo_googleapis",
        build_file_proto_mode = "disable",
        importpath = "github.com/gogo/googleapis",
        sum = "h1:1Yx4Myt7BxzvUr5ldGSbwYiZG6t9wGBZ+8/fX3Wvtq0=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_github_gogo_protobuf",
        build_file_generation = "off",
        importpath = "github.com/gogo/protobuf",
        patch_args = ["-p1"],
        patches = ["@io_bazel_rules_go//third_party:com_github_gogo_protobuf-gazelle.patch"],
        sum = "h1:Ov1cvc58UF3b5XjBnZv7+opcTcQFZebYjWzi34vdm4Q=",
        version = "v1.3.2",
    )
    go_repository(
        name = "com_github_gogo_status",
        importpath = "github.com/gogo/status",
        sum = "h1:+eIkrewn5q6b30y+g/BJINVVdi2xH7je5MPJ3ZPK3JA=",
        version = "v1.1.0",
    )
    go_repository(
        name = "com_github_golang_glog",
        importpath = "github.com/golang/glog",
        sum = "h1:1+mZ9upx1Dh6FmUTFR1naJ77miKiXgALjWOZ3NVFPmY=",
        version = "v1.2.2",
    )
    go_repository(
        name = "com_github_golang_groupcache",
        importpath = "github.com/golang/groupcache",
        sum = "h1:oI5xCqsCo564l8iNU+DwB5epxmsaqB+rhGL0m5jtYqE=",
        version = "v0.0.0-20210331224755-41bb18bfe9da",
    )
    go_repository(
        name = "com_github_golang_mock",
        importpath = "github.com/golang/mock",
        sum = "h1:YojYx61/OLFsiv6Rw1Z96LpldJIy31o+UHmwAUMJ6/U=",
        version = "v1.7.0-rc.1",
    )
    go_repository(
        name = "com_github_golang_protobuf",
        build_file_proto_mode = "disable_global",
        importpath = "github.com/golang/protobuf",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:new_export_license.patch",
        ],
        sum = "h1:i7eJL8qZTpSEXOPTxNKhASYpMn+8e5Q6AdndVa1dWek=",
        version = "v1.5.4",
    )
    go_repository(
        name = "com_github_golang_snappy",
        importpath = "github.com/golang/snappy",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:yAGX7huGHXlcLOEtBnF4w7FQwA26wojNCwOYAEhLjQM=",
        version = "v0.0.4",
    )
    go_repository(
        name = "com_github_google_brotli_go_cbrotli",
        importpath = "github.com/google/brotli/go/cbrotli",
        sum = "h1:Cl8UPT6eo1fkL+TFS6LSW3J6gUGgD0rFKZVAgw/0iwY=",
        version = "v0.0.0-20230919092154-53947c15f577",
    )
    go_repository(
        name = "com_github_google_btree",
        importpath = "github.com/google/btree",
        sum = "h1:0udJVsspx3VBr5FwtLhQQtuAsVc79tTq0ocGIPAU6qo=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_google_codesearch",
        importpath = "github.com/google/codesearch",
        sum = "h1:VlyAH+AntnIbGGArOUs6sEBdPVwYvf1e8Uw3/TC77cA=",
        version = "v1.2.0",
    )
    go_repository(
        name = "com_github_google_go_cmp",
        importpath = "github.com/google/go-cmp",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:ofyhxvXcZhMsU5ulbFiLKl/XBFqE1GSq7atu8tAmTRI=",
        version = "v0.6.0",
    )
    go_repository(
        name = "com_github_google_go_pkcs11",
        importpath = "github.com/google/go-pkcs11",
        sum = "h1:PVRnTgtArZ3QQqTGtbtjtnIkzl2iY2kt24yqbrf7td8=",
        version = "v0.3.0",
    )
    go_repository(
        name = "com_github_google_gofuzz",
        importpath = "github.com/google/gofuzz",
        sum = "h1:A8PeW59pxE9IoFRqBp37U+mSNaQoZ46F1f0f863XSXw=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_google_martian",
        importpath = "github.com/google/martian",
        sum = "h1:/CP5g8u/VJHijgedC/Legn3BAbAaWPgecwXBIDzw5no=",
        version = "v2.1.0+incompatible",
    )
    go_repository(
        name = "com_github_google_martian_v3",
        importpath = "github.com/google/martian/v3",
        sum = "h1:DIhPTQrbPkgs2yJYdXU/eNACCG5DVQjySNRNlflZ9Fc=",
        version = "v3.3.3",
    )
    go_repository(
        name = "com_github_google_orderedcode",
        importpath = "github.com/google/orderedcode",
        sum = "h1:UzfcAexk9Vhv8+9pNOgRu41f16lHq725vPwnSeiG/Us=",
        version = "v0.0.1",
    )
    go_repository(
        name = "com_github_google_pprof",
        importpath = "github.com/google/pprof",
        sum = "h1:Ak8CrdlwwXwAZxzS66vgPt4U8yUZX7JwLvVR58FN5jM=",
        version = "v0.0.0-20200708004538-1a94d8640e99",
    )
    go_repository(
        name = "com_github_google_renameio",
        importpath = "github.com/google/renameio",
        sum = "h1:GOZbcHa3HfsPKPlmyPyN2KEohoMXOhdMbHrvbpl2QaA=",
        version = "v0.1.0",
    )
    go_repository(
        name = "com_github_google_s2a_go",
        importpath = "github.com/google/s2a-go",
        sum = "h1:zZDs9gcbt9ZPLV0ndSyQk6Kacx2g/X+SKYovpnz3SMM=",
        version = "v0.1.8",
    )
    go_repository(
        name = "com_github_google_subcommands",
        importpath = "github.com/google/subcommands",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:vWQspBTo2nEqTUFita5/KeEWlUL8kQObDFbub/EN9oE=",
        version = "v1.2.0",
    )
    go_repository(
        name = "com_github_google_uuid",
        importpath = "github.com/google/uuid",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:NIvaJDMOsjHA8n1jAhLSgzrAzy1Hgr+hNrb57e+94F0=",
        version = "v1.6.0",
    )
    go_repository(
        name = "com_github_googleapis_enterprise_certificate_proxy",
        importpath = "github.com/googleapis/enterprise-certificate-proxy",
        sum = "h1:XYIDZApgAnrN1c855gTgghdIA6Stxb52D5RnLI1SLyw=",
        version = "v0.3.4",
    )
    go_repository(
        name = "com_github_googleapis_gax_go_v2",
        build_file_proto_mode = "disable",
        importpath = "github.com/googleapis/gax-go/v2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:f+jMrjBPl+DL9nI4IQzLUxMq7XrAqFYB7hBPqMNIe8o=",
        version = "v2.14.0",
    )
    go_repository(
        name = "com_github_googlecloudplatform_opentelemetry_operations_go_detectors_gcp",
        importpath = "github.com/GoogleCloudPlatform/opentelemetry-operations-go/detectors/gcp",
        sum = "h1:3c8yed4lgqTt+oTQ+JNMDo+F4xprBf+O/il4ZC0nRLw=",
        version = "v1.25.0",
    )
    go_repository(
        name = "com_github_gorilla_css",
        importpath = "github.com/gorilla/css",
        sum = "h1:BQqNyPTi50JCFMTw/b67hByjMVXZRwGha6wxVGkeihY=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_gorilla_websocket",
        importpath = "github.com/gorilla/websocket",
        sum = "h1:q7AeDBpnBk8AogcD4DSag/Ukw/KV+YhzLj2bP5HvKCM=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_github_guptarohit_asciigraph",
        importpath = "github.com/guptarohit/asciigraph",
        sum = "h1:ccFnUF8xYIOUPPY3tmdvRyHqmn1MYI9iv1pLKX+/ZkQ=",
        version = "v0.5.5",
    )
    go_repository(
        name = "com_github_hanwen_go_fuse",
        importpath = "github.com/hanwen/go-fuse",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:GxS9Zrn6c35/BnfiVsZVWmsG803xwE7eVRDvcf/BEVc=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_hashicorp_golang_lru",
        importpath = "github.com/hashicorp/golang-lru",
        sum = "h1:0hERBMJE1eitiLkihrMvRVBYAkpHzc/J3QdDN+dAcgU=",
        version = "v0.5.1",
    )
    go_repository(
        name = "com_github_hdrhistogram_hdrhistogram_go",
        importpath = "github.com/HdrHistogram/hdrhistogram-go",
        sum = "h1:5IcZpTvzydCQeHzK4Ef/D5rrSqwxob0t8PQPMybUNFM=",
        version = "v1.1.2",
    )
    go_repository(
        name = "com_github_hpcloud_tail",
        importpath = "github.com/hpcloud/tail",
        sum = "h1:nfCOvKYfkgYP8hkirhJocXT2+zOD8yUNjXaWfTlyFKI=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_hydrogen18_memlistener",
        importpath = "github.com/hydrogen18/memlistener",
        sum = "h1:JR7eDj8HD6eXrc5fWLbSUnfcQFL06PYvCc0DKQnWfaU=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_inconshreveable_mousetrap",
        importpath = "github.com/inconshreveable/mousetrap",
        sum = "h1:Z8tu5sraLXCXIcARxBp/8cbvlwVa7Z1NHg9XEKhtSvM=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_iris_contrib_jade",
        importpath = "github.com/iris-contrib/jade",
        sum = "h1:WoYdfyJFfZIUgqNAeOyRfTNQZOksSlZ6+FnXR3AEpX0=",
        version = "v1.1.4",
    )
    go_repository(
        name = "com_github_iris_contrib_schema",
        importpath = "github.com/iris-contrib/schema",
        sum = "h1:CPSBLyx2e91H2yJzPuhGuifVRnZBBJ3pCOMbOvPZaTw=",
        version = "v0.0.6",
    )
    go_repository(
        name = "com_github_jmhodges_levigo",
        importpath = "github.com/jmhodges/levigo",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:levigo.patch",
        ],
        sum = "h1:q5EC36kV79HWeTBWsod3mG11EgStG3qArTKcvlksN1U=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_johanneskaufmann_html_to_markdown",
        importpath = "github.com/JohannesKaufmann/html-to-markdown",
        sum = "h1:CMAl6hz2MRfs03ZGAwYqQTC43Egi3vbc9SVo6nEKUE0=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_github_josharian_intern",
        importpath = "github.com/josharian/intern",
        sum = "h1:vlS4z54oSdjm0bgjRigI+G1HpF+tI+9rE5LLzOg8HmY=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_jpillora_backoff",
        importpath = "github.com/jpillora/backoff",
        sum = "h1:uvFg412JmmHBHw7iwprIxkPMI+sGQ4kzOWsMeHnm2EA=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_json_iterator_go",
        importpath = "github.com/json-iterator/go",
        sum = "h1:PV8peI4a0ysnczrg+LtxykD8LfKY9ML6u2jnxaEnrnM=",
        version = "v1.1.12",
    )
    go_repository(
        name = "com_github_jstemmer_go_junit_report",
        importpath = "github.com/jstemmer/go-junit-report",
        sum = "h1:6QPYqodiu3GuPL+7mfx+NwDdp2eTkp9IfEUpgAwUN0o=",
        version = "v0.9.1",
    )
    go_repository(
        name = "com_github_julienschmidt_httprouter",
        importpath = "github.com/julienschmidt/httprouter",
        sum = "h1:U0609e9tgbseu3rBINet9P48AI/D3oJs4dN7jwJOQ1U=",
        version = "v1.3.0",
    )
    go_repository(
        name = "com_github_kataras_blocks",
        importpath = "github.com/kataras/blocks",
        sum = "h1:cF3RDY/vxnSRezc7vLFlQFTYXG/yAr1o7WImJuZbzC4=",
        version = "v0.0.7",
    )
    go_repository(
        name = "com_github_kataras_golog",
        importpath = "github.com/kataras/golog",
        sum = "h1:0TY5tHn5L5DlRIikepcaRR/6oInIr9AiWsxzt0vvlBE=",
        version = "v0.1.7",
    )
    go_repository(
        name = "com_github_kataras_iris_v12",
        importpath = "github.com/kataras/iris/v12",
        sum = "h1:grB/oCf5baZhmYIeDMfgN3LYrtEcmK8pbxlRvEZ2pgw=",
        version = "v12.2.0-beta5",
    )
    go_repository(
        name = "com_github_kataras_pio",
        importpath = "github.com/kataras/pio",
        sum = "h1:kqreJ5KOEXGMwHAWHDwIl+mjfNCPhAwZPa8gK7MKlyw=",
        version = "v0.0.11",
    )
    go_repository(
        name = "com_github_kataras_sitemap",
        importpath = "github.com/kataras/sitemap",
        sum = "h1:w71CRMMKYMJh6LR2wTgnk5hSgjVNB9KL60n5e2KHvLY=",
        version = "v0.0.6",
    )
    go_repository(
        name = "com_github_kataras_tunnel",
        importpath = "github.com/kataras/tunnel",
        sum = "h1:sCAqWuJV7nPzGrlb0os3j49lk2JhILT0rID38NHNLpA=",
        version = "v0.0.4",
    )
    go_repository(
        name = "com_github_kisielk_errcheck",
        importpath = "github.com/kisielk/errcheck",
        sum = "h1:e8esj/e4R+SAOwFwN+n3zr0nYeCyeweozKfO23MvHzY=",
        version = "v1.5.0",
    )
    go_repository(
        name = "com_github_kisielk_gotool",
        importpath = "github.com/kisielk/gotool",
        sum = "h1:AV2c/EiW3KqPNT9ZKl07ehoAGi4C5/01Cfbblndcapg=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_klauspost_compress",
        importpath = "github.com/klauspost/compress",
        sum = "h1:EF27CXIuDsYJ6mmvtBRlEuB2UVOqHG1tAXgZ7yIO+lw=",
        version = "v1.15.15",
    )
    go_repository(
        name = "com_github_konsorten_go_windows_terminal_sequences",
        importpath = "github.com/konsorten/go-windows-terminal-sequences",
        sum = "h1:CE8S1cTafDpPvMhIxNJKvHsGVBgn1xWYf1NbHQhywc8=",
        version = "v1.0.3",
    )
    go_repository(
        name = "com_github_kr_logfmt",
        importpath = "github.com/kr/logfmt",
        sum = "h1:T+h1c/A9Gawja4Y9mFVWj2vyii2bbUNDw3kt9VxK2EY=",
        version = "v0.0.0-20140226030751-b84e30acd515",
    )
    go_repository(
        name = "com_github_kr_pretty",
        importpath = "github.com/kr/pretty",
        sum = "h1:flRD4NNwYAUpkphVc1HcthR4KEIFJ65n8Mw5qdRn3LE=",
        version = "v0.3.1",
    )
    go_repository(
        name = "com_github_kr_pty",
        importpath = "github.com/kr/pty",
        sum = "h1:VkoXIwSboBpnk99O/KFauAEILuNHv5DVFKZMBN/gUgw=",
        version = "v1.1.1",
    )
    go_repository(
        name = "com_github_kr_text",
        importpath = "github.com/kr/text",
        sum = "h1:5Nx0Ya0ZqY2ygV366QzturHI13Jq95ApcVaJBhpS+AY=",
        version = "v0.2.0",
    )
    go_repository(
        name = "com_github_labstack_echo_v4",
        importpath = "github.com/labstack/echo/v4",
        sum = "h1:wPOF1CE6gvt/kmbMR4dGzWvHMPT+sAEUJOwOTtvITVY=",
        version = "v4.9.0",
    )
    go_repository(
        name = "com_github_labstack_gommon",
        importpath = "github.com/labstack/gommon",
        sum = "h1:OomWaJXm7xR6L1HmEtGyQf26TEn7V6X88mktX9kee9o=",
        version = "v0.3.1",
    )
    go_repository(
        name = "com_github_leodido_go_urn",
        importpath = "github.com/leodido/go-urn",
        sum = "h1:BqpAaACuzVSgi/VLzGZIobT2z4v53pjosyNd9Yv6n/w=",
        version = "v1.2.1",
    )
    go_repository(
        name = "com_github_mailgun_raymond_v2",
        importpath = "github.com/mailgun/raymond/v2",
        sum = "h1:aOYHhvTpF5USySJ0o7cpPno/Uh2I5qg2115K25A+Ft4=",
        version = "v2.0.46",
    )
    go_repository(
        name = "com_github_mailru_easyjson",
        importpath = "github.com/mailru/easyjson",
        sum = "h1:UGYAvKxe3sBsEDzO8ZeWOSlIQfWFlxbzLZe7hwFURr0=",
        version = "v0.7.7",
    )
    go_repository(
        name = "com_github_mattn_go_colorable",
        importpath = "github.com/mattn/go-colorable",
        sum = "h1:fFA4WZxdEF4tXPZVKMLwD8oUnCTTo08duU7wxecdEvA=",
        version = "v0.1.13",
    )
    go_repository(
        name = "com_github_mattn_go_isatty",
        importpath = "github.com/mattn/go-isatty",
        sum = "h1:bq3VjFmv/sOjHtdEhmkEV4x1AJtvUvOJ2PFAZ5+peKQ=",
        version = "v0.0.16",
    )
    go_repository(
        name = "com_github_matttproud_golang_protobuf_extensions",
        importpath = "github.com/matttproud/golang_protobuf_extensions",
        sum = "h1:I0XW9+e1XWDxdcEniV4rQAIOPUGDq67JSCiRCgGCZLI=",
        version = "v1.0.2-0.20181231171920-c182affec369",
    )
    go_repository(
        name = "com_github_microcosm_cc_bluemonday",
        importpath = "github.com/microcosm-cc/bluemonday",
        sum = "h1:dNH3e4PSyE4vNX+KlRGHT5KrSvjeUkoNPwEORjffHJg=",
        version = "v1.0.21",
    )
    go_repository(
        name = "com_github_minio_highwayhash",
        importpath = "github.com/minio/highwayhash",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Aak5U0nElisjDCfPSG79Tgzkn2gl66NxOMspRrKnA/g=",
        version = "v1.0.2",
    )
    go_repository(
        name = "com_github_modern_go_concurrent",
        importpath = "github.com/modern-go/concurrent",
        sum = "h1:TRLaZ9cD/w8PVh93nsPXa1VrQ6jlwL5oN8l14QlcNfg=",
        version = "v0.0.0-20180306012644-bacd9c7ef1dd",
    )
    go_repository(
        name = "com_github_modern_go_reflect2",
        importpath = "github.com/modern-go/reflect2",
        sum = "h1:xBagoLtFs94CBntxluKeaWgTMpvLxC4ur3nMaC9Gz0M=",
        version = "v1.0.2",
    )
    go_repository(
        name = "com_github_mwitkow_go_conntrack",
        importpath = "github.com/mwitkow/go-conntrack",
        sum = "h1:KUppIJq7/+SVif2QVs3tOP0zanoHgBEVAwHxUSIzRqU=",
        version = "v0.0.0-20190716064945-2f068394615f",
    )
    go_repository(
        name = "com_github_onsi_ginkgo",
        importpath = "github.com/onsi/ginkgo",
        sum = "h1:VkHVNpR4iVnU8XQR6DBm8BqYjN7CRzw+xKUbVVbbW9w=",
        version = "v1.8.0",
    )
    go_repository(
        name = "com_github_onsi_gomega",
        importpath = "github.com/onsi/gomega",
        sum = "h1:izbySO9zDPmjJ8rDjLvkA2zJHIo+HkYXHnf7eN7SSyo=",
        version = "v1.5.0",
    )
    go_repository(
        name = "com_github_pelletier_go_toml_v2",
        importpath = "github.com/pelletier/go-toml/v2",
        sum = "h1:ipoSadvV8oGUjnUbMub59IDPPwfxF694nG/jwbMiyQg=",
        version = "v2.0.5",
    )
    go_repository(
        name = "com_github_pingcap_errors",
        importpath = "github.com/pingcap/errors",
        sum = "h1:lFuQV/oaUMGcD2tqt+01ROSmJs75VG1ToEOkZIZ4nE4=",
        version = "v0.11.4",
    )
    go_repository(
        name = "com_github_pkg_diff",
        importpath = "github.com/pkg/diff",
        sum = "h1:aoZm08cpOy4WuID//EZDgcC4zIxODThtZNPirFr42+A=",
        version = "v0.0.0-20210226163009-20ebb0f2a09e",
    )
    go_repository(
        name = "com_github_pkg_errors",
        importpath = "github.com/pkg/errors",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:FEBLx1zS214owpjy7qsBeixbURkuhQAwrK5UwLGTwt4=",
        version = "v0.9.1",
    )
    go_repository(
        name = "com_github_planetscale_vtprotobuf",
        importpath = "github.com/planetscale/vtprotobuf",
        sum = "h1:GFCKgmp0tecUJ0sJuv4pzYCqS9+RGSn52M3FUwPs+uo=",
        version = "v0.6.1-0.20240319094008-0393e58bdf10",
    )
    go_repository(
        name = "com_github_pmezard_go_difflib",
        importpath = "github.com/pmezard/go-difflib",
        sum = "h1:4DBwDE0NGyQoBHbLQYPwSUPoCMWR5BEzIk/f1lZbAQM=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_prometheus_client_golang",
        importpath = "github.com/prometheus/client_golang",
        sum = "h1:C+UIj/QWtmqY13Arb8kwMt5j34/0Z2iKamrJ+ryC0Gg=",
        version = "v1.12.0",
    )
    go_repository(
        name = "com_github_prometheus_client_model",
        importpath = "github.com/prometheus/client_model",
        sum = "h1:CmF68hwI0XsOQ5UwlBopMi2Ow4Pbg32akc4KIVCOm+Y=",
        version = "v0.2.1-0.20210607210712-147c58e9608a",
    )
    go_repository(
        name = "com_github_prometheus_common",
        importpath = "github.com/prometheus/common",
        sum = "h1:hWIdL3N2HoUx3B8j3YN9mWor0qhY/NlEKZEaXxuIRh4=",
        version = "v0.32.1",
    )
    go_repository(
        name = "com_github_prometheus_procfs",
        importpath = "github.com/prometheus/procfs",
        sum = "h1:4jVXhlkAyzOScmCkXBTOLRLTz8EeU+eyjrwB/EPq0VU=",
        version = "v0.7.3",
    )
    go_repository(
        name = "com_github_puerkitobio_goquery",
        importpath = "github.com/PuerkitoBio/goquery",
        sum = "h1:uQxhNlArOIdbrH1tr0UXwdVFgDcZDrZVdcpygAcwmWM=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_github_rogpeppe_go_internal",
        importpath = "github.com/rogpeppe/go-internal",
        sum = "h1:73kH8U+JUqXU8lRuOHeVHaa/SZPifC7BkcraZVejAe8=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_github_russross_blackfriday_v2",
        importpath = "github.com/russross/blackfriday/v2",
        sum = "h1:JIOH55/0cWyOuilr9/qlrm0BSXldqnqwMsf35Ld67mk=",
        version = "v2.1.0",
    )
    go_repository(
        name = "com_github_schollz_closestmatch",
        importpath = "github.com/schollz/closestmatch",
        sum = "h1:Uel2GXEpJqOWBrlyI+oY9LTiyyjYS17cCYRqP13/SHk=",
        version = "v2.1.0+incompatible",
    )
    go_repository(
        name = "com_github_sebdah_goldie_v2",
        importpath = "github.com/sebdah/goldie/v2",
        sum = "h1:9ES/mNN+HNUbNWpVAlrzuZ7jE+Nrczbj8uFRjM7624Y=",
        version = "v2.5.3",
    )
    go_repository(
        name = "com_github_sergi_go_diff",
        importpath = "github.com/sergi/go-diff",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:xkr+Oxo4BOQKmkn/B9eMK0g5Kg/983T9DqqPHwYqD+8=",
        version = "v1.3.1",
    )
    go_repository(
        name = "com_github_shopify_goreferrer",
        importpath = "github.com/Shopify/goreferrer",
        sum = "h1:KkH3I3sJuOLP3TjA/dfr4NAY8bghDwnXiU7cTKxQqo0=",
        version = "v0.0.0-20220729165902-8cddb4f5de06",
    )
    go_repository(
        name = "com_github_sirupsen_logrus",
        importpath = "github.com/sirupsen/logrus",
        sum = "h1:trlNQbNUG3OdDrDil03MCb1H2o9nJ1x4/5LYw7byDE0=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_github_sourcegraph_go_langserver",
        importpath = "github.com/sourcegraph/go-langserver",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:lj2sRU7ZMIkW372IDVGb6fE8VAY4c/EMsiDzrB9vmiU=",
        version = "v2.0.0+incompatible",
    )
    go_repository(
        name = "com_github_sourcegraph_jsonrpc2",
        importpath = "github.com/sourcegraph/jsonrpc2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:KjN/dC4fP6aN9030MZCJs9WQbTOjWHhrtKVpzzSrr/U=",
        version = "v0.2.0",
    )
    go_repository(
        name = "com_github_spf13_cobra",
        importpath = "github.com/spf13/cobra",
        sum = "h1:6m/oheQuQ13N9ks4hubMG6BnvwOeaJrqSPLahSnczz8=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_spf13_pflag",
        importpath = "github.com/spf13/pflag",
        sum = "h1:iy+VFUOCP1a+8yFto/drg2CJ5u0yRoB7fZw3DKv/JXA=",
        version = "v1.0.5",
    )
    go_repository(
        name = "com_github_stretchr_objx",
        importpath = "github.com/stretchr/objx",
        sum = "h1:2vfRuCMp5sSVIDSqO8oNnWJq7mPa6KVP3iPIwFBuy8A=",
        version = "v0.1.1",
    )
    go_repository(
        name = "com_github_stretchr_testify",
        importpath = "github.com/stretchr/testify",
        sum = "h1:HtqpIVDClZ4nwg75+f6Lvsy/wHu+3BoSGCbBAcpTsTg=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_github_syndtr_goleveldb",
        importpath = "github.com/syndtr/goleveldb",
        sum = "h1:fBdIW9lB4Iz0n9khmH8w27SJ3QEJ7+IgjPEwGSZiFdE=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_tdewolff_minify_v2",
        importpath = "github.com/tdewolff/minify/v2",
        sum = "h1:kejsHQMM17n6/gwdw53qsi6lg0TGddZADVyQOz1KMdE=",
        version = "v2.12.4",
    )
    go_repository(
        name = "com_github_tdewolff_parse_v2",
        importpath = "github.com/tdewolff/parse/v2",
        sum = "h1:KCkDvNUMof10e3QExio9OPZJT8SbdKojLBumw8YZycQ=",
        version = "v2.6.4",
    )
    go_repository(
        name = "com_github_ugorji_go_codec",
        importpath = "github.com/ugorji/go/codec",
        sum = "h1:YPXUKf7fYbp/y8xloBqZOw2qaVggbfwMlI8WM3wZUJ0=",
        version = "v1.2.7",
    )
    go_repository(
        name = "com_github_urfave_negroni",
        importpath = "github.com/urfave/negroni",
        sum = "h1:kIimOitoypq34K7TG7DUaJ9kq/N4Ofuwi1sjz0KipXc=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_valyala_bytebufferpool",
        importpath = "github.com/valyala/bytebufferpool",
        sum = "h1:GqA5TC/0021Y/b9FG4Oi9Mr3q7XYx6KllzawFIhcdPw=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_valyala_fasthttp",
        importpath = "github.com/valyala/fasthttp",
        sum = "h1:CRq/00MfruPGFLTQKY8b+8SfdK60TxNztjRMnH0t1Yc=",
        version = "v1.40.0",
    )
    go_repository(
        name = "com_github_valyala_fasttemplate",
        importpath = "github.com/valyala/fasttemplate",
        sum = "h1:TVEnxayobAdVkhQfrfes2IzOB6o+z4roRkPF52WA1u4=",
        version = "v1.2.1",
    )
    go_repository(
        name = "com_github_vmihailenco_msgpack_v5",
        importpath = "github.com/vmihailenco/msgpack/v5",
        sum = "h1:5gO0H1iULLWGhs2H5tbAHIZTV8/cYafcFOr9znI5mJU=",
        version = "v5.3.5",
    )
    go_repository(
        name = "com_github_vmihailenco_tagparser_v2",
        importpath = "github.com/vmihailenco/tagparser/v2",
        sum = "h1:y09buUbR+b5aycVFQs/g70pqKVZNBmxwAhO7/IwNM9g=",
        version = "v2.0.0",
    )
    go_repository(
        name = "com_github_yosssi_ace",
        importpath = "github.com/yosssi/ace",
        sum = "h1:tUkIP/BLdKqrlrPwcmH0shwEEhTRHoGnc1wFIWmaBUA=",
        version = "v0.0.5",
    )
    go_repository(
        name = "com_google_cloud_go",
        importpath = "cloud.google.com/go",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:tvZe1mgqRxpiVa3XlIGMiPcEUbP1gNXELgD4y/IXmeQ=",
        version = "v0.118.0",
    )
    go_repository(
        name = "com_google_cloud_go_accessapproval",
        importpath = "cloud.google.com/go/accessapproval",
        sum = "h1:axlU03FRiXDNupsmPG7LKzuS4Enk1gf598M62lWVB74=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_accesscontextmanager",
        importpath = "cloud.google.com/go/accesscontextmanager",
        sum = "h1:8zVoeiBa4erMCLEXltOcqVEsZhS26JZ5/Vrgs59eQiI=",
        version = "v1.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_aiplatform",
        importpath = "cloud.google.com/go/aiplatform",
        sum = "h1:vnqsPkgcwlDEpWl9t6C3/HLfHeweuGXs2gcYTzH6dMs=",
        version = "v1.70.0",
    )
    go_repository(
        name = "com_google_cloud_go_analytics",
        importpath = "cloud.google.com/go/analytics",
        sum = "h1:hX6JAsNbXd2uVjqjIuMcKpmhIybKrEunBiGxK4SwEFI=",
        version = "v0.25.3",
    )
    go_repository(
        name = "com_google_cloud_go_apigateway",
        importpath = "cloud.google.com/go/apigateway",
        sum = "h1:Mn7cC5iWJz+cSMS/Hb+N2410CpZ6c8XpJKaexBl0Gxs=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_apigeeconnect",
        importpath = "cloud.google.com/go/apigeeconnect",
        sum = "h1:Wlr+30Tha0SMCvQYZKdrh+HkpOyl0CQFSlzeY/Gg1gs=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_apigeeregistry",
        importpath = "cloud.google.com/go/apigeeregistry",
        sum = "h1:j9CJg/oC884OX5cDpiwNt1ZlDXNV6Zb9Mp1YmRrOG0k=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_appengine",
        importpath = "cloud.google.com/go/appengine",
        sum = "h1:jrcanSzj9J1erevZuxldvsDwY+0k/DeFFzlnSfPGfL8=",
        version = "v1.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_area120",
        importpath = "cloud.google.com/go/area120",
        sum = "h1:dPQ07rW4eku8OgNWDOaQaVGcE4+XfhH8BSbVwdVQ+wU=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_artifactregistry",
        importpath = "cloud.google.com/go/artifactregistry",
        sum = "h1:ZNXGB6+T7VmWdf6//VqxLdZ/sk0no8W0ujanHeJwDRw=",
        version = "v1.16.1",
    )
    go_repository(
        name = "com_google_cloud_go_asset",
        importpath = "cloud.google.com/go/asset",
        sum = "h1:6oNgjcs5KCPGBD71G0IccK6TfeFsEtBTyQ3Q+Dn09bs=",
        version = "v1.20.4",
    )
    go_repository(
        name = "com_google_cloud_go_assuredworkloads",
        importpath = "cloud.google.com/go/assuredworkloads",
        sum = "h1:RU1WhF1zMggdXAZ+ezYTn4Eh/FdiX7sz8lLXGERn4Po=",
        version = "v1.12.3",
    )
    go_repository(
        name = "com_google_cloud_go_auth",
        importpath = "cloud.google.com/go/auth",
        sum = "h1:8Fu8TZy167JkW8Tj3q7dIkr2v4cndv41ouecJx0PAHs=",
        version = "v0.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_auth_oauth2adapt",
        importpath = "cloud.google.com/go/auth/oauth2adapt",
        sum = "h1:V6a6XDu2lTwPZWOawrAa9HUK+DB2zfJyTuciBG5hFkU=",
        version = "v0.2.6",
    )
    go_repository(
        name = "com_google_cloud_go_automl",
        importpath = "cloud.google.com/go/automl",
        sum = "h1:vkD+hQ75SMINMgJBT/KDpFYvfQLzJbtIQZdw0AWq8Rs=",
        version = "v1.14.4",
    )
    go_repository(
        name = "com_google_cloud_go_baremetalsolution",
        importpath = "cloud.google.com/go/baremetalsolution",
        sum = "h1:OL+KT+wCumdDhG44aeqGAdkwdT8Wa4Lh+o4INM+CQjw=",
        version = "v1.3.3",
    )
    go_repository(
        name = "com_google_cloud_go_batch",
        importpath = "cloud.google.com/go/batch",
        sum = "h1:TLfFZJXu+89CGbDK2mMql8f6HHFXarr8uUsaQ6wKatU=",
        version = "v1.11.5",
    )
    go_repository(
        name = "com_google_cloud_go_beyondcorp",
        importpath = "cloud.google.com/go/beyondcorp",
        sum = "h1:ezavJc0Gzh4N8zBskO/DnUVMWPa8lqH/tmQSyaknmCA=",
        version = "v1.1.3",
    )
    go_repository(
        name = "com_google_cloud_go_bigquery",
        importpath = "cloud.google.com/go/bigquery",
        sum = "h1:ZZ1EOJMHTYf6R9lhxIXZJic1qBD4/x9loBIS+82moUs=",
        version = "v1.65.0",
    )
    go_repository(
        name = "com_google_cloud_go_bigtable",
        importpath = "cloud.google.com/go/bigtable",
        sum = "h1:eIgi3QLcN4aq8p6n9U/zPgmHeBP34sm9FiKq4ik/ZoY=",
        version = "v1.34.0",
    )
    go_repository(
        name = "com_google_cloud_go_billing",
        importpath = "cloud.google.com/go/billing",
        sum = "h1:xMlO3hc5BI0s23tRB40bL40xSpxUR1x3E07Y5/VWcjU=",
        version = "v1.20.1",
    )
    go_repository(
        name = "com_google_cloud_go_binaryauthorization",
        importpath = "cloud.google.com/go/binaryauthorization",
        sum = "h1:X8JRfmk0/vyRqLusEyAPr0nZCK6RKae9omB4lrit0XI=",
        version = "v1.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_certificatemanager",
        importpath = "cloud.google.com/go/certificatemanager",
        sum = "h1:2UP31fg7b+y3F0OmNbPHOKPEJ+6LOMfxAXX4p8xGCy4=",
        version = "v1.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_channel",
        importpath = "cloud.google.com/go/channel",
        sum = "h1:oHyO3QAZ6kdf6SwqnUTBz50ND6Nk2rxZtboUiF4dgLE=",
        version = "v1.19.2",
    )
    go_repository(
        name = "com_google_cloud_go_cloudbuild",
        importpath = "cloud.google.com/go/cloudbuild",
        sum = "h1:fYsJweKNT1b9cCQHoE3499n1Olr+7z50Ep7jnA+szDs=",
        version = "v1.19.2",
    )
    go_repository(
        name = "com_google_cloud_go_clouddms",
        importpath = "cloud.google.com/go/clouddms",
        sum = "h1:T/rkkKE0KhQFMcO3+QWL82xakA9kRumLXY1lq5adIts=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_cloudtasks",
        importpath = "cloud.google.com/go/cloudtasks",
        sum = "h1:rXdznKjCa7WpzmvR2plrn2KJ+RZC1oYxPiRWNQjjf3k=",
        version = "v1.13.3",
    )
    go_repository(
        name = "com_google_cloud_go_compute",
        importpath = "cloud.google.com/go/compute",
        sum = "h1:SObuy8Fs6woazArpXp1fsHCw+ZH4iJ/8dGGTxUhHZQA=",
        version = "v1.31.1",
    )
    go_repository(
        name = "com_google_cloud_go_compute_metadata",
        importpath = "cloud.google.com/go/compute/metadata",
        sum = "h1:A6hENjEsCDtC1k8byVsgwvVcioamEHvZ4j01OwKxG9I=",
        version = "v0.6.0",
    )
    go_repository(
        name = "com_google_cloud_go_contactcenterinsights",
        importpath = "cloud.google.com/go/contactcenterinsights",
        sum = "h1:xJoZbX0HM1zht8KxAB38hs2v4Hcl+vXGLo454LrdwxA=",
        version = "v1.17.1",
    )
    go_repository(
        name = "com_google_cloud_go_container",
        importpath = "cloud.google.com/go/container",
        sum = "h1:eaMrgOl6NCk+Blhh29GgUVe3QGo7IiJQlP0w/EwLoV0=",
        version = "v1.42.1",
    )
    go_repository(
        name = "com_google_cloud_go_containeranalysis",
        importpath = "cloud.google.com/go/containeranalysis",
        sum = "h1:1D8U75BeotZxrG4jR6NYBtOt+uAeBsWhpBZmSYLakQw=",
        version = "v0.13.3",
    )
    go_repository(
        name = "com_google_cloud_go_datacatalog",
        importpath = "cloud.google.com/go/datacatalog",
        sum = "h1:3bAfstDB6rlHyK0TvqxEwaeOvoN9UgCs2bn03+VXmss=",
        version = "v1.24.3",
    )
    go_repository(
        name = "com_google_cloud_go_dataflow",
        importpath = "cloud.google.com/go/dataflow",
        sum = "h1:+7IfIXzYWSybIIDGK9FN2uqBsP/5b/Y0pBYzNhcmKSU=",
        version = "v0.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_dataform",
        importpath = "cloud.google.com/go/dataform",
        sum = "h1:ZpGkZV8OyhUhvN/tfLffU2ki5ERTtqOunkIaiVAhmw0=",
        version = "v0.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_datafusion",
        importpath = "cloud.google.com/go/datafusion",
        sum = "h1:FTMtsf2nfGGlDCuE84/RvVaCcTIYE7WQSB0noeO0cwI=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_datalabeling",
        importpath = "cloud.google.com/go/datalabeling",
        sum = "h1:PqoA3gnOWaLcHCnqoZe4jh3jmiv6+Z7W2xUUkw/j4jE=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_dataplex",
        importpath = "cloud.google.com/go/dataplex",
        sum = "h1:oswf105Cr2EwHrW2n7wk3nRZQf7hCe3apE/GqJ8yjvY=",
        version = "v1.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_dataproc_v2",
        importpath = "cloud.google.com/go/dataproc/v2",
        sum = "h1:2vOv471LrcSn91VNzijcH+OkDRLa3kdyymOfKqbwZ4c=",
        version = "v2.10.1",
    )
    go_repository(
        name = "com_google_cloud_go_dataqna",
        importpath = "cloud.google.com/go/dataqna",
        sum = "h1:lGUj2FYs650EUPDMV6plWBAoh8qH9Bu1KCz1PUYF2VY=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_datastore",
        importpath = "cloud.google.com/go/datastore",
        sum = "h1:NNpXoyEqIJmZFc0ACcwBEaXnmscUpcG4NkKnbCePmiM=",
        version = "v1.20.0",
    )
    go_repository(
        name = "com_google_cloud_go_datastream",
        importpath = "cloud.google.com/go/datastream",
        sum = "h1:j5cIRYJHjx/058aHa4Slip7fl62UTGHCJc4GL9bxQLQ=",
        version = "v1.12.1",
    )
    go_repository(
        name = "com_google_cloud_go_deploy",
        importpath = "cloud.google.com/go/deploy",
        sum = "h1:Hm3pXBzMFJFPOdwtDkg5e/LP53bXqIpwQpjwsVasjhU=",
        version = "v1.26.1",
    )
    go_repository(
        name = "com_google_cloud_go_dialogflow",
        importpath = "cloud.google.com/go/dialogflow",
        sum = "h1:6fU4IKLpvgpXqiUCE8gUp8eV5u629SCtiyXMudXtZSg=",
        version = "v1.64.1",
    )
    go_repository(
        name = "com_google_cloud_go_dlp",
        importpath = "cloud.google.com/go/dlp",
        sum = "h1:qAEGTTtC97zuDm6YPBozNvy4BLBszVCJah3efNytl3g=",
        version = "v1.20.1",
    )
    go_repository(
        name = "com_google_cloud_go_documentai",
        importpath = "cloud.google.com/go/documentai",
        sum = "h1:52RfiUsoblXcE57CfKJGnITWLxRM30BcqNk/BKZl2LI=",
        version = "v1.35.1",
    )
    go_repository(
        name = "com_google_cloud_go_domains",
        importpath = "cloud.google.com/go/domains",
        sum = "h1:wnqN5YwMrtLSjn+HB2sChgmZ6iocOta4Q41giQsiRjY=",
        version = "v0.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_edgecontainer",
        importpath = "cloud.google.com/go/edgecontainer",
        sum = "h1:SwQuHQiheVfL7b5ar/AXDberiaqr/yiue8X55AdWnZU=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_google_cloud_go_errorreporting",
        importpath = "cloud.google.com/go/errorreporting",
        sum = "h1:isaoPwWX8kbAOea4qahcmttoS79+gQhvKsfg5L5AgH8=",
        version = "v0.3.2",
    )
    go_repository(
        name = "com_google_cloud_go_essentialcontacts",
        importpath = "cloud.google.com/go/essentialcontacts",
        sum = "h1:Paw495vxVyKuAgcQ2NQk09iRZBhPYRytknydEnvzcv4=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_eventarc",
        importpath = "cloud.google.com/go/eventarc",
        sum = "h1:RMymT7R87LaxKugOKwooOoheWXUm1NMeOfh3CVU9g54=",
        version = "v1.15.1",
    )
    go_repository(
        name = "com_google_cloud_go_filestore",
        importpath = "cloud.google.com/go/filestore",
        sum = "h1:vTXQI5qYKZ8dmCyHN+zVfaMyXCYbyZNM0CkPzpPUn7Q=",
        version = "v1.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_firestore",
        importpath = "cloud.google.com/go/firestore",
        sum = "h1:cuydCaLS7Vl2SatAeivXyhbhDEIR8BDmtn4egDhIn2s=",
        version = "v1.18.0",
    )
    go_repository(
        name = "com_google_cloud_go_functions",
        importpath = "cloud.google.com/go/functions",
        sum = "h1:V0vCHSgFTUqKn57+PUXp1UfQY0/aMkveAw7wXeM3Lq0=",
        version = "v1.19.3",
    )
    go_repository(
        name = "com_google_cloud_go_gkebackup",
        importpath = "cloud.google.com/go/gkebackup",
        sum = "h1:djdExe/QgoKdp1gnIO1G5BoO1o/yGQOQJJEZ4QKTEXQ=",
        version = "v1.6.3",
    )
    go_repository(
        name = "com_google_cloud_go_gkeconnect",
        importpath = "cloud.google.com/go/gkeconnect",
        sum = "h1:YVpR0vlHSP/wD74PXEbKua4Aamud+wiYm4TiewNjD3M=",
        version = "v0.12.1",
    )
    go_repository(
        name = "com_google_cloud_go_gkehub",
        importpath = "cloud.google.com/go/gkehub",
        sum = "h1:yZ6lNJ9rNIoQmWrG14dB3+BFjS/EIRBf7Bo6jc5QWlE=",
        version = "v0.15.3",
    )
    go_repository(
        name = "com_google_cloud_go_gkemulticloud",
        importpath = "cloud.google.com/go/gkemulticloud",
        sum = "h1:8Z1rWFbnNGgB3KMFbg8OCiIiw2Hl1nxZWwZGU740XRs=",
        version = "v1.5.0",
    )
    go_repository(
        name = "com_google_cloud_go_gsuiteaddons",
        importpath = "cloud.google.com/go/gsuiteaddons",
        sum = "h1:QafYhVhyFGpidBUUlVhy6lUHFogFOycVYm9DV7MinhA=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_iam",
        importpath = "cloud.google.com/go/iam",
        sum = "h1:KFf8SaT71yYq+sQtRISn90Gyhyf4X8RGgeAVC8XGf3E=",
        version = "v1.3.1",
    )
    go_repository(
        name = "com_google_cloud_go_iap",
        importpath = "cloud.google.com/go/iap",
        sum = "h1:OWNYFHPyIBNHEAEFdVKOltYWe0g3izSrpFJW6Iidovk=",
        version = "v1.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_ids",
        importpath = "cloud.google.com/go/ids",
        sum = "h1:wbFF7twu0XScFr+dtsVxTTttbFIRYt/SJjZiHFidtYE=",
        version = "v1.5.3",
    )
    go_repository(
        name = "com_google_cloud_go_iot",
        importpath = "cloud.google.com/go/iot",
        sum = "h1:aPWYQ+A1NX6ou/5U0nFAiXWdVT8OBxZYVZt2fBl2gWA=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_kms",
        importpath = "cloud.google.com/go/kms",
        sum = "h1:aQQ8esAIVZ1atdJRxihhdxGQ64/zEbJoJnCz/ydSmKg=",
        version = "v1.20.5",
    )
    go_repository(
        name = "com_google_cloud_go_language",
        importpath = "cloud.google.com/go/language",
        sum = "h1:8hmFMiS3wjjj3TX/U1zZYTgzwZoUjDbo9PaqcYEmuB4=",
        version = "v1.14.3",
    )
    go_repository(
        name = "com_google_cloud_go_lifesciences",
        importpath = "cloud.google.com/go/lifesciences",
        sum = "h1:Z05C+Ui953f0EQx9hJ1la6+QQl8ADrIs3iNwP5Elkpg=",
        version = "v0.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_logging",
        importpath = "cloud.google.com/go/logging",
        sum = "h1:7j0HgAp0B94o1YRDqiqm26w4q1rDMH7XNRU34lJXHYc=",
        version = "v1.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_longrunning",
        importpath = "cloud.google.com/go/longrunning",
        sum = "h1:3tyw9rO3E2XVXzSApn1gyEEnH2K9SynNQjMlBi3uHLg=",
        version = "v0.6.4",
    )
    go_repository(
        name = "com_google_cloud_go_managedidentities",
        importpath = "cloud.google.com/go/managedidentities",
        sum = "h1:b9xGs24BIjfyvLgCtJoClOZpPi8d8owPgWe5JEINgaY=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_maps",
        importpath = "cloud.google.com/go/maps",
        sum = "h1:u7U/DieTxYYMDyvHQ00la5ayXLjDImTfnhdAsyPZXyY=",
        version = "v1.17.1",
    )
    go_repository(
        name = "com_google_cloud_go_mediatranslation",
        importpath = "cloud.google.com/go/mediatranslation",
        sum = "h1:nRBjeaMLipw05Br+qDAlSCcCQAAlat4mvpafztbEVgc=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_memcache",
        importpath = "cloud.google.com/go/memcache",
        sum = "h1:XH/qT3GbbSH//R0JTqR77lRpBxaa0N9sHgAzfwbTrv0=",
        version = "v1.11.3",
    )
    go_repository(
        name = "com_google_cloud_go_metastore",
        importpath = "cloud.google.com/go/metastore",
        sum = "h1:jDqeCw6NGDRAPT9+2Y/EjnWAB0BfCcUfmPLOyhB0eHs=",
        version = "v1.14.3",
    )
    go_repository(
        name = "com_google_cloud_go_monitoring",
        importpath = "cloud.google.com/go/monitoring",
        sum = "h1:KQbnAC4IAH+5x3iWuPZT5iN9VXqKMzzOgqcYB6fqPDE=",
        version = "v1.22.1",
    )
    go_repository(
        name = "com_google_cloud_go_networkconnectivity",
        importpath = "cloud.google.com/go/networkconnectivity",
        sum = "h1:YsVhG71ZC4FkqCP2oCI55x/JeGFyd7738Lt8iNTrzJw=",
        version = "v1.16.1",
    )
    go_repository(
        name = "com_google_cloud_go_networkmanagement",
        importpath = "cloud.google.com/go/networkmanagement",
        sum = "h1:nnOEtE8HGt5zm0SZ0TUTAKAqSQaLdqALW56/0yBoyAg=",
        version = "v1.17.1",
    )
    go_repository(
        name = "com_google_cloud_go_networksecurity",
        importpath = "cloud.google.com/go/networksecurity",
        sum = "h1:JLJBFbxc8D7/OS81MyRoKhc2OvnVJxy5VMoQqqAhA7k=",
        version = "v0.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_notebooks",
        importpath = "cloud.google.com/go/notebooks",
        sum = "h1:+9DrGJcZhCu6B2t0JJorekjIUBvg/KvBmXJYGmfvVvA=",
        version = "v1.12.3",
    )
    go_repository(
        name = "com_google_cloud_go_optimization",
        importpath = "cloud.google.com/go/optimization",
        sum = "h1:JwQjjoBZJpsoMQe/3mhVBMVZuSdagHg2pGOnwh2Jk+E=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_orchestration",
        importpath = "cloud.google.com/go/orchestration",
        sum = "h1:ug13kXJRdaZO94IgACVF4JwItuVnU0WFbHcJ5Z6ioNs=",
        version = "v1.11.3",
    )
    go_repository(
        name = "com_google_cloud_go_orgpolicy",
        importpath = "cloud.google.com/go/orgpolicy",
        sum = "h1:WFvgmjq/FO5GiXlhebltA9N14KdbLMcgG88ME+SWeBo=",
        version = "v1.14.2",
    )
    go_repository(
        name = "com_google_cloud_go_osconfig",
        importpath = "cloud.google.com/go/osconfig",
        sum = "h1:cyf1PMK5c2/WOIr5r2lxjH/XBJMA9P4zC8Tm10i0z3M=",
        version = "v1.14.3",
    )
    go_repository(
        name = "com_google_cloud_go_oslogin",
        importpath = "cloud.google.com/go/oslogin",
        sum = "h1:yomxnFPk+ye0zd0mJ15nn9fH4Ns7ex4xA3ll+u2q59A=",
        version = "v1.14.3",
    )
    go_repository(
        name = "com_google_cloud_go_phishingprotection",
        importpath = "cloud.google.com/go/phishingprotection",
        sum = "h1:T5mGFV0ggBKg3qt9myFRiGJu+nIUucuHLAtVpAuQ08I=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_policytroubleshooter",
        importpath = "cloud.google.com/go/policytroubleshooter",
        sum = "h1:ekIWI8JbKkpOfrgH/THGamQE/D16tcVBYJyrkseVcYI=",
        version = "v1.11.3",
    )
    go_repository(
        name = "com_google_cloud_go_privatecatalog",
        importpath = "cloud.google.com/go/privatecatalog",
        sum = "h1:fu2LABMi7CgZORQ2oNGbc0hoZ0FTqLkjGqIgAV/Kc7U=",
        version = "v0.10.4",
    )
    go_repository(
        name = "com_google_cloud_go_pubsub",
        importpath = "cloud.google.com/go/pubsub",
        sum = "h1:prYj8EEAAAwkp6WNoGTE4ahe0DgHoyJd5Pbop931zow=",
        version = "v1.45.3",
    )
    go_repository(
        name = "com_google_cloud_go_pubsublite",
        importpath = "cloud.google.com/go/pubsublite",
        sum = "h1:jLQozsEVr+c6tOU13vDugtnaBSUy/PD5zK6mhm+uF1Y=",
        version = "v1.8.2",
    )
    go_repository(
        name = "com_google_cloud_go_recaptchaenterprise_v2",
        importpath = "cloud.google.com/go/recaptchaenterprise/v2",
        sum = "h1:8tyxuDlUnyNsLd3Pf2qfdj6pjwE5UxQCLJdmPX8NIpk=",
        version = "v2.19.3",
    )
    go_repository(
        name = "com_google_cloud_go_recommendationengine",
        importpath = "cloud.google.com/go/recommendationengine",
        sum = "h1:kBpcYPx4ys4lrDGKp4OhP2uy8h7UjlmLW/qoO5Xb2bY=",
        version = "v0.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_recommender",
        importpath = "cloud.google.com/go/recommender",
        sum = "h1:dVlOjxsbjuhlwu4MIcyPWe09qVcDqc419iOjdPl5RHk=",
        version = "v1.13.3",
    )
    go_repository(
        name = "com_google_cloud_go_redis",
        importpath = "cloud.google.com/go/redis",
        sum = "h1:ROQXi5dCDSJCVezt/2nD1g+Ym0T6sio3DIzZ56NgMZI=",
        version = "v1.17.3",
    )
    go_repository(
        name = "com_google_cloud_go_resourcemanager",
        importpath = "cloud.google.com/go/resourcemanager",
        sum = "h1:SHOMw0kX0xWratC5Vb5VULBeWiGlPYAs82kiZqNtWpM=",
        version = "v1.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_resourcesettings",
        importpath = "cloud.google.com/go/resourcesettings",
        sum = "h1:13HOFU7v4cEvIHXSAQbinF4wp2Baybbq7q9FMctg1Ek=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_retail",
        importpath = "cloud.google.com/go/retail",
        sum = "h1:PT6CUlazIFIOLLJnV+bPBtiSH8iusKZ+FZRzZYFt2vk=",
        version = "v1.19.2",
    )
    go_repository(
        name = "com_google_cloud_go_run",
        importpath = "cloud.google.com/go/run",
        sum = "h1:aeVLygw0BGLH+Zbj8v3K3nEHvKlgoq+j8fcRJaYZtxY=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_scheduler",
        importpath = "cloud.google.com/go/scheduler",
        sum = "h1:p6+h8BoYJC+TvUijGBfORN6nuhOvJ3EwZ2H84CZ1ZEU=",
        version = "v1.11.3",
    )
    go_repository(
        name = "com_google_cloud_go_secretmanager",
        importpath = "cloud.google.com/go/secretmanager",
        sum = "h1:XVGHbcXEsbrgi4XHzgK5np81l1eO7O72WOXHhXUemrM=",
        version = "v1.14.3",
    )
    go_repository(
        name = "com_google_cloud_go_security",
        importpath = "cloud.google.com/go/security",
        sum = "h1:ya9gfY1ign6Yy25VMMMgZ9xy7D/TczDB0ElXcyWmEVE=",
        version = "v1.18.3",
    )
    go_repository(
        name = "com_google_cloud_go_securitycenter",
        importpath = "cloud.google.com/go/securitycenter",
        sum = "h1:H8UvBpcvs1OjI4jZuXX8xsN1IZo88a9PezHXkU2sGps=",
        version = "v1.35.3",
    )
    go_repository(
        name = "com_google_cloud_go_servicedirectory",
        importpath = "cloud.google.com/go/servicedirectory",
        sum = "h1:oFkCp6ti7fc7hzeROmOPQuPBHFqwyhcsv3Yrma28+uc=",
        version = "v1.12.3",
    )
    go_repository(
        name = "com_google_cloud_go_shell",
        importpath = "cloud.google.com/go/shell",
        sum = "h1:mjYgUsOtV3jl9xvDmcvlRRmA64deEPf52zOfuc68b/g=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_spanner",
        importpath = "cloud.google.com/go/spanner",
        sum = "h1:0bab8QDn6MNj9lNK6XyGAVFhMlhMU2waePPa6GZNoi8=",
        version = "v1.73.0",
    )
    go_repository(
        name = "com_google_cloud_go_speech",
        importpath = "cloud.google.com/go/speech",
        sum = "h1:qvURtJs7BQzQhbxWxwai0pT79S8KLVKJ/4W8igVkt1Y=",
        version = "v1.26.0",
    )
    go_repository(
        name = "com_google_cloud_go_storage",
        importpath = "cloud.google.com/go/storage",
        sum = "h1:CcxnSohZwizt4LCzQHWvBf1/kvtHUn7gk9QERXPyXFs=",
        version = "v1.43.0",
    )
    go_repository(
        name = "com_google_cloud_go_storagetransfer",
        importpath = "cloud.google.com/go/storagetransfer",
        sum = "h1:W3v9A7MGBN7H9sAFstyciwP/1XEQhUhZfrjclmDnpMs=",
        version = "v1.12.1",
    )
    go_repository(
        name = "com_google_cloud_go_talent",
        importpath = "cloud.google.com/go/talent",
        sum = "h1:olv+s2g+LGXeJi+MYF1wI44/TwHaVnO0N7PiucVf5ZQ=",
        version = "v1.8.0",
    )
    go_repository(
        name = "com_google_cloud_go_texttospeech",
        importpath = "cloud.google.com/go/texttospeech",
        sum = "h1:YF/RdNb+jUEp22cIZCvqiFjfA5OxGE+Dxss3mhXU7oQ=",
        version = "v1.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_tpu",
        importpath = "cloud.google.com/go/tpu",
        sum = "h1:PszqG+pvC7u/cv51GWQIN9M++jciIBr5vVn6/MWzU8I=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_trace",
        importpath = "cloud.google.com/go/trace",
        sum = "h1:c+I4YFjxRQjvAhRmSsmjpASUKq88chOX854ied0K/pE=",
        version = "v1.11.3",
    )
    go_repository(
        name = "com_google_cloud_go_translate",
        importpath = "cloud.google.com/go/translate",
        sum = "h1:XJ7LipYJi80BCgVk2lx1fwc7DIYM6oV2qx1G4IAGQ5w=",
        version = "v1.12.3",
    )
    go_repository(
        name = "com_google_cloud_go_video",
        importpath = "cloud.google.com/go/video",
        sum = "h1:C2FH+6yr6LCZC4fP0gm9FwJB/SRh5Ul88O5Sc/bL83I=",
        version = "v1.23.3",
    )
    go_repository(
        name = "com_google_cloud_go_videointelligence",
        importpath = "cloud.google.com/go/videointelligence",
        sum = "h1:zNTOUQyatGQtnCJ2dR3faRtpWQOlC8wszJqwG5CtwVM=",
        version = "v1.12.3",
    )
    go_repository(
        name = "com_google_cloud_go_vision_v2",
        importpath = "cloud.google.com/go/vision/v2",
        sum = "h1:dPvfDuPqPH+Yscf0f2f1RprvKkoo+N/j0a+IbLYX7Cs=",
        version = "v2.9.3",
    )
    go_repository(
        name = "com_google_cloud_go_vmmigration",
        importpath = "cloud.google.com/go/vmmigration",
        sum = "h1:dpCQq3pj2HnKdbvGTftdWymm3r4ovF7JW5z8xBcO2x4=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_vmwareengine",
        importpath = "cloud.google.com/go/vmwareengine",
        sum = "h1:TfuQr5j7qriINulUMotaC/+27SQaW2thIkF3Gb6VJ38=",
        version = "v1.3.3",
    )
    go_repository(
        name = "com_google_cloud_go_vpcaccess",
        importpath = "cloud.google.com/go/vpcaccess",
        sum = "h1:vxVaoFM64M/ht619c4wZNF0iq0QPaMWElOh7Ns4r41A=",
        version = "v1.8.3",
    )
    go_repository(
        name = "com_google_cloud_go_webrisk",
        importpath = "cloud.google.com/go/webrisk",
        sum = "h1:yh0v/5n49VO4/i9pYfDm1gLJUj1Ph3Xzegn8WvK9YRA=",
        version = "v1.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_websecurityscanner",
        importpath = "cloud.google.com/go/websecurityscanner",
        sum = "h1:/uxhVCWKXzPw5pVfnBOVjaSiQ6Bm0tDExDOCLV40thw=",
        version = "v1.7.3",
    )
    go_repository(
        name = "com_google_cloud_go_workflows",
        importpath = "cloud.google.com/go/workflows",
        sum = "h1:lNFDMranJymDEB7cTI7DI9czbc1WU0RWY9KCEv9zuDY=",
        version = "v1.13.3",
    )
    go_repository(
        name = "com_shuralyov_dmitri_gpu_mtl",
        importpath = "dmitri.shuralyov.com/gpu/mtl",
        sum = "h1:VpgP7xuJadIUuKccphEpTJnWhS2jkQyMt6Y7pJCD7fY=",
        version = "v0.0.0-20190408044501-666a987793e9",
    )
    go_repository(
        name = "dev_cel_expr",
        importpath = "cel.dev/expr",
        sum = "h1:yloc84fytn4zmJX2GU3TkXGsaieaV7dQ057Qs4sIG2Y=",
        version = "v0.16.0",
    )
    go_repository(
        name = "in_gopkg_alecthomas_kingpin_v2",
        importpath = "gopkg.in/alecthomas/kingpin.v2",
        sum = "h1:jMFz6MfLP0/4fUyZle81rXUoxOBFi19VUFKVDOQfozc=",
        version = "v2.2.6",
    )
    go_repository(
        name = "in_gopkg_check_v1",
        importpath = "gopkg.in/check.v1",
        sum = "h1:Hei/4ADfdWqJk1ZMxUNpqntNwaWcugrBjAiHlqqRiVk=",
        version = "v1.0.0-20201130134442-10cb98267c6c",
    )
    go_repository(
        name = "in_gopkg_errgo_v2",
        importpath = "gopkg.in/errgo.v2",
        sum = "h1:0vLT13EuvQ0hNvakwLuFZ/jYrLp5F3kcWHXdRggjCE8=",
        version = "v2.1.0",
    )
    go_repository(
        name = "in_gopkg_fsnotify_v1",
        importpath = "gopkg.in/fsnotify.v1",
        sum = "h1:xOHLXZwVvI9hhs+cLKq5+I5onOuwQLhQwiu63xxlHs4=",
        version = "v1.4.7",
    )
    go_repository(
        name = "in_gopkg_ini_v1",
        importpath = "gopkg.in/ini.v1",
        sum = "h1:Dgnx+6+nfE+IfzjUEISNeydPJh9AXNNsWbGP9KzCsOA=",
        version = "v1.67.0",
    )
    go_repository(
        name = "in_gopkg_tomb_v1",
        importpath = "gopkg.in/tomb.v1",
        sum = "h1:uRGJdciOHaEIrze2W8Q3AKkepLTh2hOroT7a+7czfdQ=",
        version = "v1.0.0-20141024135613-dd632973f1e7",
    )
    go_repository(
        name = "in_gopkg_yaml_v2",
        importpath = "gopkg.in/yaml.v2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:D8xgwECY7CYvx+Y2n4sBz93Jn9JRvxdiyyo8CTfuKaY=",
        version = "v2.4.0",
    )
    go_repository(
        name = "in_gopkg_yaml_v3",
        importpath = "gopkg.in/yaml.v3",
        sum = "h1:fxVm/GzAzEWqLHuvctI91KS9hhNmmWOoWu0XTYJS7CA=",
        version = "v3.0.1",
    )
    go_repository(
        name = "io_k8s_sigs_yaml",
        importpath = "sigs.k8s.io/yaml",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:a2VclLzOGrwOHDiV8EfBGhvjHvP46CtW5j6POvhYGGo=",
        version = "v1.3.0",
    )
    go_repository(
        name = "io_opencensus_go",
        importpath = "go.opencensus.io",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:y73uSU6J157QMP2kn2r30vwW1A2W2WFwSCGnAVxeaD0=",
        version = "v0.24.0",
    )
    go_repository(
        name = "io_opentelemetry_go_contrib_detectors_gcp",
        importpath = "go.opentelemetry.io/contrib/detectors/gcp",
        sum = "h1:eAaOyCwPqwAG7INWn0JTDD3KFR4qbSlhh0YCuFOmmDE=",
        version = "v1.28.0",
    )
    go_repository(
        name = "io_opentelemetry_go_contrib_instrumentation_google_golang_org_grpc_otelgrpc",
        importpath = "go.opentelemetry.io/contrib/instrumentation/google.golang.org/grpc/otelgrpc",
        sum = "h1:r6I7RJCN86bpD/FQwedZ0vSixDpwuWREjW9oRMsmqDc=",
        version = "v0.54.0",
    )
    go_repository(
        name = "io_opentelemetry_go_contrib_instrumentation_net_http_otelhttp",
        importpath = "go.opentelemetry.io/contrib/instrumentation/net/http/otelhttp",
        sum = "h1:TT4fX+nBOA/+LUkobKGW1ydGcn+G3vRw9+g5HwCphpk=",
        version = "v0.54.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel",
        importpath = "go.opentelemetry.io/otel",
        sum = "h1:PdomN/Al4q/lN6iBJEN3AwPvUiHPMlt93c8bqTG5Llw=",
        version = "v1.29.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_metric",
        importpath = "go.opentelemetry.io/otel/metric",
        sum = "h1:vPf/HFWTNkPu1aYeIsc98l4ktOQaL6LeSoeV2g+8YLc=",
        version = "v1.29.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_sdk",
        importpath = "go.opentelemetry.io/otel/sdk",
        sum = "h1:vkqKjk7gwhS8VaWb0POZKmIEDimRCMsopNYnriHyryo=",
        version = "v1.29.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_sdk_metric",
        importpath = "go.opentelemetry.io/otel/sdk/metric",
        sum = "h1:OkuaKgKrgAbYrrY0t92c+cC+2F6hsFNnCQArXCKlg08=",
        version = "v1.28.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_trace",
        importpath = "go.opentelemetry.io/otel/trace",
        sum = "h1:J/8ZNK4XgR7a21DZUAsbF8pZ5Jcw1VhACmnYt39JTi4=",
        version = "v1.29.0",
    )
    go_repository(
        name = "io_rsc_binaryregexp",
        importpath = "rsc.io/binaryregexp",
        sum = "h1:HfqmD5MEmC0zvwBuF187nq9mdnXjXsSivRiXN7SmRkE=",
        version = "v0.2.0",
    )
    go_repository(
        name = "net_starlark_go",
        importpath = "go.starlark.net",
        sum = "h1:xwwDQW5We85NaTk2APgoN9202w/l0DVGp+GZMfsrh7s=",
        version = "v0.0.0-20210223155950-e043a3d3c984",
    )
    go_repository(
        name = "org_bitbucket_creachadair_shell",
        importpath = "bitbucket.org/creachadair/shell",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Z96pB6DkSb7F3Y3BBnJeOZH2gazyMTWlvecSD4vDqfk=",
        version = "v0.0.7",
    )
    go_repository(
        name = "org_bitbucket_creachadair_stringset",
        importpath = "bitbucket.org/creachadair/stringset",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:6Sv4CCv14Wm+OipW4f3tWOb0SQVpBDLW0knnJqUnmZ8=",
        version = "v0.0.11",
    )
    go_repository(
        name = "org_golang_google_api",
        importpath = "google.golang.org/api",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:h2Gkq07OYi6kusGOaT/9rnNljuXmqPnaig7WGPmKbwA=",
        version = "v0.214.0",
    )
    go_repository(
        name = "org_golang_google_appengine",
        importpath = "google.golang.org/appengine",
        sum = "h1:IhEN5q69dyKagZPYMSdIjS2HqprW324FRQZJcGqPAsM=",
        version = "v1.6.8",
    )
    go_repository(
        name = "org_golang_google_genproto",
        importpath = "google.golang.org/genproto",
        sum = "h1:387Y+JbxF52bmesc8kq1NyYIp33dnxCw6eiA7JMsTmw=",
        version = "v0.0.0-20250115164207-1a7da9e5054f",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_api",
        importpath = "google.golang.org/genproto/googleapis/api",
        sum = "h1:GVIKPyP/kLIyVOgOnTwFOrvQaQUzOzGMCxgFUOEmm24=",
        version = "v0.0.0-20250106144421-5f5ef82da422",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_bytestream",
        importpath = "google.golang.org/genproto/googleapis/bytestream",
        sum = "h1:H8LrtQMZ6iQnV+zpgeb0YqwdByodQltmFqIhjuwexOI=",
        version = "v0.0.0-20241209162323-e6fa225c2576",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_rpc",
        importpath = "google.golang.org/genproto/googleapis/rpc",
        sum = "h1:3UsHvIr4Wc2aW4brOaSCmcxh9ksica6fHEr8P1XhkYw=",
        version = "v0.0.0-20250106144421-5f5ef82da422",
    )
    go_repository(
        name = "org_golang_google_grpc",
        importpath = "google.golang.org/grpc",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:OgPcDAFKHnH8X3O4WcO4XUc8GRDeKsKReqbQtiCj7N8=",
        version = "v1.67.3",
    )
    go_repository(
        name = "org_golang_google_grpc_cmd_protoc_gen_go_grpc",
        importpath = "google.golang.org/grpc/cmd/protoc-gen-go-grpc",
        sum = "h1:F29+wU6Ee6qgu9TddPgooOdaqsxTMunOoj8KA5yuS5A=",
        version = "v1.5.1",
    )
    go_repository(
        name = "org_golang_google_protobuf",
        importpath = "google.golang.org/protobuf",
        sum = "h1:82DV7MYdb8anAVi3qge1wSnMDrnKK7ebr+I0hHRN1BU=",
        version = "v1.36.3",
    )
    go_repository(
        name = "org_golang_x_crypto",
        importpath = "golang.org/x/crypto",
        sum = "h1:IOBPskki6Lysi0lo9qQvbxiQ+FvsCC/YWOecCHAixus=",
        version = "v0.33.0",
    )
    go_repository(
        name = "org_golang_x_exp",
        importpath = "golang.org/x/exp",
        sum = "h1:UA2aFVmmsIlefxMk29Dp2juaUSth8Pyn3Tq5Y5mJGME=",
        version = "v0.0.0-20230626212559-97b1e661b5df",
    )
    go_repository(
        name = "org_golang_x_image",
        importpath = "golang.org/x/image",
        sum = "h1:+qEpEAPhDZ1o0x3tHzZTQDArnOixOzGD9HUJfcg0mb4=",
        version = "v0.0.0-20190802002840-cff245a6509b",
    )
    go_repository(
        name = "org_golang_x_lint",
        importpath = "golang.org/x/lint",
        sum = "h1:Wh+f8QHJXR411sJR8/vRBTZ7YapZaRvUcLFFJhusH0k=",
        version = "v0.0.0-20200302205851-738671d3881b",
    )
    go_repository(
        name = "org_golang_x_mobile",
        importpath = "golang.org/x/mobile",
        sum = "h1:4+4C/Iv2U4fMZBiMCc98MG1In4gJY5YRhtpDNeDeHWs=",
        version = "v0.0.0-20190719004257-d2bd2a29d028",
    )
    go_repository(
        name = "org_golang_x_mod",
        importpath = "golang.org/x/mod",
        sum = "h1:Zb7khfcRGKk+kqfxFaP5tZqCnDZMjC5VtUBs87Hr6QM=",
        version = "v0.23.0",
    )
    go_repository(
        name = "org_golang_x_net",
        importpath = "golang.org/x/net",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:T5GQRQb2y08kTAByq9L4/bz8cipCdA8FbRTXewonqY8=",
        version = "v0.35.0",
    )
    go_repository(
        name = "org_golang_x_oauth2",
        importpath = "golang.org/x/oauth2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:KTBBxWqUa0ykRPLtV69rRto9TLXcqYkeswu48x/gvNE=",
        version = "v0.24.0",
    )
    go_repository(
        name = "org_golang_x_perf",
        importpath = "golang.org/x/perf",
        sum = "h1:ObuXPmIgI4ZMyQLIz48cJYgSyWdjUXc2SZAdyJMwEAU=",
        version = "v0.0.0-20230113213139-801c7ef9e5c5",
    )
    go_repository(
        name = "org_golang_x_sync",
        importpath = "golang.org/x/sync",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:GGz8+XQP4FvTTrjZPzNKTMFtSXH80RAzG+5ghFPgK9w=",
        version = "v0.11.0",
    )
    go_repository(
        name = "org_golang_x_sys",
        importpath = "golang.org/x/sys",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:QjkSwP/36a20jFYWkSue1YwXzLmsV5Gfq7Eiy72C1uc=",
        version = "v0.30.0",
    )
    go_repository(
        name = "org_golang_x_telemetry",
        importpath = "golang.org/x/telemetry",
        sum = "h1:zf5N6UOrA487eEFacMePxjXAJctxKmyjKUsjA11Uzuk=",
        version = "v0.0.0-20240521205824-bda55230c457",
    )
    go_repository(
        name = "org_golang_x_term",
        importpath = "golang.org/x/term",
        sum = "h1:L6pJp37ocefwRRtYPKSWOWzOtWSxVajvz2ldH/xi3iU=",
        version = "v0.29.0",
    )
    go_repository(
        name = "org_golang_x_text",
        importpath = "golang.org/x/text",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:bofq7m3/HAFvbF51jz3Q9wLg3jkvSPuiZu/pD1XwgtM=",
        version = "v0.22.0",
    )
    go_repository(
        name = "org_golang_x_time",
        importpath = "golang.org/x/time",
        sum = "h1:9i3RxcPv3PZnitoVGMPDKZSq1xW1gK1Xy3ArNOGZfEg=",
        version = "v0.8.0",
    )
    go_repository(
        name = "org_golang_x_tools_go_vcs",
        importpath = "golang.org/x/tools/go/vcs",
        sum = "h1:cOIJqWBl99H1dH5LWizPa+0ImeeJq3t3cJjaeOWUAL4=",
        version = "v0.1.0-deprecated",
    )
    go_repository(
        name = "org_golang_x_xerrors",
        importpath = "golang.org/x/xerrors",
        sum = "h1:go1bK/D/BFZV2I8cIQd1NKEZ+0owSTG1fDTci4IqFcE=",
        version = "v0.0.0-20200804184101-5ec99f83aff1",
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
    )

def _bindings():
    native.bind(
        name = "vnames_config",
        actual = "@io_kythe//kythe/data:vnames_config",
    )

    native.bind(
        name = "libuuid",
        actual = "@io_kythe//third_party:libuuid",
    )

    native.bind(
        name = "libmemcached",
        actual = "@org_libmemcached_libmemcached//:libmemcached",
    )

    native.bind(
        name = "guava",  # required by @com_google_protobuf
        actual = "@io_kythe//third_party/guava",
    )

    native.bind(
        name = "gson",  # required by @com_google_protobuf
        actual = "@maven//:com_google_code_gson_gson",
    )

    native.bind(
        name = "zlib",  # required by @com_google_protobuf
        actual = "@net_zlib//:zlib",
    )

    # This binding is needed for protobuf. See https://github.com/protocolbuffers/protobuf/pull/5811
    native.bind(
        name = "error_prone_annotations",
        actual = "@maven//:com_google_errorprone_error_prone_annotations",
    )

def kythe_dependencies():
    """Defines external repositories for Kythe dependencies.

    Call this once in your WORKSPACE file to load all @io_kythe dependencies.
    """
    bazel_skylib_workspace()
    _common_dependencies()
    _cc_dependencies()
    _go_dependencies()
    _java_dependencies()

    _bindings()
    _rule_dependencies()
    hedron_compile_commands_setup()

    maybe(
        http_file,
        name = "bazel_toolchains_rbe_gen_config_linux_amd64",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/releases/download/v5.1.2/rbe_configs_gen_linux_amd64",
            "https://github.com/bazelbuild/bazel-toolchains/releases/download/v5.1.2/rbe_configs_gen_linux_amd64",
        ],
        sha256 = "1206e8a79b41cb22524f73afa4f4ee648478f46ef6990d78e7cc953665a1db89",
        executable = True,
    )

def _remote_jdk_repository(name, version, os, cpu, sha256 = None):
    jdk = version.split(".")[0]
    jdk_cpu = {
        "x86_64": "x64",
        "arm64": "aarch64",
    }.get(cpu, cpu)
    jdk_os = {
        "macos": "macosx",
        "windows": "win",
    }.get(os, os)

    basename = "zulu{version}-{os}_{cpu}".format(
        version = version,
        jdk = jdk,
        os = jdk_os,
        cpu = jdk_cpu,
    )

    remote_java_repository(
        name = name,
        target_compatible_with = [
            c.format(os = os, cpu = cpu)
            for c in [
                "@platforms//os:{os}",
                "@platforms//cpu:{cpu}",
            ]
        ],
        sha256 = sha256,
        strip_prefix = basename,
        urls = [
            url.format(basename = basename)
            for url in [
                "https://mirror.bazel.build/cdn.azul.com/zulu/bin/{basename}.tar.gz",
                "https://cdn.azul.com/zulu/bin/{basename}.tar.gz",
            ]
        ],
        version = jdk,
    )

def remote_jdk19_repos():
    """Imports OpenJDK 19 repositories."""
    maybe(
        _remote_jdk_repository,
        name = "remotejdk19_linux",
        os = "linux",
        cpu = "x86_64",
        version = "19.32.13-ca-jdk19.0.2",
        sha256 = "4a994aded1d9b35258d543a59d4963d2687a1094a818b79a21f00273fbbc5bca",
    )

    maybe(
        _remote_jdk_repository,
        name = "remotejdk19_macos",
        os = "macos",
        cpu = "x86_64",
        version = "19.32.13-ca-jdk19.0.2",
    )

    maybe(
        _remote_jdk_repository,
        name = "remotejdk19_macos_aarch64",
        os = "macos",
        cpu = "aarch64",
        version = "19.32.13-ca-jdk19.0.2",
    )

    maybe(
        _remote_jdk_repository,
        name = "remotejdk19_win",
        os = "windows",
        cpu = "x86_64",
        version = "19.32.13-ca-jdk19.0.2",
    )
