load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", _go_repository = "go_repository")
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
load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")
load("@io_kythe//:setup.bzl", "github_archive")
load("@io_kythe//kythe/cxx/extractor:toolchain.bzl", cxx_extractor_register_toolchains = "register_toolchains")
load("@io_kythe//third_party/bazel:bazel_repository_files.bzl", "bazel_repository_files")
load("@io_kythe//tools/build_rules/lexyacc:lexyacc.bzl", "lexyacc_configure")
load("@llvm-raw//utils/bazel:configure.bzl", "llvm_configure")
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
load(
    "@rules_java//java:repositories.bzl",
    "remote_jdk17_repos",
    "remote_jdk20_repos",
    "rules_java_dependencies",
)
load("@rules_java//toolchains:remote_java_repository.bzl", "remote_java_repository")
load("@rules_jvm_external//:defs.bzl", "maven_install")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@rules_python//python:repositories.bzl", "py_repositories")

# Defined here rather than in //tools:build_rules/shims.bzl because that file
# loads @io_bazel_rules_go//go:def.bzl, which transitively loads
# @io_bazel_rules_nogo. That repository is created by go_rules_dependencies()
# below, so loading it from here would use it before it is defined.
def go_repository(name, **kwargs):
    """Macro wrapping the Gazelle go_repository rule.

    This conditionally defines the repository if it hasn't already been.

    Args:
      name: the name of the repository to define.
      **kwargs: arguments forwarded to the underlying go_repository rule.
    """
    if name in native.existing_rules():
        return
    _go_repository(name = name, **kwargs)

