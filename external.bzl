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
    "remote_jdk20_repos",
    "rules_java_dependencies",
)
load("@rules_java//toolchains:remote_java_repository.bzl", "remote_java_repository")
load("@rules_jvm_external//:defs.bzl", "maven_install")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@rules_python//python:repositories.bzl", "py_repositories")

def _rule_dependencies():
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
        commit = "8edcd4f7cad4f35e9bd304ff9d45a035c50c9290",
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
        sum = "h1:YHkwssgvCXDRU7sLCq1kGqaGYO9pKNR1Wku7UT2LhoY=",
        version = "v0.34.0",
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
        sum = "h1:HTuxyug8GyFbRkrffIpzNCSK4luc0TY3wzXvzIZhEXc=",
        version = "v4.6.0",
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
        sum = "h1:DC2CZ1Ep5Y4k3ZQ899DldepgrayRUGE6BBZ/cd9Cj44=",
        version = "v2.2.0",
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
        sum = "h1:/inchEIKaYC1Akx+H+gqO04wryn5h75LSazbRlnya1k=",
        version = "v0.0.0-20230607035331-e9ce68804cb4",
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
        name = "com_github_envoyproxy_go_control_plane",
        importpath = "github.com/envoyproxy/go-control-plane",
        sum = "h1:wSUXTlLfiAQRWs2F+p+EKOY9rUyis1MyGqJ2DIk5HpM=",
        version = "v0.11.1",
    )
    go_repository(
        name = "com_github_envoyproxy_protoc_gen_validate",
        importpath = "github.com/envoyproxy/protoc-gen-validate",
        sum = "h1:QkIBuU5k+x7/QXPvPPnWXWlCdaBFApVqftFV6k087DA=",
        version = "v1.0.2",
    )

    go_repository(
        name = "com_github_fsnotify_fsnotify",
        importpath = "github.com/fsnotify/fsnotify",
        sum = "h1:n+5WquG0fcWoWp6xPWfHdbskMCQaFnG6PfBrh1Ky4HY=",
        version = "v1.6.0",
    )

    go_repository(
        name = "com_github_golang_glog",
        importpath = "github.com/golang/glog",
        sum = "h1:/d3pCKDPWNnvIWe0vVUpNP32qc8U3PDVxySP/y360qE=",
        version = "v1.1.0",
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
        sum = "h1:ofyhxvXcZhMsU5ulbFiLKl/XBFqE1GSq7atu8tAmTRI=",
        version = "v0.6.0",
    )
    go_repository(
        name = "com_github_google_go_pkcs11",
        importpath = "github.com/google/go-pkcs11",
        sum = "h1:OF1IPgv+F4NmqmJ98KTjdN97Vs1JxDPB3vbmYzV2dpk=",
        version = "v0.2.1-0.20230907215043-c6f79328ddf9",
    )

    go_repository(
        name = "com_github_google_martian_v3",
        importpath = "github.com/google/martian/v3",
        sum = "h1:IqNFLAmvJOgVlpdEBiQbDc2EwKW77amAycfTuWKdfvw=",
        version = "v3.3.2",
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
        sum = "h1:60BLSyTrOV4/haCDW4zb1guZItoSq8foHCXrAnjBo/o=",
        version = "v0.1.7",
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
        sum = "h1:KjJaJ9iWZ3jOFZIf1Lqf4laDRCasjl0BCmnEGxkdLb4=",
        version = "v1.3.1",
    )
    go_repository(
        name = "com_github_googleapis_enterprise_certificate_proxy",
        importpath = "github.com/googleapis/enterprise-certificate-proxy",
        sum = "h1:SBWmZhjUDRorQxrN0nwzf+AHBxnbFjViHQS4P0yVpmQ=",
        version = "v0.3.1",
    )

    go_repository(
        name = "com_github_googleapis_gax_go_v2",
        build_file_proto_mode = "disable",
        importpath = "github.com/googleapis/gax-go/v2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:A+gCJKdRfqXkr+BIRGtZLibNXf0m1f9E4HG56etFpas=",
        version = "v2.12.0",
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
        sum = "h1:L/CwN0zerZDmRFUapSPitk6f+Q3+0za1rQkzVuMiMFI=",
        version = "v0.1.0",
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
        sum = "h1:45sCR5RtlFHMR4UwH9sdQ5TC8v0qDQCHnXt+kaKSTVE=",
        version = "v0.1.0",
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
        name = "com_github_pmezard_go_difflib",
        importpath = "github.com/pmezard/go-difflib",
        sum = "h1:4DBwDE0NGyQoBHbLQYPwSUPoCMWR5BEzIk/f1lZbAQM=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_prometheus_client_model",
        importpath = "github.com/prometheus/client_model",
        sum = "h1:gQz4mCbXsO+nc9n1hCxHcGA3Zx3Eo+UHZoInFGUIXNM=",
        version = "v0.0.0-20190812154241-14fe0d1b01d4",
    )
    go_repository(
        name = "com_github_puerkitobio_goquery",
        importpath = "github.com/PuerkitoBio/goquery",
        sum = "h1:uQxhNlArOIdbrH1tr0UXwdVFgDcZDrZVdcpygAcwmWM=",
        version = "v1.8.1",
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
        name = "com_github_stretchr_objx",
        importpath = "github.com/stretchr/objx",
        sum = "h1:1zr/of2m5FGMsad5YfcqgdqdWrIhu+EBEJRhR1U7z/c=",
        version = "v0.5.0",
    )
    go_repository(
        name = "com_github_stretchr_testify",
        importpath = "github.com/stretchr/testify",
        sum = "h1:w7B6lhMri9wdJUVmEZPGGhZzrYTPvgJArz7wNPgYKsk=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_github_syndtr_goleveldb",
        importpath = "github.com/syndtr/goleveldb",
        sum = "h1:fBdIW9lB4Iz0n9khmH8w27SJ3QEJ7+IgjPEwGSZiFdE=",
        version = "v1.0.0",
    )

    go_repository(
        name = "com_google_cloud_go",
        importpath = "cloud.google.com/go",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:tyNdfIxjzaWctIiLYOTalaLKZ17SI44SKFW26QbOhME=",
        version = "v0.110.8",
    )
    go_repository(
        name = "com_google_cloud_go_accessapproval",
        importpath = "cloud.google.com/go/accessapproval",
        sum = "h1:/5YjNhR6lzCvmJZAnByYkfEgWjfAKwYP6nkuTk6nKFE=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_accesscontextmanager",
        importpath = "cloud.google.com/go/accesscontextmanager",
        sum = "h1:WIAt9lW9AXtqw/bnvrEUaE8VG/7bAAeMzRCBGMkc4+w=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_aiplatform",
        importpath = "cloud.google.com/go/aiplatform",
        sum = "h1:OEYe9gBca6ydogM2G0mQRMHE+aUhEV4cyaqxbk3+M1o=",
        version = "v1.51.0",
    )
    go_repository(
        name = "com_google_cloud_go_analytics",
        importpath = "cloud.google.com/go/analytics",
        sum = "h1:TFBC1ZAqX9/jL56GEXdLrVe5vT3I22bDVWyDwZX4IEg=",
        version = "v0.21.3",
    )
    go_repository(
        name = "com_google_cloud_go_apigateway",
        importpath = "cloud.google.com/go/apigateway",
        sum = "h1:aBSwCQPcp9rZ0zVEUeJbR623palnqtvxJlUyvzsKGQc=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_apigeeconnect",
        importpath = "cloud.google.com/go/apigeeconnect",
        sum = "h1:6u/jj0P2c3Mcm+H9qLsXI7gYcTiG9ueyQL3n6vCmFJM=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_apigeeregistry",
        importpath = "cloud.google.com/go/apigeeregistry",
        sum = "h1:hgq0ANLDx7t2FDZDJQrCMtCtddR/pjCqVuvQWGrQbXw=",
        version = "v0.7.1",
    )

    go_repository(
        name = "com_google_cloud_go_appengine",
        importpath = "cloud.google.com/go/appengine",
        sum = "h1:J+aaUZ6IbTpBegXbmEsh8qZZy864ZVnOoWyfa1XSNbI=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_area120",
        importpath = "cloud.google.com/go/area120",
        sum = "h1:wiOq3KDpdqXmaHzvZwKdpoM+3lDcqsI2Lwhyac7stss=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_artifactregistry",
        importpath = "cloud.google.com/go/artifactregistry",
        sum = "h1:xJIxeBs9ZYH2j8Wg/K/aCkroNUnDXgIhSHgz5FUE/Q4=",
        version = "v1.14.2",
    )
    go_repository(
        name = "com_google_cloud_go_asset",
        importpath = "cloud.google.com/go/asset",
        sum = "h1:4SdWreholqB0ZOHjBO+K+RSsW9TcZBbfpfXtFir23R0=",
        version = "v1.15.0",
    )
    go_repository(
        name = "com_google_cloud_go_assuredworkloads",
        importpath = "cloud.google.com/go/assuredworkloads",
        sum = "h1:yaO0kwS+SnhVSTF7BqTyVGt3DTocI6Jqo+S3hHmCwNk=",
        version = "v1.11.1",
    )
    go_repository(
        name = "com_google_cloud_go_automl",
        importpath = "cloud.google.com/go/automl",
        sum = "h1:iP9iQurb0qbz+YOOMfKSEjhONA/WcoOIjt6/m+6pIgo=",
        version = "v1.13.1",
    )
    go_repository(
        name = "com_google_cloud_go_baremetalsolution",
        importpath = "cloud.google.com/go/baremetalsolution",
        sum = "h1:3zztyuQHjfU0C0qEsI9LkC3kf5/TQQ3jUJhbmetUoRA=",
        version = "v1.2.0",
    )
    go_repository(
        name = "com_google_cloud_go_batch",
        importpath = "cloud.google.com/go/batch",
        sum = "h1:xjhQeEcBXJDxW2cBZEQgCKlGeXRlVJildU67rtoBY6A=",
        version = "v1.5.0",
    )
    go_repository(
        name = "com_google_cloud_go_beyondcorp",
        importpath = "cloud.google.com/go/beyondcorp",
        sum = "h1:VPg+fZXULQjs8LiMeWdLaB5oe8G9sEoZ0I0j6IMiG1Q=",
        version = "v1.0.0",
    )

    go_repository(
        name = "com_google_cloud_go_bigquery",
        importpath = "cloud.google.com/go/bigquery",
        sum = "h1:LHIc9E7Kw+ftFpQFKzZYBB88IAFz7qONawXXx0F3QBo=",
        version = "v1.56.0",
    )
    go_repository(
        name = "com_google_cloud_go_billing",
        importpath = "cloud.google.com/go/billing",
        sum = "h1:YSu8a17uJ6sOnlrnJVOBWkimmHZGtSpSkyElv9+JjRM=",
        version = "v1.17.1",
    )
    go_repository(
        name = "com_google_cloud_go_binaryauthorization",
        importpath = "cloud.google.com/go/binaryauthorization",
        sum = "h1:7L6uUWo/xNCfdVNnnzh2M4x5YA732YPgqRdCG8aKVAU=",
        version = "v1.7.0",
    )
    go_repository(
        name = "com_google_cloud_go_certificatemanager",
        importpath = "cloud.google.com/go/certificatemanager",
        sum = "h1:uKsohpE0hiobx1Eak9jNcPCznwfB6gvyQCcS28Ah9E8=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_channel",
        importpath = "cloud.google.com/go/channel",
        sum = "h1:Hy2EaOiOB7BS1IJmg2lLilEo8uMfFWTy7RgjTzbUqjM=",
        version = "v1.17.0",
    )
    go_repository(
        name = "com_google_cloud_go_cloudbuild",
        importpath = "cloud.google.com/go/cloudbuild",
        sum = "h1:YTMxmFra7eIjKFgnyQUxOwWNseNqeO38kGh7thy7v4s=",
        version = "v1.14.0",
    )
    go_repository(
        name = "com_google_cloud_go_clouddms",
        importpath = "cloud.google.com/go/clouddms",
        sum = "h1:vTcaFaFZTZZ11gXB6aZHdAx+zn30P8YJw4X/S3NC+VQ=",
        version = "v1.7.0",
    )
    go_repository(
        name = "com_google_cloud_go_cloudtasks",
        importpath = "cloud.google.com/go/cloudtasks",
        sum = "h1:cMh9Q6dkvh+Ry5LAPbD/U2aw6KAqdiU6FttwhbTo69w=",
        version = "v1.12.1",
    )
    go_repository(
        name = "com_google_cloud_go_compute",
        importpath = "cloud.google.com/go/compute",
        sum = "h1:tP41Zoavr8ptEqaW6j+LQOnyBBhO7OkOMAGrgLopTwY=",
        version = "v1.23.0",
    )
    go_repository(
        name = "com_google_cloud_go_compute_metadata",
        importpath = "cloud.google.com/go/compute/metadata",
        sum = "h1:mg4jlk7mCAj6xXp9UJ4fjI9VUI5rubuGBW5aJ7UnBMY=",
        version = "v0.2.3",
    )
    go_repository(
        name = "com_google_cloud_go_contactcenterinsights",
        importpath = "cloud.google.com/go/contactcenterinsights",
        sum = "h1:u3GlrTrchHF91z58TBSdQ80G6UbVvF6Egb4utrjSvtI=",
        version = "v1.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_container",
        importpath = "cloud.google.com/go/container",
        sum = "h1:SszQdI0qlyKsImz8/l26rpTZMyqvaH9yfua7rirDZvY=",
        version = "v1.26.0",
    )
    go_repository(
        name = "com_google_cloud_go_containeranalysis",
        importpath = "cloud.google.com/go/containeranalysis",
        sum = "h1:/EsoP+UTIjvl4yqrLA4WgUG83kwQhqZmbXEfqirT2LM=",
        version = "v0.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_datacatalog",
        importpath = "cloud.google.com/go/datacatalog",
        sum = "h1:AZHHhoSEK4n3yMsHFLibUjMX5jQz/0FcKKD4T1vxyGM=",
        version = "v1.18.0",
    )
    go_repository(
        name = "com_google_cloud_go_dataflow",
        importpath = "cloud.google.com/go/dataflow",
        sum = "h1:VzG2tqsk/HbmOtq/XSfdF4cBvUWRK+S+oL9k4eWkENQ=",
        version = "v0.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_dataform",
        importpath = "cloud.google.com/go/dataform",
        sum = "h1:xcWso0hKOoxeW72AjBSIp/UfkvpqHNzzS0/oygHlcqY=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_datafusion",
        importpath = "cloud.google.com/go/datafusion",
        sum = "h1:eX9CZoyhKQW6g1Xj7+RONeDj1mV8KQDKEB9KLELX9/8=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_datalabeling",
        importpath = "cloud.google.com/go/datalabeling",
        sum = "h1:zxsCD/BLKXhNuRssen8lVXChUj8VxF3ofN06JfdWOXw=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_dataplex",
        importpath = "cloud.google.com/go/dataplex",
        sum = "h1:wqPAP1vRskOoWwNka1yey2wxxCrxRrcxJf78MyFvrbs=",
        version = "v1.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_dataproc_v2",
        importpath = "cloud.google.com/go/dataproc/v2",
        sum = "h1:jKijbdsERm2hy/5dFl/LeQN+7CNssLdGXQYBMvMH/M4=",
        version = "v2.2.0",
    )

    go_repository(
        name = "com_google_cloud_go_dataqna",
        importpath = "cloud.google.com/go/dataqna",
        sum = "h1:ITpUJep04hC9V7C+gcK390HO++xesQFSUJ7S4nSnF3U=",
        version = "v0.8.1",
    )

    go_repository(
        name = "com_google_cloud_go_datastore",
        importpath = "cloud.google.com/go/datastore",
        sum = "h1:0P9WcsQeTWjuD1H14JIY7XQscIPQ4Laje8ti96IC5vg=",
        version = "v1.15.0",
    )
    go_repository(
        name = "com_google_cloud_go_datastream",
        importpath = "cloud.google.com/go/datastream",
        sum = "h1:ra/+jMv36zTAGPfi8TRne1hXme+UsKtdcK4j6bnqQiw=",
        version = "v1.10.0",
    )
    go_repository(
        name = "com_google_cloud_go_deploy",
        importpath = "cloud.google.com/go/deploy",
        sum = "h1:A+w/xpWgz99EYzB6e31gMGAI/P5jTZ2UO7veQK5jQ8o=",
        version = "v1.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_dialogflow",
        importpath = "cloud.google.com/go/dialogflow",
        sum = "h1:F/fSUxRD/fAfjqjClwSzg1OsQGdDG7SbO1i4x5SHuUI=",
        version = "v1.44.0",
    )
    go_repository(
        name = "com_google_cloud_go_dlp",
        importpath = "cloud.google.com/go/dlp",
        sum = "h1:tF3wsJ2QulRhRLWPzWVkeDz3FkOGVoMl6cmDUHtfYxw=",
        version = "v1.10.1",
    )
    go_repository(
        name = "com_google_cloud_go_documentai",
        importpath = "cloud.google.com/go/documentai",
        sum = "h1:Gxrx8dgCjEPsJGjsI6wPdaURNG9tniQv7xDGQmLPNw0=",
        version = "v1.23.0",
    )
    go_repository(
        name = "com_google_cloud_go_domains",
        importpath = "cloud.google.com/go/domains",
        sum = "h1:rqz6KY7mEg7Zs/69U6m6LMbB7PxFDWmT3QWNXIqhHm0=",
        version = "v0.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_edgecontainer",
        importpath = "cloud.google.com/go/edgecontainer",
        sum = "h1:zhHWnLzg6AqzE+I3gzJqiIwHfjEBhWctNQEzqb+FaRo=",
        version = "v1.1.1",
    )
    go_repository(
        name = "com_google_cloud_go_errorreporting",
        importpath = "cloud.google.com/go/errorreporting",
        sum = "h1:kj1XEWMu8P0qlLhm3FwcaFsUvXChV/OraZwA70trRR0=",
        version = "v0.3.0",
    )
    go_repository(
        name = "com_google_cloud_go_essentialcontacts",
        importpath = "cloud.google.com/go/essentialcontacts",
        sum = "h1:OEJ0MLXXCW/tX1fkxzEZOsv/wRfyFsvDVNaHWBAvoV0=",
        version = "v1.6.2",
    )
    go_repository(
        name = "com_google_cloud_go_eventarc",
        importpath = "cloud.google.com/go/eventarc",
        sum = "h1:xIP3XZi0Xawx8DEfh++mE2lrIi5kQmCr/KcWhJ1q0J4=",
        version = "v1.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_filestore",
        importpath = "cloud.google.com/go/filestore",
        sum = "h1:Eiz8xZzMJc5ppBWkuaod/PUdUZGCFR8ku0uS+Ah2fRw=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_firestore",
        importpath = "cloud.google.com/go/firestore",
        sum = "h1:/3S4RssUV4GO/kvgJZB+tayjhOfyAHs+KcpJgRVu/Qk=",
        version = "v1.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_functions",
        importpath = "cloud.google.com/go/functions",
        sum = "h1:LtAyqvO1TFmNLcROzHZhV0agEJfBi+zfMZsF4RT/a7U=",
        version = "v1.15.1",
    )

    go_repository(
        name = "com_google_cloud_go_gkebackup",
        importpath = "cloud.google.com/go/gkebackup",
        sum = "h1:Kfha8SOF2tqsu4O4jVle66mk7qNdlJ2KhL3E2YyiNZc=",
        version = "v1.3.1",
    )
    go_repository(
        name = "com_google_cloud_go_gkeconnect",
        importpath = "cloud.google.com/go/gkeconnect",
        sum = "h1:a1ckRvVznnuvDWESM2zZDzSVFvggeBaVY5+BVB8tbT0=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_gkehub",
        importpath = "cloud.google.com/go/gkehub",
        sum = "h1:2BLSb8i+Co1P05IYCKATXy5yaaIw/ZqGvVSBTLdzCQo=",
        version = "v0.14.1",
    )
    go_repository(
        name = "com_google_cloud_go_gkemulticloud",
        importpath = "cloud.google.com/go/gkemulticloud",
        sum = "h1:MluqhtPVZReoriP5+adGIw+ij/RIeRik8KApCW2WMTw=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_google_cloud_go_gsuiteaddons",
        importpath = "cloud.google.com/go/gsuiteaddons",
        sum = "h1:mi9jxZpzVjLQibTS/XfPZvl+Jr6D5Bs8pGqUjllRb00=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_iam",
        importpath = "cloud.google.com/go/iam",
        sum = "h1:gacbrBdWcoVmGLozRuStX45YKvJtzIjJdAolzUs1sm4=",
        version = "v1.1.2",
    )
    go_repository(
        name = "com_google_cloud_go_iap",
        importpath = "cloud.google.com/go/iap",
        sum = "h1:RNhVq/6OMI99/wjPVhqFxjlBxYOBRdaG6rLpBvyaqYY=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_google_cloud_go_ids",
        importpath = "cloud.google.com/go/ids",
        sum = "h1:khXYmSoDDhWGEVxHl4c4IgbwSRR+qE/L4hzP3vaU9Hc=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_google_cloud_go_iot",
        importpath = "cloud.google.com/go/iot",
        sum = "h1:yrH0OSmicD5bqGBoMlWG8UltzdLkYzNUwNVUVz7OT54=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_kms",
        importpath = "cloud.google.com/go/kms",
        sum = "h1:lh6qra6oC4AyWe5fUUUBe/S27k12OHAleOOOw6KakdE=",
        version = "v1.15.2",
    )
    go_repository(
        name = "com_google_cloud_go_language",
        importpath = "cloud.google.com/go/language",
        sum = "h1:KnYolG0T5Oex722ZW/sP5QErhVAVNcqpJ16tVJd9RTw=",
        version = "v1.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_lifesciences",
        importpath = "cloud.google.com/go/lifesciences",
        sum = "h1:axkANGx1wiBXHiPcJZAE+TDjjYoJRIDzbHC/WYllCBU=",
        version = "v0.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_logging",
        importpath = "cloud.google.com/go/logging",
        sum = "h1:26skQWPeYhvIasWKm48+Eq7oUqdcdbwsCVwz5Ys0FvU=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_longrunning",
        importpath = "cloud.google.com/go/longrunning",
        sum = "h1:Fr7TXftcqTudoyRJa113hyaqlGdiBQkp0Gq7tErFDWI=",
        version = "v0.5.1",
    )
    go_repository(
        name = "com_google_cloud_go_managedidentities",
        importpath = "cloud.google.com/go/managedidentities",
        sum = "h1:2/qZuOeLgUHorSdxSQGtnOu9xQkBn37+j+oZQv/KHJY=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_maps",
        importpath = "cloud.google.com/go/maps",
        sum = "h1:PdfgpBLhAoSzZrQXP+/zBc78fIPLZSJp5y8+qSMn2UU=",
        version = "v1.4.0",
    )
    go_repository(
        name = "com_google_cloud_go_mediatranslation",
        importpath = "cloud.google.com/go/mediatranslation",
        sum = "h1:50cF7c1l3BanfKrpnTCaTvhf+Fo6kdF21DG0byG7gYU=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_memcache",
        importpath = "cloud.google.com/go/memcache",
        sum = "h1:7lkLsF0QF+Mre0O/NvkD9Q5utUNwtzvIYjrOLOs0HO0=",
        version = "v1.10.1",
    )
    go_repository(
        name = "com_google_cloud_go_metastore",
        importpath = "cloud.google.com/go/metastore",
        sum = "h1:iMMU4DY4yojvKatMfv1q9WqBHi3ZrcwAIYQ+ZrlXM2o=",
        version = "v1.13.0",
    )
    go_repository(
        name = "com_google_cloud_go_monitoring",
        importpath = "cloud.google.com/go/monitoring",
        sum = "h1:rlndy4K8yknMY9JuGe2aK4SbCh21FXoCdX7SAGHmRgI=",
        version = "v1.16.0",
    )
    go_repository(
        name = "com_google_cloud_go_networkconnectivity",
        importpath = "cloud.google.com/go/networkconnectivity",
        sum = "h1:NBuIU8e/kBNAJE7Rgij+wyXZMcybnlxVTpoZEyQppHc=",
        version = "v1.14.0",
    )
    go_repository(
        name = "com_google_cloud_go_networkmanagement",
        importpath = "cloud.google.com/go/networkmanagement",
        sum = "h1:aA6L8aioyM4S6nlPYzp2SvB88lBcByZmqMJM6ReafzU=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_google_cloud_go_networksecurity",
        importpath = "cloud.google.com/go/networksecurity",
        sum = "h1:TBLEkMp3AE+6IV/wbIGRNTxnqLXHCTEQWoxRVC18TzY=",
        version = "v0.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_notebooks",
        importpath = "cloud.google.com/go/notebooks",
        sum = "h1:6x2K1JAWv6RW2yQO6oa+xtKUGOpGQseCmT94vpOt1vc=",
        version = "v1.10.0",
    )
    go_repository(
        name = "com_google_cloud_go_optimization",
        importpath = "cloud.google.com/go/optimization",
        sum = "h1:sGvPVtBJUKNYAwldhJvFmnM+EEdOXjDzjcly3g0n0Xg=",
        version = "v1.5.0",
    )
    go_repository(
        name = "com_google_cloud_go_orchestration",
        importpath = "cloud.google.com/go/orchestration",
        sum = "h1:KmN18kE/xa1n91cM5jhCh7s1/UfIguSCisw7nTMUzgE=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_orgpolicy",
        importpath = "cloud.google.com/go/orgpolicy",
        sum = "h1:I/7dHICQkNwym9erHqmlb50LRU588NPCvkfIY0Bx9jI=",
        version = "v1.11.1",
    )
    go_repository(
        name = "com_google_cloud_go_osconfig",
        importpath = "cloud.google.com/go/osconfig",
        sum = "h1:dgyEHdfqML6cUW6/MkihNdTVc0INQst0qSE8Ou1ub9c=",
        version = "v1.12.1",
    )
    go_repository(
        name = "com_google_cloud_go_oslogin",
        importpath = "cloud.google.com/go/oslogin",
        sum = "h1:7OA/BHWna8s+8k1sjTLHs0zRttoktR8a36qjWIvzTco=",
        version = "v1.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_phishingprotection",
        importpath = "cloud.google.com/go/phishingprotection",
        sum = "h1:aK/lNmSd1vtbft/vLe2g7edXK72sIQbqr2QyrZN/iME=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_policytroubleshooter",
        importpath = "cloud.google.com/go/policytroubleshooter",
        sum = "h1:pT4qSiL5o0hBSWHDiOcmes/s301PeLLWEhAr/eMQB/g=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_google_cloud_go_privatecatalog",
        importpath = "cloud.google.com/go/privatecatalog",
        sum = "h1:B/18xGo+E0EMS9LOEQ0zXz7F2asMgmVgTYGSI89MHOA=",
        version = "v0.9.1",
    )

    go_repository(
        name = "com_google_cloud_go_pubsub",
        importpath = "cloud.google.com/go/pubsub",
        sum = "h1:6SPCPvWav64tj0sVX/+npCBKhUi/UjJehy9op/V3p2g=",
        version = "v1.33.0",
    )
    go_repository(
        name = "com_google_cloud_go_pubsublite",
        importpath = "cloud.google.com/go/pubsublite",
        sum = "h1:pX+idpWMIH30/K7c0epN6V703xpIcMXWRjKJsz0tYGY=",
        version = "v1.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_recaptchaenterprise_v2",
        importpath = "cloud.google.com/go/recaptchaenterprise/v2",
        sum = "h1:4qPn0UZ1LUZje+JcmJcVRtsR2qbCMwCFU+MmsCRzDbk=",
        version = "v2.8.0",
    )
    go_repository(
        name = "com_google_cloud_go_recommendationengine",
        importpath = "cloud.google.com/go/recommendationengine",
        sum = "h1:nMr1OEVHuDambRn+/y4RmNAmnR/pXCuHtH0Y4tCgGRQ=",
        version = "v0.8.1",
    )
    go_repository(
        name = "com_google_cloud_go_recommender",
        importpath = "cloud.google.com/go/recommender",
        sum = "h1:SuzbMJhDAiPro7tR9QP7EX97+TI31urjsIgNh9XQHl8=",
        version = "v1.11.0",
    )
    go_repository(
        name = "com_google_cloud_go_redis",
        importpath = "cloud.google.com/go/redis",
        sum = "h1:YrjQnCC7ydk+k30op7DSjSHw1yAYhqYXFcOq1bSXRYA=",
        version = "v1.13.1",
    )
    go_repository(
        name = "com_google_cloud_go_resourcemanager",
        importpath = "cloud.google.com/go/resourcemanager",
        sum = "h1:QIAMfndPOHR6yTmMUB0ZN+HSeRmPjR/21Smq5/xwghI=",
        version = "v1.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_resourcesettings",
        importpath = "cloud.google.com/go/resourcesettings",
        sum = "h1:Fdyq418U69LhvNPFdlEO29w+DRRjwDA4/pFamm4ksAg=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_retail",
        importpath = "cloud.google.com/go/retail",
        sum = "h1:gYBrb9u/Hc5s5lUTFXX1Vsbc/9BEvgtioY6ZKaK0DK8=",
        version = "v1.14.1",
    )
    go_repository(
        name = "com_google_cloud_go_run",
        importpath = "cloud.google.com/go/run",
        sum = "h1:NR3ibstYygrvNZQ+7+rSWmD+oKvbjB/B9Ve9mqhkj6s=",
        version = "v1.3.0",
    )
    go_repository(
        name = "com_google_cloud_go_scheduler",
        importpath = "cloud.google.com/go/scheduler",
        sum = "h1:yoZbZR8880KgPGLmACOMCiY2tPk+iX4V/dkxqTirlz8=",
        version = "v1.10.1",
    )
    go_repository(
        name = "com_google_cloud_go_secretmanager",
        importpath = "cloud.google.com/go/secretmanager",
        sum = "h1:cLTCwAjFh9fKvU6F13Y4L9vPcx9yiWPyWXE4+zkuEQs=",
        version = "v1.11.1",
    )
    go_repository(
        name = "com_google_cloud_go_security",
        importpath = "cloud.google.com/go/security",
        sum = "h1:jR3itwycg/TgGA0uIgTItcVhA55hKWiNJxaNNpQJaZE=",
        version = "v1.15.1",
    )
    go_repository(
        name = "com_google_cloud_go_securitycenter",
        importpath = "cloud.google.com/go/securitycenter",
        sum = "h1:XOGJ9OpnDtqg8izd7gYk/XUhj8ytjIalyjjsR6oyG0M=",
        version = "v1.23.0",
    )

    go_repository(
        name = "com_google_cloud_go_servicedirectory",
        importpath = "cloud.google.com/go/servicedirectory",
        sum = "h1:pBWpjCFVGWkzVTkqN3TBBIqNSoSHY86/6RL0soSQ4z8=",
        version = "v1.11.0",
    )

    go_repository(
        name = "com_google_cloud_go_shell",
        importpath = "cloud.google.com/go/shell",
        sum = "h1:aHbwH9LSqs4r2rbay9f6fKEls61TAjT63jSyglsw7sI=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_spanner",
        importpath = "cloud.google.com/go/spanner",
        sum = "h1:QrJFOpaxCXdXF+GkiruLz642PHxkdj68PbbnLw3O2Zw=",
        version = "v1.50.0",
    )
    go_repository(
        name = "com_google_cloud_go_speech",
        importpath = "cloud.google.com/go/speech",
        sum = "h1:MCagaq8ObV2tr1kZJcJYgXYbIn8Ai5rp42tyGYw9rls=",
        version = "v1.19.0",
    )

    go_repository(
        name = "com_google_cloud_go_storage",
        importpath = "cloud.google.com/go/storage",
        sum = "h1:PVrDOkIC8qQVa1P3SXGpQvfuJhN2LHOoyZvWs8D2X5M=",
        version = "v1.33.0",
    )
    go_repository(
        name = "com_google_cloud_go_storagetransfer",
        importpath = "cloud.google.com/go/storagetransfer",
        sum = "h1:+ZLkeXx0K0Pk5XdDmG0MnUVqIR18lllsihU/yq39I8Q=",
        version = "v1.10.0",
    )
    go_repository(
        name = "com_google_cloud_go_talent",
        importpath = "cloud.google.com/go/talent",
        sum = "h1:j46ZgD6N2YdpFPux9mc7OAf4YK3tiBCsbLKc8rQx+bU=",
        version = "v1.6.2",
    )
    go_repository(
        name = "com_google_cloud_go_texttospeech",
        importpath = "cloud.google.com/go/texttospeech",
        sum = "h1:S/pR/GZT9p15R7Y2dk2OXD/3AufTct/NSxT4a7nxByw=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_tpu",
        importpath = "cloud.google.com/go/tpu",
        sum = "h1:kQf1jgPY04UJBYYjNUO+3GrZtIb57MfGAW2bwgLbR3A=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_trace",
        importpath = "cloud.google.com/go/trace",
        sum = "h1:EwGdOLCNfYOOPtgqo+D2sDLZmRCEO1AagRTJCU6ztdg=",
        version = "v1.10.1",
    )
    go_repository(
        name = "com_google_cloud_go_translate",
        importpath = "cloud.google.com/go/translate",
        sum = "h1:0na4gC54Lu05ir00dmUSuMkLAojDe1ALq4hBTUkhwjE=",
        version = "v1.9.0",
    )
    go_repository(
        name = "com_google_cloud_go_video",
        importpath = "cloud.google.com/go/video",
        sum = "h1:AkjXyJfQ7DtPyDOAbTMeiGcuKsO8/iKSb3fAmTUHYSg=",
        version = "v1.20.0",
    )
    go_repository(
        name = "com_google_cloud_go_videointelligence",
        importpath = "cloud.google.com/go/videointelligence",
        sum = "h1:MBMWnkQ78GQnRz5lfdTAbBq/8QMCF3wahgtHh3s/J+k=",
        version = "v1.11.1",
    )
    go_repository(
        name = "com_google_cloud_go_vision_v2",
        importpath = "cloud.google.com/go/vision/v2",
        sum = "h1:ccK6/YgPfGHR/CyESz1mvIbsht5Y2xRsWCPqmTNydEw=",
        version = "v2.7.2",
    )
    go_repository(
        name = "com_google_cloud_go_vmmigration",
        importpath = "cloud.google.com/go/vmmigration",
        sum = "h1:gnjIclgqbEMc+cF5IJuPxp53wjBIlqZ8h9hE8Rkwp7A=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_vmwareengine",
        importpath = "cloud.google.com/go/vmwareengine",
        sum = "h1:qsJ0CPlOQu/3MFBGklu752v3AkD+Pdu091UmXJ+EjTA=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_google_cloud_go_vpcaccess",
        importpath = "cloud.google.com/go/vpcaccess",
        sum = "h1:ram0GzjNWElmbxXMIzeOZUkQ9J8ZAahD6V8ilPGqX0Y=",
        version = "v1.7.1",
    )
    go_repository(
        name = "com_google_cloud_go_webrisk",
        importpath = "cloud.google.com/go/webrisk",
        sum = "h1:Ssy3MkOMOnyRV5H2bkMQ13Umv7CwB/kugo3qkAX83Fk=",
        version = "v1.9.1",
    )
    go_repository(
        name = "com_google_cloud_go_websecurityscanner",
        importpath = "cloud.google.com/go/websecurityscanner",
        sum = "h1:CfEF/vZ+xXyAR3zC9iaC/QRdf1MEgS20r5UR17Q4gOg=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_google_cloud_go_workflows",
        importpath = "cloud.google.com/go/workflows",
        sum = "h1:cSUlx4PVV9O0vYCl+pHAUmu0996A7eN602d4wjjVHRs=",
        version = "v1.12.0",
    )

    go_repository(
        name = "in_gopkg_check_v1",
        importpath = "gopkg.in/check.v1",
        sum = "h1:YR8cESwS4TdDjEe65xsg0ogRM/Nc3DYOhEAlW+xobZo=",
        version = "v1.0.0-20190902080502-41f04d3bba15",
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
        sum = "h1:9aBYT4vQXt9dhCuLNfwfd3zpwu8atg0yPkjBymwSrOM=",
        version = "v0.146.0",
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
        sum = "h1:ml3TAUoIIzQUtX88s/icpXCFW9lV5VwsuIuS1htNjKY=",
        version = "v0.0.0-20231009173412-8bfb1ae86b6c",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_api",
        importpath = "google.golang.org/genproto/googleapis/api",
        sum = "h1:0RtEmmHjemvUXloH7+RuBSIw7n+GEHMOMY1CkGYnWq4=",
        version = "v0.0.0-20231009173412-8bfb1ae86b6c",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_bytestream",
        importpath = "google.golang.org/genproto/googleapis/bytestream",
        sum = "h1:AzcXcS6RbpBm65S0+/F78J9hFCL0/GZWp8oCRZod780=",
        version = "v0.0.0-20230920204549-e6e6cdab5c13",
    )
    go_repository(
        name = "org_golang_google_genproto_googleapis_rpc",
        importpath = "google.golang.org/genproto/googleapis/rpc",
        sum = "h1:jHkCUWkseRf+W+edG5hMzr/Uh1xkDREY4caybAq4dpY=",
        version = "v0.0.0-20231009173412-8bfb1ae86b6c",
    )

    go_repository(
        name = "org_golang_google_grpc",
        importpath = "google.golang.org/grpc",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:BjnpXut1btbtgN/6sp+brB2Kbm2LjNXnidYujAVbSoQ=",
        version = "v1.58.3",
    )

    go_repository(
        name = "org_golang_google_protobuf",
        importpath = "google.golang.org/protobuf",
        sum = "h1:g0LDEJHgrBl9N9r17Ru3sqWhkIx2NB67okBHPwC7hs8=",
        version = "v1.31.0",
    )
    go_repository(
        name = "org_golang_x_crypto",
        importpath = "golang.org/x/crypto",
        sum = "h1:X31++rzVUdKhX5sWmSOFZxx8UW/ldWx55cbf08iNAMA=",
        version = "v0.21.0",
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
        sum = "h1:I/DsJXRlw/8l/0c24sM9yb0T4z9liZTduXvdAWYiysY=",
        version = "v0.13.0",
    )
    go_repository(
        name = "org_golang_x_net",
        importpath = "golang.org/x/net",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:7EYJ93RZ9vYSZAIb2x3lnuvqO5zneoD6IvWjuhfxjTs=",
        version = "v0.23.0",
    )

    go_repository(
        name = "org_golang_x_oauth2",
        importpath = "golang.org/x/oauth2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:jDDenyj+WgFtmV3zYVoi8aE2BwtXFLWOA67ZfNWftiY=",
        version = "v0.13.0",
    )

    go_repository(
        name = "org_golang_x_sync",
        importpath = "golang.org/x/sync",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:zxkM55ReGkDlKSM+Fu41A+zmbZuaPVbGMzvvdUPznYQ=",
        version = "v0.4.0",
    )

    go_repository(
        name = "org_golang_x_term",
        importpath = "golang.org/x/term",
        sum = "h1:FcHjZXDMxI8mM3nwhX9HlKop4C0YQvCVCdwYl2wOtE8=",
        version = "v0.18.0",
    )

    go_repository(
        name = "org_golang_x_text",
        importpath = "golang.org/x/text",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:ScX5w1eTa3QqT8oi6+ziP7dTV1S2+ALU0bI+0zXKWiQ=",
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
        sum = "h1:+cNy6SZtPcJQH3LJVLOSmiC7MMxXNOb3PU/VUEz+EhU=",
        version = "v0.0.0-20231012003039-104605ab7028",
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