def _rule_dependencies():
    go_rules_dependencies()
    go_register_toolchains(version = "1.25.1")
    gazelle_dependencies()
    rules_java_dependencies()

    # Specifically define and register only Java toolchains we intend to support.
    remote_jdk17_repos()
    remote_jdk19_repos()
    remote_jdk20_repos()
    remote_jdk21_repos()
    native.register_toolchains("@local_jdk//:runtime_toolchain_definition")
    for version in ("11", "17", "19", "20", "21"):
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
        sha256 = "f3a3082a23b37c293a4fcd1053147b371f2ff91fa7ea1b2a52e335676bac82dc",  # 2025-08-02
        strip_prefix = "libffi-3.5.2",
        urls = ["https://github.com/libffi/libffi/releases/download/v3.5.2/libffi-3.5.2.tar.gz"],
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
            "@io_kythe//third_party:souffle_atomic.patch",
        ],
    )

    maybe(
        http_archive,
        name = "net_zlib",
        build_file = "@io_kythe//third_party:zlib.BUILD",
        integrity = "sha256-OO+WuN/lENQnB9nHgYd5FHklQRM+GHCEFGO/pz+IPjI=",
        strip_prefix = "zlib-1.3.1",
        urls = [
            "https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.xz",
            "https://zlib.net/zlib-1.3.1.tar.xz",
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
        patches = [
            "@io_kythe//third_party:rapidjson_assignment.patch",
        ],
    )

    # Abseil LTS release. Must stay compatible with @com_google_protobuf, which
    # references absl/utility:if_constexpr (removed after LTS 20250127).
    maybe(
        http_archive,
        name = "com_google_absl",
        sha256 = "b396401fd29e2e679cace77867481d388c807671dc2acc602a0259eeb79b7811",
        strip_prefix = "abseil-cpp-20250127.1",
        urls = ["https://github.com/abseil/abseil-cpp/releases/download/20250127.1/abseil-cpp-20250127.1.tar.gz"],
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
        commit = "793247783c7d9e6322c2b40f85ceb775a7f29f49",
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
        sum = "h1:/hemPrYIhOhy8zYrNj+069zDB68us2sMGsfkFJO0iZs=",
        version = "v0.0.0-20190523083050-ea95bdfd59fc",
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
        name = "com_github_bazelbuild_bazel_gazelle",
        importpath = "github.com/bazelbuild/bazel-gazelle",
        sum = "h1:WnJGYk1bMIjw8FCYA/UxKBK/Y6hUnOItrtR+vjFIIKo=",
        version = "v0.33.0",
    )
    go_repository(
        name = "com_github_bazelbuild_buildtools",
        importpath = "github.com/bazelbuild/buildtools",
        sum = "h1:bXeNqRn5Rp0ofg26u7n+NlbiRusRQZ6RiNfZD9mcH7A=",
        version = "v0.0.0-20231011133658-72c8ba35684c",
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
        name = "com_github_bmatcuk_doublestar_v4",
        importpath = "github.com/bmatcuk/doublestar/v4",
        sum = "h1:X8jg9rRZmJd4yRy7ZeNDRnM+T3ZfHv15JiBJ/avrEXE=",
        version = "v4.9.1",
    )

    go_repository(
        name = "com_github_burntsushi_toml",
        importpath = "github.com/BurntSushi/toml",
        sum = "h1:WXkYYl6Yr3qBf1K79EBnL4mak0OimBfB0XUf9Vl28OQ=",
        version = "v0.3.1",
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
        name = "com_github_cncf_xds_go",
        importpath = "github.com/cncf/xds/go",
        sum = "h1:aBangftG7EVZoUb69Os8IaYg++6uMOdKK83QtkkvJik=",
        version = "v0.0.0-20260202195803-dba9d589def2",
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
        sum = "h1:U9qPSI2PIWSS1VwoXQT9A3Wy9MM3WgvqSxFWenqJduM=",
        version = "v1.1.2-0.20180830191138-d8f796af33cc",
    )

    go_repository(
        name = "com_github_envoyproxy_go_control_plane",
        importpath = "github.com/envoyproxy/go-control-plane",
        sum = "h1:hbG2kr4RuFj222B6+7T83thSPqLjwBIfQawTkC++2HA=",
        version = "v0.14.0",
    )
    go_repository(
        name = "com_github_envoyproxy_go_control_plane_envoy",
        importpath = "github.com/envoyproxy/go-control-plane/envoy",
        sum = "h1:u3riX6BoYRfF4Dr7dwSOroNfdSbEPe9Yyl09/B6wBrQ=",
        version = "v1.37.0",
    )
    go_repository(
        name = "com_github_envoyproxy_go_control_plane_ratelimit",
        importpath = "github.com/envoyproxy/go-control-plane/ratelimit",
        sum = "h1:/G9QYbddjL25KvtKTv3an9lx6VBE2cnb8wp1vEGNYGI=",
        version = "v0.1.0",
    )
    go_repository(
        name = "com_github_envoyproxy_protoc_gen_validate",
        importpath = "github.com/envoyproxy/protoc-gen-validate",
        sum = "h1:MVQghNeW+LZcmXe7SY1V36Z+WFMDjpqGAGacLe2T0ds=",
        version = "v1.3.3",
    )
    go_repository(
        name = "com_github_felixge_httpsnoop",
        importpath = "github.com/felixge/httpsnoop",
        sum = "h1:NFTV2Zj1bL4mc9sqWACXbQFVBBg2W3GPvqp8/ESS2Wg=",
        version = "v1.0.4",
    )

    go_repository(
        name = "com_github_fsnotify_fsnotify",
        importpath = "github.com/fsnotify/fsnotify",
        sum = "h1:n+5WquG0fcWoWp6xPWfHdbskMCQaFnG6PfBrh1Ky4HY=",
        version = "v1.6.0",
    )
    go_repository(
        name = "com_github_go_jose_go_jose_v4",
        importpath = "github.com/go-jose/go-jose/v4",
        sum = "h1:moDMcTHmvE6Groj34emNPLs/qtYXRVcd6S7NHbHz3kA=",
        version = "v4.1.4",
    )
    go_repository(
        name = "com_github_go_logr_logr",
        importpath = "github.com/go-logr/logr",
        sum = "h1:CjnDlHq8ikf6E492q6eKboGOC0T8CDaOvkHCIg8idEI=",
        version = "v1.4.3",
    )
    go_repository(
        name = "com_github_go_logr_stdr",
        importpath = "github.com/go-logr/stdr",
        sum = "h1:hSWxHoqTgW2S2qGc0LTAI563KZ5YKYRhT3MFKZMbjag=",
        version = "v1.2.2",
    )

    go_repository(
        name = "com_github_golang_glog",
        importpath = "github.com/golang/glog",
        sum = "h1:DrW6hGnjIhtvhOIiAKT6Psh/Kd/ldepEa81DKeiRJ5I=",
        version = "v1.2.5",
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
        sum = "h1:ErTB+efbowRARo13NNdxyJji2egdxLGQhRaY+DUumQc=",
        version = "v1.6.0",
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
        sum = "h1:wk8382ETsv4JYUZwIsn6YpYiWiBsYLSJiTsyBybVuN8=",
        version = "v0.7.0",
    )
    go_repository(
        name = "com_github_google_go_pkcs11",
        importpath = "github.com/google/go-pkcs11",
        sum = "h1:PVRnTgtArZ3QQqTGtbtjtnIkzl2iY2kt24yqbrf7td8=",
        version = "v0.3.0",
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
        name = "com_github_google_s2a_go",
        importpath = "github.com/google/s2a-go",
        sum = "h1:LGD7gtMgezd8a/Xak7mEWL0PjoTQFvpRudN895yqKW0=",
        version = "v0.1.9",
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
        sum = "h1:vAe81Msw+8tKUxi2Dqh/NZMz7475yUvmRIkXr4oN2ao=",
        version = "v0.3.11",
    )

    go_repository(
        name = "com_github_googleapis_gax_go_v2",
        build_file_proto_mode = "disable",
        importpath = "github.com/googleapis/gax-go/v2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:RksgfBpxqff0EZkDWYuz9q/uWsTVz+kf43LsZ1J6SMc=",
        version = "v2.17.0",
    )
    go_repository(
        name = "com_github_googlecloudplatform_opentelemetry_operations_go_detectors_gcp",
        importpath = "github.com/GoogleCloudPlatform/opentelemetry-operations-go/detectors/gcp",
        sum = "h1:l7+6kwRMJNwdCvYdDl7Eax+wzEYHSnNY7zrrfbhDdTA=",
        version = "v1.33.0",
    )
    go_repository(
        name = "com_github_googlecloudplatform_opentelemetry_operations_go_exporter_metric",
        importpath = "github.com/GoogleCloudPlatform/opentelemetry-operations-go/exporter/metric",
        sum = "h1:owcC2UnmsZycprQ5RfRgjydWhuoxg71LUfyiQdijZuM=",
        version = "v0.53.0",
    )
    go_repository(
        name = "com_github_googlecloudplatform_opentelemetry_operations_go_internal_cloudmock",
        importpath = "github.com/GoogleCloudPlatform/opentelemetry-operations-go/internal/cloudmock",
        sum = "h1:4LP6hvB4I5ouTbGgWtixJhgED6xdf67twf9PoY96Tbg=",
        version = "v0.53.0",
    )
    go_repository(
        name = "com_github_googlecloudplatform_opentelemetry_operations_go_internal_resourcemapping",
        importpath = "github.com/GoogleCloudPlatform/opentelemetry-operations-go/internal/resourcemapping",
        sum = "h1:Ron4zCA/yk6U7WOBXhTJcDpsUBG9npumK6xw2auFltQ=",
        version = "v0.53.0",
    )
    go_repository(
        name = "com_github_gorilla_websocket",
        importpath = "github.com/gorilla/websocket",
        sum = "h1:q7AeDBpnBk8AogcD4DSag/Ukw/KV+YhzLj2bP5HvKCM=",
        version = "v1.4.1",
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
        name = "com_github_hpcloud_tail",
        importpath = "github.com/hpcloud/tail",
        sum = "h1:nfCOvKYfkgYP8hkirhJocXT2+zOD8yUNjXaWfTlyFKI=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_iancoleman_strcase",
        importpath = "github.com/iancoleman/strcase",
        sum = "h1:nTXanmYxhfFAMjZL34Ov6gkzEsSJZ5DbhxWjvSASxEI=",
        version = "v0.3.0",
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
        name = "com_github_lyft_protoc_gen_star_v2",
        importpath = "github.com/lyft/protoc-gen-star/v2",
        sum = "h1:JDlNKttNIRd68AAIychs0AqEpO8/I/WYi01OQ7Raw6Q=",
        version = "v2.0.4",
    )
    go_repository(
        name = "com_github_microsoft_go_winio",
        importpath = "github.com/Microsoft/go-winio",
        sum = "h1:F2VQgta7ecxGYO8k3ZZz3RS8fVIXVxONVUPlNERoyfY=",
        version = "v0.6.2",
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
        sum = "h1:Jamvg5psRIccs7FGNTlIRMkT8wgtp5eCXdBlqhYGL6U=",
        version = "v1.0.1-0.20181226105442-5d4384ee4fb2",
    )
    go_repository(
        name = "com_github_prometheus_client_model",
        importpath = "github.com/prometheus/client_model",
        sum = "h1:oBsgwpGs7iVziMvrGhE53c/GrLUsZdHnqNwqPLxwZyk=",
        version = "v0.6.2",
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
        sum = "h1:UQB4HGPB6osV0SQTLymcB4TgvyWu6ZyliaW0tI/otEQ=",
        version = "v1.14.1",
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
        name = "com_github_spf13_afero",
        importpath = "github.com/spf13/afero",
        sum = "h1:b/YBCLWAJdFWJTN9cLhiXXcD7mzKn9Dm86dNnfyQw1I=",
        version = "v1.15.0",
    )
    go_repository(
        name = "com_github_spiffe_go_spiffe_v2",
        importpath = "github.com/spiffe/go-spiffe/v2",
        sum = "h1:uXe1MflJoHw58wAUvxVlcM7WpKtijWG7I1UidcGh6g4=",
        version = "v2.7.0",
    )

    go_repository(
        name = "com_github_stretchr_objx",
        importpath = "github.com/stretchr/objx",
        sum = "h1:4G4v2dO3VZwixGIRoQ5Lfboy6nUhCyYzaqnIAPPhYs4=",
        version = "v0.1.0",
    )
    go_repository(
        name = "com_github_stretchr_testify",
        importpath = "github.com/stretchr/testify",
        sum = "h1:7s2iGBzp5EwR7/aIZr8ao5+dra3wiQyKjjFuvgVKu7U=",
        version = "v1.11.1",
    )
    go_repository(
        name = "com_github_syndtr_goleveldb",
        importpath = "github.com/syndtr/goleveldb",
        sum = "h1:fBdIW9lB4Iz0n9khmH8w27SJ3QEJ7+IgjPEwGSZiFdE=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_zeebo_errs",
        importpath = "github.com/zeebo/errs",
        sum = "h1:XNdoD/RRMKP7HD0UhJnIzUy74ISdGGxURlYG8HSWSfM=",
        version = "v1.4.0",
    )

    go_repository(
        name = "com_google_cloud_go",
        importpath = "cloud.google.com/go",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:2NAUJwPR47q+E35uaJeYoNhuNEM9kM8SjgRgdeOJUSE=",
        version = "v0.123.0",
    )
    go_repository(
        name = "com_google_cloud_go_accessapproval",
        importpath = "cloud.google.com/go/accessapproval",
        sum = "h1:gq8OS+rQWgGRo91D2qztN+ion6AZ2T1CxBIu0ifCmVo=",
        version = "v1.8.8",
    )
    go_repository(
        name = "com_google_cloud_go_accesscontextmanager",
        importpath = "cloud.google.com/go/accesscontextmanager",
        sum = "h1:aKIfg7Jyc73pe8bzx0zypNdS5gfFdSvFvB8YNA9k2kA=",
        version = "v1.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_aiplatform",
        importpath = "cloud.google.com/go/aiplatform",
        sum = "h1:TCrSLci+NFEAx0PZMv8btGe5j68RivArmDJbBLIc/3o=",
        version = "v1.114.0",
    )
    go_repository(
        name = "com_google_cloud_go_analytics",
        importpath = "cloud.google.com/go/analytics",
        sum = "h1:souLxu9tQHzF+0NDpKoIw4pl2WQ9K2JfkdPPs36BfXw=",
        version = "v0.30.1",
    )
    go_repository(
        name = "com_google_cloud_go_apigateway",
        importpath = "cloud.google.com/go/apigateway",
        sum = "h1:ehKUTy+QFsb3n07fEi18S2dpDDjCV4UlRyrbwfZV3Zk=",
        version = "v1.7.7",
    )
    go_repository(
        name = "com_google_cloud_go_apigeeconnect",
        importpath = "cloud.google.com/go/apigeeconnect",
        sum = "h1:S6s2zojwMymx0fyZYKm0eK1TdDxrriIBAlNVvRAOzug=",
        version = "v1.7.7",
    )
    go_repository(
        name = "com_google_cloud_go_apigeeregistry",
        importpath = "cloud.google.com/go/apigeeregistry",
        sum = "h1:QziFVsuPU2lhy40Ht9uWEyciV23SH9GETWiwcu3qzdg=",
        version = "v0.10.0",
    )

    go_repository(
        name = "com_google_cloud_go_appengine",
        importpath = "cloud.google.com/go/appengine",
        sum = "h1:IxGz6j5xv0nTJX285wu95Vn6KEi2CeV9vbyRgCSEAoU=",
        version = "v1.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_area120",
        importpath = "cloud.google.com/go/area120",
        sum = "h1:BbpzLwaIXVPorrrzTH+ni7P5mLemmPPfSZ7o39k7zQc=",
        version = "v0.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_artifactregistry",
        importpath = "cloud.google.com/go/artifactregistry",
        sum = "h1:DaOHWeURq93K27/6Sa2fy3rJoftrVXKeT3tonM4fxtI=",
        version = "v1.19.0",
    )
    go_repository(
        name = "com_google_cloud_go_asset",
        importpath = "cloud.google.com/go/asset",
        sum = "h1:81Ru5hjHfiGtk+u/Ix69eaWieKpvm7Ce7UHtcZhOLbk=",
        version = "v1.22.0",
    )
    go_repository(
        name = "com_google_cloud_go_assuredworkloads",
        importpath = "cloud.google.com/go/assuredworkloads",
        sum = "h1:NQXyyGLksPmiapE1Oc64a3cMwYIBAoDBg6cWR+B3eaY=",
        version = "v1.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_auth",
        importpath = "cloud.google.com/go/auth",
        sum = "h1:+Nbt5Ev0xEqxlNjd6c+yYUeosQ5TtEUaNcN/3FozlaM=",
        version = "v0.18.2",
    )
    go_repository(
        name = "com_google_cloud_go_auth_oauth2adapt",
        importpath = "cloud.google.com/go/auth/oauth2adapt",
        sum = "h1:keo8NaayQZ6wimpNSmW5OPc283g65QNIiLpZnkHRbnc=",
        version = "v0.2.8",
    )
    go_repository(
        name = "com_google_cloud_go_automl",
        importpath = "cloud.google.com/go/automl",
        sum = "h1:YRwLbsBv4yApX64pkrdyy4emhWE6lHEnljX4b1aTQC4=",
        version = "v1.15.0",
    )
    go_repository(
        name = "com_google_cloud_go_baremetalsolution",
        importpath = "cloud.google.com/go/baremetalsolution",
        sum = "h1:g67fjVdrNCHZl8jDWdZvo+6zGTTMMuvNWO7HSgG8lnI=",
        version = "v1.4.0",
    )
    go_repository(
        name = "com_google_cloud_go_batch",
        importpath = "cloud.google.com/go/batch",
        sum = "h1:r5DEMPNXZk1as36Le3DaNQTRhhnR+E95a99SFxwF52o=",
        version = "v1.14.0",
    )
    go_repository(
        name = "com_google_cloud_go_beyondcorp",
        importpath = "cloud.google.com/go/beyondcorp",
        sum = "h1:mre997ya7QHFWSU+O5cT/FhBKTMy6Riqf1EXFxN46zw=",
        version = "v1.2.0",
    )

    go_repository(
        name = "com_google_cloud_go_bigquery",
        importpath = "cloud.google.com/go/bigquery",
        sum = "h1:D/yLju+3Ens2IXx7ou1DJ62juBm+/coBInn4VVOg5Cw=",
        version = "v1.72.0",
    )
    go_repository(
        name = "com_google_cloud_go_bigtable",
        importpath = "cloud.google.com/go/bigtable",
        sum = "h1:99KOWShm/MUyuIbXBeVscdWJFV7GdgiYwFUrB5Iu4BI=",
        version = "v1.41.0",
    )
    go_repository(
        name = "com_google_cloud_go_billing",
        importpath = "cloud.google.com/go/billing",
        sum = "h1:nbQjTXkpgB/E4XnYZQwcZnR63QFsbFwJ9DGsNg61Ghg=",
        version = "v1.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_binaryauthorization",
        importpath = "cloud.google.com/go/binaryauthorization",
        sum = "h1:YYK0BwiZv9uA6z+Ict908AykX4OBfDECMTE476OnS3A=",
        version = "v1.10.0",
    )
    go_repository(
        name = "com_google_cloud_go_certificatemanager",
        importpath = "cloud.google.com/go/certificatemanager",
        sum = "h1:v5X8X+THKrS9OFZb6k0GRDP1WQxLXTdMko7OInBliw4=",
        version = "v1.9.6",
    )
    go_repository(
        name = "com_google_cloud_go_channel",
        importpath = "cloud.google.com/go/channel",
        sum = "h1:ThoAmHBd9WkX2SSuF6n6uEOvbBNoTuhBT7Rk6bFS5ho=",
        version = "v1.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_cloudbuild",
        importpath = "cloud.google.com/go/cloudbuild",
        sum = "h1:Fkg+iJdN7bfICZJzLr/XV+k9aVxXS/hakIlhjDIRIDw=",
        version = "v1.25.0",
    )
    go_repository(
        name = "com_google_cloud_go_clouddms",
        importpath = "cloud.google.com/go/clouddms",
        sum = "h1:YWsmRXTyK6Ba0hm4qTBak5g1oLhryuM8rSBxHWC8iq4=",
        version = "v1.8.8",
    )
    go_repository(
        name = "com_google_cloud_go_cloudtasks",
        importpath = "cloud.google.com/go/cloudtasks",
        sum = "h1:H2v8GEolNtMFfYzUpZBaZbydqU7drpyo99GtAgA+m4I=",
        version = "v1.13.7",
    )
    go_repository(
        name = "com_google_cloud_go_compute",
        importpath = "cloud.google.com/go/compute",
        sum = "h1:4CKmnpO+40z44bKG5bdcKxQ7ocNpRtOc9SCLLUzze1w=",
        version = "v1.54.0",
    )
    go_repository(
        name = "com_google_cloud_go_compute_metadata",
        importpath = "cloud.google.com/go/compute/metadata",
        sum = "h1:pDUj4QMoPejqq20dK0Pg2N4yG9zIkYGdBtwLoEkH9Zs=",
        version = "v0.9.0",
    )
    go_repository(
        name = "com_google_cloud_go_contactcenterinsights",
        importpath = "cloud.google.com/go/contactcenterinsights",
        sum = "h1:wA4j99BhsoeYlLx6xEIqrNN1aOTtUme0wimHZegg80s=",
        version = "v1.17.4",
    )
    go_repository(
        name = "com_google_cloud_go_container",
        importpath = "cloud.google.com/go/container",
        sum = "h1:i1No5obpPxlIFLGHdUF6h2YjRR1qN9t/ZkA8KA5B//o=",
        version = "v1.45.0",
    )
    go_repository(
        name = "com_google_cloud_go_containeranalysis",
        importpath = "cloud.google.com/go/containeranalysis",
        sum = "h1:OW2dlMPtR5VnjQGyAP+uJlZahc1l+JFxFlH/J3+l7gw=",
        version = "v0.14.2",
    )
    go_repository(
        name = "com_google_cloud_go_datacatalog",
        importpath = "cloud.google.com/go/datacatalog",
        sum = "h1:bCRKA8uSQN8wGW3Tw0gwko4E9a64GRmbW1nCblhgC2k=",
        version = "v1.26.1",
    )
    go_repository(
        name = "com_google_cloud_go_dataflow",
        importpath = "cloud.google.com/go/dataflow",
        sum = "h1:Z+UYlGrE+IoB+5IAN4/qWdPKO0IpIK9bs2Dy40HK6lg=",
        version = "v0.11.1",
    )
    go_repository(
        name = "com_google_cloud_go_dataform",
        importpath = "cloud.google.com/go/dataform",
        sum = "h1:yf6Up6m1FUt+YB5CBgNtIZfz2OzjuNBdzZWV3SLSVNE=",
        version = "v0.12.1",
    )
    go_repository(
        name = "com_google_cloud_go_datafusion",
        importpath = "cloud.google.com/go/datafusion",
        sum = "h1:tLCV+xYuOrSjdrRTkc9Cqsb5mBSQEsNfFmuTNYl5/rA=",
        version = "v1.8.7",
    )
    go_repository(
        name = "com_google_cloud_go_datalabeling",
        importpath = "cloud.google.com/go/datalabeling",
        sum = "h1:wwoct7mw38s75XvEmLoItQ2TY0RFsGiRDb0iNbXUcX4=",
        version = "v0.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_dataplex",
        importpath = "cloud.google.com/go/dataplex",
        sum = "h1:rROI3iqMVI9nXT701ULoFRETQVAOAPC3mPSWFDxXFl0=",
        version = "v1.28.0",
    )
    go_repository(
        name = "com_google_cloud_go_dataproc_v2",
        importpath = "cloud.google.com/go/dataproc/v2",
        sum = "h1:I/Yux/d8uaxf3W+d59kolGTOc52+VZaL6RzJw7oDOeg=",
        version = "v2.15.0",
    )

    go_repository(
        name = "com_google_cloud_go_dataqna",
        importpath = "cloud.google.com/go/dataqna",
        sum = "h1:3FREvU+sjaEHSjlKrKF6KjUmafdOvM8CbZ897rttxNs=",
        version = "v0.9.8",
    )

    go_repository(
        name = "com_google_cloud_go_datastore",
        importpath = "cloud.google.com/go/datastore",
        sum = "h1:dUrYq47ysCA4nM7u8kRT0WnbfXc6TzX49cP3TCwIiA0=",
        version = "v1.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_datastream",
        importpath = "cloud.google.com/go/datastream",
        sum = "h1:7PKeDpksi8nbOR4gspmNokzsr0q/uRzDIt20bR3BtRs=",
        version = "v1.15.1",
    )
    go_repository(
        name = "com_google_cloud_go_deploy",
        importpath = "cloud.google.com/go/deploy",
        sum = "h1:QU8gLXsXDRqLyEWNrI6zJiVzuuOBX/WpMi4p0oexV+c=",
        version = "v1.27.3",
    )
    go_repository(
        name = "com_google_cloud_go_dialogflow",
        importpath = "cloud.google.com/go/dialogflow",
        sum = "h1:YtSrYLd6CpRS1NZ67r4Drvm1gafUEGDm/8vVBRYEv+I=",
        version = "v1.74.0",
    )
    go_repository(
        name = "com_google_cloud_go_dlp",
        importpath = "cloud.google.com/go/dlp",
        sum = "h1:+aMQYODOxCCZHpdzKvv/rIc9CbKd6XVmjVBRjaF8UvQ=",
        version = "v1.28.0",
    )
    go_repository(
        name = "com_google_cloud_go_documentai",
        importpath = "cloud.google.com/go/documentai",
        sum = "h1:ngPNmVUb3xxj6/j7ruK4vmOMm+XTJO0Vi7Jkm0R3AdU=",
        version = "v1.39.0",
    )
    go_repository(
        name = "com_google_cloud_go_domains",
        importpath = "cloud.google.com/go/domains",
        sum = "h1:G3kUq0vKBMhyOj5GqAfEYbVuez05U+ENHZUAtrEp/pI=",
        version = "v0.10.7",
    )
    go_repository(
        name = "com_google_cloud_go_edgecontainer",
        importpath = "cloud.google.com/go/edgecontainer",
        sum = "h1:6KTQo6Qf0iEtfPVotlG7orazEO1I93Ham0PMlkHYpdQ=",
        version = "v1.4.4",
    )
    go_repository(
        name = "com_google_cloud_go_errorreporting",
        importpath = "cloud.google.com/go/errorreporting",
        sum = "h1:uLcasn2hKpj6iSPvHrzRjkJcaNVaKx8yKQcP3VTS6aI=",
        version = "v0.4.0",
    )
    go_repository(
        name = "com_google_cloud_go_essentialcontacts",
        importpath = "cloud.google.com/go/essentialcontacts",
        sum = "h1:v9sO4IHFuwplaOuDnEXZFtfOrjw2bi11TSIVp5PnAU4=",
        version = "v1.7.7",
    )
    go_repository(
        name = "com_google_cloud_go_eventarc",
        importpath = "cloud.google.com/go/eventarc",
        sum = "h1:8WWG1/ogInYur1NQjML6EMHQ0ZBzAdMDGlUVpLD56cI=",
        version = "v1.18.0",
    )
    go_repository(
        name = "com_google_cloud_go_filestore",
        importpath = "cloud.google.com/go/filestore",
        sum = "h1:3KZifUVTqGhNNv6MLeONYth1HjlVM4vDhaH+xrdPljU=",
        version = "v1.10.3",
    )
    go_repository(
        name = "com_google_cloud_go_firestore",
        importpath = "cloud.google.com/go/firestore",
        sum = "h1:BhopUsx7kh6NFx77ccRsHhrtkbJUmDAxNY3uapWdjcM=",
        version = "v1.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_functions",
        importpath = "cloud.google.com/go/functions",
        sum = "h1:7LcOD18euIVGRUPaeCmgO6vfWSLNIsi6STWRQcdANG8=",
        version = "v1.19.7",
    )

    go_repository(
        name = "com_google_cloud_go_gkebackup",
        importpath = "cloud.google.com/go/gkebackup",
        sum = "h1:gUgI3lZJYALZsHXE7YJOKI8bMpoAX/tF6jnNugvzT1g=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_gkeconnect",
        importpath = "cloud.google.com/go/gkeconnect",
        sum = "h1:EFql3zRaFw74yATt5lf+mcPDqPZ4EeLvoIJ+0NaEkag=",
        version = "v0.12.5",
    )
    go_repository(
        name = "com_google_cloud_go_gkehub",
        importpath = "cloud.google.com/go/gkehub",
        sum = "h1:Jk5pAXG54FlQzTRXhuKyym/NzOgS8oWRs0XNatZYDf4=",
        version = "v0.16.0",
    )
    go_repository(
        name = "com_google_cloud_go_gkemulticloud",
        importpath = "cloud.google.com/go/gkemulticloud",
        sum = "h1:m0FX9o7t7xVmSZhqzm/m8nEZn8LnC5Kh60Wg4Yx1lyQ=",
        version = "v1.6.0",
    )
    go_repository(
        name = "com_google_cloud_go_gsuiteaddons",
        importpath = "cloud.google.com/go/gsuiteaddons",
        sum = "h1:Dayrv57XW8kZIvmQjAc89Tp7Kr3O9Am/hf6pXkTjYFY=",
        version = "v1.7.8",
    )
    go_repository(
        name = "com_google_cloud_go_iam",
        importpath = "cloud.google.com/go/iam",
        sum = "h1:+vMINPiDF2ognBJ97ABAYYwRgsaqxPbQDlMnbHMjolc=",
        version = "v1.5.3",
    )
    go_repository(
        name = "com_google_cloud_go_iap",
        importpath = "cloud.google.com/go/iap",
        sum = "h1:Nheb77nO0/pECm/thoE3wHVAbkQSI+G8KBWviqBepiA=",
        version = "v1.11.3",
    )
    go_repository(
        name = "com_google_cloud_go_ids",
        importpath = "cloud.google.com/go/ids",
        sum = "h1:V0pSk+KKW+5/AVpeQMhM9D1VI7aMZkayj5jddNETJos=",
        version = "v1.5.7",
    )
    go_repository(
        name = "com_google_cloud_go_iot",
        importpath = "cloud.google.com/go/iot",
        sum = "h1:PDUtxCzlFwFHODEFAgaGJy/Zv4tdvLbZ+lvZ1mKQXE4=",
        version = "v1.8.7",
    )
    go_repository(
        name = "com_google_cloud_go_kms",
        importpath = "cloud.google.com/go/kms",
        sum = "h1:gVqvGGUmz0nYCmtoxWmdc1wli2L1apgP8U4fghPGSbQ=",
        version = "v1.25.0",
    )
    go_repository(
        name = "com_google_cloud_go_language",
        importpath = "cloud.google.com/go/language",
        sum = "h1:/0Fbd3/T4oNmpPqIq5/hrWdHc/eoYGtVH5lDNkuHH3k=",
        version = "v1.14.6",
    )
    go_repository(
        name = "com_google_cloud_go_lifesciences",
        importpath = "cloud.google.com/go/lifesciences",
        sum = "h1:MO5aBahcYv7JeuCpHbg/11h7KL/BYt1+PpgHhleLDbI=",
        version = "v0.10.7",
    )
    go_repository(
        name = "com_google_cloud_go_logging",
        importpath = "cloud.google.com/go/logging",
        sum = "h1:O7LvmO0kGLaHY/gq8cV7T0dyp6zJhYAOtZPX4TF3QtY=",
        version = "v1.13.1",
    )
    go_repository(
        name = "com_google_cloud_go_longrunning",
        importpath = "cloud.google.com/go/longrunning",
        sum = "h1:LiKK77J3bx5gDLi4SMViHixjD2ohlkwBi+mKA7EhfW8=",
        version = "v0.8.0",
    )
    go_repository(
        name = "com_google_cloud_go_managedidentities",
        importpath = "cloud.google.com/go/managedidentities",
        sum = "h1:vC/q7D+97PZfb0UNf7r/+/clHauuaf1PqWwP7neuaeg=",
        version = "v1.7.7",
    )
    go_repository(
        name = "com_google_cloud_go_maps",
        importpath = "cloud.google.com/go/maps",
        sum = "h1:tcdo9oB3Ap4N9JJJFOhxRFldKUok4Mesd3ta7Rm79r0=",
        version = "v1.26.0",
    )
    go_repository(
        name = "com_google_cloud_go_mediatranslation",
        importpath = "cloud.google.com/go/mediatranslation",
        sum = "h1:JXbjms+JxgaWkj/YuaQm1OeCzuF+IZCDV17uUcZgFOU=",
        version = "v0.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_memcache",
        importpath = "cloud.google.com/go/memcache",
        sum = "h1:ZDIfIMZsKKPzwdbvTMOL1il0shX24J7B9DC+sEt4Yj4=",
        version = "v1.11.7",
    )
    go_repository(
        name = "com_google_cloud_go_metastore",
        importpath = "cloud.google.com/go/metastore",
        sum = "h1:nfyUDD9AeKIs6btY5buQ1No0OVco20WpX9wIruL8UOA=",
        version = "v1.14.8",
    )
    go_repository(
        name = "com_google_cloud_go_monitoring",
        importpath = "cloud.google.com/go/monitoring",
        sum = "h1:dde+gMNc0UhPZD1Azu6at2e79bfdztVDS5lvhOdsgaE=",
        version = "v1.24.3",
    )
    go_repository(
        name = "com_google_cloud_go_networkconnectivity",
        importpath = "cloud.google.com/go/networkconnectivity",
        sum = "h1:A0uRcZJdq7F6LYWcc2NIea3h0i7p6kY1/CyLavOTG0I=",
        version = "v1.20.0",
    )
    go_repository(
        name = "com_google_cloud_go_networkmanagement",
        importpath = "cloud.google.com/go/networkmanagement",
        sum = "h1:041d0RqmwP/H7AWkF/AoAwFvuCkz8shUMA4EoQt0lOA=",
        version = "v1.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_networksecurity",
        importpath = "cloud.google.com/go/networksecurity",
        sum = "h1:+ahtCqEqwHw3a3UIeG21vT817xt9kkDDAO6k9+LCc18=",
        version = "v0.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_notebooks",
        importpath = "cloud.google.com/go/notebooks",
        sum = "h1:g5LTI1LHa/86abDTWd8nrq7/4qq8oFhVx1SmnNpZLVg=",
        version = "v1.12.7",
    )
    go_repository(
        name = "com_google_cloud_go_optimization",
        importpath = "cloud.google.com/go/optimization",
        sum = "h1:dMtxINB6G7wULbdm8nZ/x1NMa579Q+GfJc5gaN8VeDw=",
        version = "v1.7.7",
    )
    go_repository(
        name = "com_google_cloud_go_orchestration",
        importpath = "cloud.google.com/go/orchestration",
        sum = "h1:TVWDiZyvcflLFeTQH2GexHmtJ6iUSjzr0zsSiT338dA=",
        version = "v1.11.10",
    )
    go_repository(
        name = "com_google_cloud_go_orgpolicy",
        importpath = "cloud.google.com/go/orgpolicy",
        sum = "h1:0hq12wxNwcfUMojr5j3EjWECSInIuyYDhkAWXTomRhc=",
        version = "v1.15.1",
    )
    go_repository(
        name = "com_google_cloud_go_osconfig",
        importpath = "cloud.google.com/go/osconfig",
        sum = "h1:QQzK5njfsfO2rdOWYVDyLQktqSq9gKf2ohRYeKUuA10=",
        version = "v1.15.1",
    )
    go_repository(
        name = "com_google_cloud_go_oslogin",
        importpath = "cloud.google.com/go/oslogin",
        sum = "h1:YQ8P/+MLwH0tpENYU9QOgwKQxe8DYfAKxIfm6y+OBtA=",
        version = "v1.14.7",
    )
    go_repository(
        name = "com_google_cloud_go_phishingprotection",
        importpath = "cloud.google.com/go/phishingprotection",
        sum = "h1:ZJqHirY2/H6s+uTq1y1iiVASzm3ZuDiMglT5NXywPBE=",
        version = "v0.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_policytroubleshooter",
        importpath = "cloud.google.com/go/policytroubleshooter",
        sum = "h1:Bbj1EiVh96u9mfO2p+JNoHrvvyC0Ms6zP+vxqQnsaG8=",
        version = "v1.11.7",
    )
    go_repository(
        name = "com_google_cloud_go_privatecatalog",
        importpath = "cloud.google.com/go/privatecatalog",
        sum = "h1:yOdy85WDvSCPxAMixkhs5X0Z96D74kosgOTp7aJEYvU=",
        version = "v0.10.8",
    )

    go_repository(
        name = "com_google_cloud_go_pubsub",
        importpath = "cloud.google.com/go/pubsub",
        sum = "h1:fzbXpPyJnSGvWXF1jabhQeXyxdbCIkXTpjXHy7xviBM=",
        version = "v1.50.1",
    )
    go_repository(
        name = "com_google_cloud_go_pubsub_v2",
        importpath = "cloud.google.com/go/pubsub/v2",
        sum = "h1:0qS6mRJ41gD1lNmM/vdm6bR7DQu6coQcVwD+VPf0Bz0=",
        version = "v2.0.0",
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
        sum = "h1:zHaPdgmV3LmzaUfn9Xiiqp5zE1Y16f0O8XCwERrAs2E=",
        version = "v2.21.0",
    )
    go_repository(
        name = "com_google_cloud_go_recommendationengine",
        importpath = "cloud.google.com/go/recommendationengine",
        sum = "h1:NH89CyKQP8e98kpdKLwV0jXkQGzSEEZia0V867vkoy8=",
        version = "v0.9.7",
    )
    go_repository(
        name = "com_google_cloud_go_recommender",
        importpath = "cloud.google.com/go/recommender",
        sum = "h1:ZVZg4wr1G7yzjIPcYUNSUJAaz9+2o78rmBU4QJgC7kg=",
        version = "v1.13.6",
    )
    go_repository(
        name = "com_google_cloud_go_redis",
        importpath = "cloud.google.com/go/redis",
        sum = "h1:6LI8zSt+vmE3WQ7hE5GsJ13CbJBLV1qUw6B7CY31Wcw=",
        version = "v1.18.3",
    )
    go_repository(
        name = "com_google_cloud_go_resourcemanager",
        importpath = "cloud.google.com/go/resourcemanager",
        sum = "h1:oPZKIdjyVTuag+D4HF7HO0mnSqcqgjcuA18xblwA0V0=",
        version = "v1.10.7",
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
        sum = "h1:S9Rl1sB6Dmdak5y9I2Pwn9hcWA5ubk33Vnevxw+CSrI=",
        version = "v1.25.1",
    )
    go_repository(
        name = "com_google_cloud_go_run",
        importpath = "cloud.google.com/go/run",
        sum = "h1:4cwyNv9SUQEsQOf5/DfPKyMWYSA52p38/o119BgMhO4=",
        version = "v1.15.0",
    )
    go_repository(
        name = "com_google_cloud_go_scheduler",
        importpath = "cloud.google.com/go/scheduler",
        sum = "h1:BoXY2BvBsaRw3ggVMzC9tborZqJBu+NcJcD9PqeC5Kc=",
        version = "v1.11.8",
    )
    go_repository(
        name = "com_google_cloud_go_secretmanager",
        importpath = "cloud.google.com/go/secretmanager",
        sum = "h1:19QT7ZsLJ8FSP1k+4esQvuCD7npMJml6hYzilxVyT+k=",
        version = "v1.16.0",
    )
    go_repository(
        name = "com_google_cloud_go_security",
        importpath = "cloud.google.com/go/security",
        sum = "h1:cF3FkCRRbRC1oXuaGZFl3qU2sdu2gP3iOAHKzL5y04Y=",
        version = "v1.19.2",
    )
    go_repository(
        name = "com_google_cloud_go_securitycenter",
        importpath = "cloud.google.com/go/securitycenter",
        sum = "h1:D9zpeguY4frQU35GBw8+M6Gw79CiuTF9iVs4sFm3FDY=",
        version = "v1.38.1",
    )

    go_repository(
        name = "com_google_cloud_go_servicedirectory",
        importpath = "cloud.google.com/go/servicedirectory",
        sum = "h1:je2yZlVcVFI/TshPXjjF9ZAlWedj0s5EbO2kozJrzBo=",
        version = "v1.12.7",
    )

    go_repository(
        name = "com_google_cloud_go_shell",
        importpath = "cloud.google.com/go/shell",
        sum = "h1:K1C9sh9EuNNhGpyCoqRdeudcU9zmfYTA95bhF5cokK8=",
        version = "v1.8.7",
    )
    go_repository(
        name = "com_google_cloud_go_spanner",
        importpath = "cloud.google.com/go/spanner",
        sum = "h1:M9RGcj/4gJk6yY1lRLOz1Ze+5ufoWhbIiurzXLOOfcw=",
        version = "v1.87.0",
    )
    go_repository(
        name = "com_google_cloud_go_speech",
        importpath = "cloud.google.com/go/speech",
        sum = "h1:ehOzN/IsAhjjAtWg4fI8A3iNtonb1N8yWjofVhSTv+c=",
        version = "v1.29.0",
    )

    go_repository(
        name = "com_google_cloud_go_storage",
        importpath = "cloud.google.com/go/storage",
        sum = "h1:iixmq2Fse2tqxMbWhLWC9HfBj1qdxqAmiK8/eqtsLxI=",
        version = "v1.56.0",
    )
    go_repository(
        name = "com_google_cloud_go_storagetransfer",
        importpath = "cloud.google.com/go/storagetransfer",
        sum = "h1:Sjukr1LtUt7vLTHNvGc2gaAqlXNFeDFRIRmWGrFaJlY=",
        version = "v1.13.1",
    )
    go_repository(
        name = "com_google_cloud_go_talent",
        importpath = "cloud.google.com/go/talent",
        sum = "h1:1kJJ+WCY5LZ1A4rCa32zKh3N2xT3I8koiS63+vV0WC4=",
        version = "v1.8.4",
    )
    go_repository(
        name = "com_google_cloud_go_texttospeech",
        importpath = "cloud.google.com/go/texttospeech",
        sum = "h1:Ra4w+6qmaeb12ozlPBqGw8Jzdge1yfzhvZgcXWdXw30=",
        version = "v1.16.0",
    )
    go_repository(
        name = "com_google_cloud_go_tpu",
        importpath = "cloud.google.com/go/tpu",
        sum = "h1:5DDheA1f7yZ/KUbVT/9lL+Yhgd3IqHDSVVrSqDVkAFY=",
        version = "v1.8.4",
    )
    go_repository(
        name = "com_google_cloud_go_trace",
        importpath = "cloud.google.com/go/trace",
        sum = "h1:kDNDX8JkaAG3R2nq1lIdkb7FCSi1rCmsEtKVsty7p+U=",
        version = "v1.11.7",
    )
    go_repository(
        name = "com_google_cloud_go_translate",
        importpath = "cloud.google.com/go/translate",
        sum = "h1:aSxMbfJ3MVmEdQzu5jGXmPPxCAb1ySsor2yBMCI5MT4=",
        version = "v1.12.7",
    )
    go_repository(
        name = "com_google_cloud_go_video",
        importpath = "cloud.google.com/go/video",
        sum = "h1:Hp+2AeM7b3AagdHcyh2820UTzSbGyqpFJVMu0nHbBcw=",
        version = "v1.27.1",
    )
    go_repository(
        name = "com_google_cloud_go_videointelligence",
        importpath = "cloud.google.com/go/videointelligence",
        sum = "h1:FisUrSZ+y3oLuGdlFQQgZoNTDm7FAfb2hwSTsSqX+9g=",
        version = "v1.12.7",
    )
    go_repository(
        name = "com_google_cloud_go_vision_v2",
        importpath = "cloud.google.com/go/vision/v2",
        sum = "h1:9UtOINPF8p9VACQ6KAyR/ZtkpuBHGmJsprutYupDcN0=",
        version = "v2.9.6",
    )
    go_repository(
        name = "com_google_cloud_go_vmmigration",
        importpath = "cloud.google.com/go/vmmigration",
        sum = "h1:6AvttGxASQTiuIsNKUKOKsRiQG4qTMOY4KMyBhdZa1w=",
        version = "v1.10.0",
    )
    go_repository(
        name = "com_google_cloud_go_vmwareengine",
        importpath = "cloud.google.com/go/vmwareengine",
        sum = "h1:TKvULKbk44QrIx674cnoVjcZueXhyCAm2sNAJu/S1ds=",
        version = "v1.3.6",
    )
    go_repository(
        name = "com_google_cloud_go_vpcaccess",
        importpath = "cloud.google.com/go/vpcaccess",
        sum = "h1:K6siDR1T4HgSTv6sy6CAwupY7UGza6TQ1O8jtvEYoX4=",
        version = "v1.8.7",
    )
    go_repository(
        name = "com_google_cloud_go_webrisk",
        importpath = "cloud.google.com/go/webrisk",
        sum = "h1:q6zEdVgD8Ka+4fQl3azDcSNRug8clNnQ9iVS2iLh+MM=",
        version = "v1.11.2",
    )
    go_repository(
        name = "com_google_cloud_go_websecurityscanner",
        importpath = "cloud.google.com/go/websecurityscanner",
        sum = "h1:udhvvDDRryM3nrITJk/eQe74D06KK2N3SF60/FH2njQ=",
        version = "v1.7.7",
    )
    go_repository(
        name = "com_google_cloud_go_workflows",
        importpath = "cloud.google.com/go/workflows",
        sum = "h1:FGF6QEl3rtOSIHPOMZofWRVy3KNx26jDdgoYzJZ6ZhY=",
        version = "v1.14.3",
    )
    go_repository(
        name = "dev_cel_expr",
        importpath = "cel.dev/expr",
        sum = "h1:K6j46C81hXtZQfuX60cVWQFBJahKSE2gfRbNuvr5bFs=",
        version = "v0.25.2",
    )

    go_repository(
        name = "in_gopkg_check_v1",
        importpath = "gopkg.in/check.v1",
        sum = "h1:Hei/4ADfdWqJk1ZMxUNpqntNwaWcugrBjAiHlqqRiVk=",
        version = "v1.0.0-20201130134442-10cb98267c6c",
    )

    go_repository(
        name = "in_gopkg_fsnotify_v1",
        importpath = "gopkg.in/fsnotify.v1",
        sum = "h1:xOHLXZwVvI9hhs+cLKq5+I5onOuwQLhQwiu63xxlHs4=",
        version = "v1.4.7",
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
        name = "io_opentelemetry_go_auto_sdk",
        importpath = "go.opentelemetry.io/auto/sdk",
        sum = "h1:jXsnJ4Lmnqd11kwkBV2LgLoFMZKizbCi5fNZ/ipaZ64=",
        version = "v1.2.1",
    )
    go_repository(
        name = "io_opentelemetry_go_contrib_detectors_gcp",
        importpath = "go.opentelemetry.io/contrib/detectors/gcp",
        sum = "h1:NmLfL734pJhM0JKaYd2Y28+nY9dPRWYAAbxhRCrKXPw=",
        version = "v1.44.0",
    )
    go_repository(
        name = "io_opentelemetry_go_contrib_instrumentation_google_golang_org_grpc_otelgrpc",
        importpath = "go.opentelemetry.io/contrib/instrumentation/google.golang.org/grpc/otelgrpc",
        sum = "h1:q4XOmH/0opmeuJtPsbFNivyl7bCt7yRBbeEm2sC/XtQ=",
        version = "v0.61.0",
    )
    go_repository(
        name = "io_opentelemetry_go_contrib_instrumentation_net_http_otelhttp",
        importpath = "go.opentelemetry.io/contrib/instrumentation/net/http/otelhttp",
        sum = "h1:F7Jx+6hwnZ41NSFTO5q4LYDtJRXBf2PD0rNBkeB/lus=",
        version = "v0.61.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel",
        importpath = "go.opentelemetry.io/otel",
        sum = "h1:JjwHmHpA4iZ3wBxluu2fbbE7j4kqlE8jXyAyPXH7HqU=",
        version = "v1.44.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_exporters_stdout_stdoutmetric",
        importpath = "go.opentelemetry.io/otel/exporters/stdout/stdoutmetric",
        sum = "h1:rixTyDGXFxRy1xzhKrotaHy3/KXdPhlWARrCgK+eqUY=",
        version = "v1.36.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_metric",
        importpath = "go.opentelemetry.io/otel/metric",
        sum = "h1:1w0gILTcHdr3YI+ixLyjemwrVnsMURbTZFrSYCdDdmc=",
        version = "v1.44.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_metric_x",
        importpath = "go.opentelemetry.io/otel/metric/x",
        sum = "h1:YkCrx1zLOChi9ZcZ6euupOcsgzbVlec7D/xoEU1+cTA=",
        version = "v0.66.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_sdk",
        importpath = "go.opentelemetry.io/otel/sdk",
        sum = "h1:nHYwb9lK+fJPU/dnT6s7W7Z8itMWyqrnVfbheVYrZ58=",
        version = "v1.44.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_sdk_metric",
        importpath = "go.opentelemetry.io/otel/sdk/metric",
        sum = "h1:3LlKgI+VjbVsjNRFZJZAJ30WjXC5VkNRks6si09iEfI=",
        version = "v1.44.0",
    )
    go_repository(
        name = "io_opentelemetry_go_otel_trace",
        importpath = "go.opentelemetry.io/otel/trace",
        sum = "h1:jxF5CsGYCe74MCRx2X4g7WsY/VBKRqqpNvXlX/6gtIk=",
        version = "v1.44.0",
    )
    go_repository(
        name = "io_opentelemetry_go_proto_otlp",
        importpath = "go.opentelemetry.io/proto/otlp",
        sum = "h1:l706jCMITVouPOqEnii2fIAuO3IVGBRPV5ICjceRb/A=",
        version = "v1.9.0",
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
        sum = "h1:+Fo3DQXBK8gLdf8rFZ3uLu39JpOnhvzJrLMQSoSYZJM=",
        version = "v0.264.0",
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
        sum = "h1:VQZ/yAbAtjkHgH80teYd2em3xtIkkHd7ZhqfH2N9CsM=",
        version = "v0.0.0-20260128011058-8636f8732409",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_api",
        importpath = "google.golang.org/genproto/googleapis/api",
        sum = "h1:Kjn0N0tCrDgiAFW+lGO4JZ3ck44CehvJQMAwj9QF0G8=",
        version = "v0.0.0-20260526163538-3dc84a4a5aaa",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_bytestream",
        importpath = "google.golang.org/genproto/googleapis/bytestream",
        sum = "h1:Q9v92SXbvCsk89QPHVik5fAtq93/x/R8/KNWeS3numk=",
        version = "v0.0.0-20260122232226-8e98ce8d340d",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_rpc",
        importpath = "google.golang.org/genproto/googleapis/rpc",
        sum = "h1:mZHHdPZl0dbGHCflZgAq/Q468DWVFcU2whhB2KAo8fk=",
        version = "v0.0.0-20260526163538-3dc84a4a5aaa",
    )

    go_repository(
        name = "org_golang_google_grpc",
        importpath = "google.golang.org/grpc",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:EManeRomTObA0BU7I8vXgg/78uE5MJ9M8B39EX2WscU=",
        version = "v1.83.2",
    )
    go_repository(
        name = "org_golang_google_grpc_examples",
        importpath = "google.golang.org/grpc/examples",
        sum = "h1:ExN12ndbJ608cboPYflpTny6mXSzPrDLh0iTaVrRrds=",
        version = "v0.0.0-20250407062114-b368379ef8f6",
    )

    go_repository(
        name = "org_golang_google_protobuf",
        # Generating proto rules here would be circular: protoc-gen-go would
        # depend on the go_proto_library for gofeaturespb, which can only be
        # built by protoc-gen-go. The checked-in .pb.go files are used instead.
        build_file_proto_mode = "disable_global",
        importpath = "google.golang.org/protobuf",
        sum = "h1:fV6ZwhNocDyBLK0dj+fg8ektcVegBBuEolpbTQyBNVE=",
        version = "v1.36.11",
    )
    go_repository(
        name = "org_golang_x_crypto",
        importpath = "golang.org/x/crypto",
        sum = "h1:+KWHjbgOaAQ66dh/YlkZKHlz9ZUlq61AFirAR9ntP8M=",
        version = "v0.55.0",
    )
    go_repository(
        name = "org_golang_x_exp",
        importpath = "golang.org/x/exp",
        sum = "h1:c2HOrn5iMezYjSlGPncknSEr/8x5LELb/ilJbXi9DEA=",
        version = "v0.0.0-20190121172915-509febef88a4",
    )

    go_repository(
        name = "org_golang_x_lint",
        importpath = "golang.org/x/lint",
        sum = "h1:XQyxROzUlZH+WIQwySDgnISgOivlhjIEwaQaJEJrrN0=",
        version = "v0.0.0-20190313153728-d0100b6bd8b3",
    )

    go_repository(
        name = "org_golang_x_mod",
        importpath = "golang.org/x/mod",
        sum = "h1:MECBjubtXD7yj4HrhIUcywNaGeNVUdfVnxmPajOk4yk=",
        version = "v0.38.0",
    )
    go_repository(
        name = "org_golang_x_net",
        importpath = "golang.org/x/net",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:ynWG7rqYi4ccpTEuPZ2QGWHktVEM9DMCj9yzDE0Q7To=",
        version = "v0.58.0",
    )

    go_repository(
        name = "org_golang_x_oauth2",
        importpath = "golang.org/x/oauth2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:peZ/1z27fi9hUOFCAZaHyrpWG5lwe0RJEEEeH0ThlIs=",
        version = "v0.36.0",
    )

    go_repository(
        name = "org_golang_x_sync",
        importpath = "golang.org/x/sync",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:SZjpbeLmrCk4xhRSZFNZW5gFUeCeFgjekvI/+gfScek=",
        version = "v0.22.0",
    )

    go_repository(
        name = "org_golang_x_sys",
        importpath = "golang.org/x/sys",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:o7XGOvZQCADBQQ4Y7VNq2dRWQR7JmOUW8Kxx4ZsNgWs=",
        version = "v0.47.0",
    )
    go_repository(
        name = "org_golang_x_telemetry",
        importpath = "golang.org/x/telemetry",
        sum = "h1:RJhm5l6Fo4rmEIcndxDllNhhf/fAx8qIm4t6A7vpm2A=",
        version = "v0.0.0-20260708182218-49f421fb7959",
    )

    go_repository(
        name = "org_golang_x_term",
        importpath = "golang.org/x/term",
        sum = "h1:NwWyBmoJCbfTHpxrWoZ9C6/VxOf7ic219I8xZZFdrf0=",
        version = "v0.45.0",
    )

    go_repository(
        name = "org_golang_x_text",
        importpath = "golang.org/x/text",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:vz/seA0lnX87Othu2f/0L24RcgrXD9/YFTSuGjj3rH8=",
        version = "v0.41.0",
    )
    go_repository(
        name = "org_golang_x_time",
        importpath = "golang.org/x/time",
        sum = "h1:MRx4UaLrDotUKUdCIqzPC48t1Y9hANFKIRpNx+Te8PI=",
        version = "v0.14.0",
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
    go_repository(
        name = "org_gonum_v1_gonum",
        importpath = "gonum.org/v1/gonum",
        sum = "h1:VbpOemQlsSMrYmn7T2OUvQ4dqxQXU+ouZFQsZOx50z4=",
        version = "v0.17.0",
    )
    go_repository(
        name = "org_uber_go_goleak",
        importpath = "go.uber.org/goleak",
        sum = "h1:2K3zAYmnTNqV73imy9J1T3WC+gmCePx2hEGkimedGto=",
        version = "v1.3.0",
    )

    # gazelle:repository go_repository name=org_golang_x_tools importpath=golang.org/x/tools
    http_archive(
        name = "org_golang_x_tools",
        # Must be kept in sync with rules_go or the patches may fail.
        urls = [
            "https://mirror.bazel.build/github.com/golang/tools/archive/refs/tags/v0.30.0.zip",
            "https://github.com/golang/tools/archive/refs/tags/v0.30.0.zip",
        ],
        sha256 = "0736b1a0aa28f48074891a0f93cef5396575dbd73b9b5cdc4de54b2a3bfa4b4b",
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

def remote_jdk21_repos():
    """Imports OpenJDK 21 repositories."""

    maybe(
        _remote_jdk_repository,
        name = "remotejdk21_linux",
        os = "linux",
        cpu = "x86_64",
        version = "21.28.85-ca-jdk21.0.0",
        sha256 = "0c0eadfbdc47a7ca64aeab51b9c061f71b6e4d25d2d87674512e9b6387e9e3a6",
    )

    maybe(
        _remote_jdk_repository,
        name = "remotejdk21_macos",
        os = "macos",
        cpu = "x86_64",
        version = "21.28.85-ca-jdk21.0.0",
    )

    maybe(
        _remote_jdk_repository,
        name = "remotejdk21_macos_aarch64",
        os = "macos",
        cpu = "aarch64",
        version = "21.28.85-ca-jdk21.0.0",
    )

    maybe(
        _remote_jdk_repository,
        name = "remotejdk21_win",
        os = "windows",
        cpu = "x86_64",
        version = "21.28.85-ca-jdk21.0.0",
    )
