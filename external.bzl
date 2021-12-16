load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")
load("@rules_java//java:repositories.bzl", "rules_java_dependencies")
load("@rules_jvm_external//:defs.bzl", "maven_install")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@io_kythe//:setup.bzl", "github_archive")
load("@io_kythe//tools:build_rules/shims.bzl", "go_repository")
load("@io_kythe//third_party/leiningen:lein_repo.bzl", "lein_repository")
load("@io_kythe//tools/build_rules/lexyacc:lexyacc.bzl", "lexyacc_configure")
load("@io_kythe//tools/build_rules/build_event_stream:repo.bzl", "build_event_stream_repository")
load("@io_kythe//kythe/cxx/extractor:toolchain.bzl", cxx_extractor_register_toolchains = "register_toolchains")
load("@rules_python//python:repositories.bzl", "py_repositories")
load("@bazel_toolchains//repositories:repositories.bzl", bazel_toolchains_repositories = "repositories")
load("@rules_rust//rust:repositories.bzl", "rust_repositories")
load("@rules_rust//proto:repositories.bzl", "rust_proto_repositories")
load("@build_bazel_rules_nodejs//:index.bzl", "npm_install")
load(
    "@bazelruby_rules_ruby//ruby:deps.bzl",
    "rules_ruby_dependencies",
    "rules_ruby_select_sdk",
)
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
load("@llvm-project-raw//utils/bazel:configure.bzl", "llvm_configure")
load("@llvm-project-raw//utils/bazel:terminfo.bzl", "llvm_terminfo_disable")
load("@llvm-project-raw//utils/bazel:zlib.bzl", "llvm_zlib_external")

# The raze macros automatically check for duplicated dependencies so we can
# simply load each macro here.
load("//kythe/rust/cargo:crates.bzl", "raze_fetch_remote_crates")

def _rule_dependencies():
    go_rules_dependencies()
    go_register_toolchains(version = "1.18beta1")
    gazelle_dependencies()
    rules_java_dependencies()
    rules_proto_dependencies()
    py_repositories()
    bazel_toolchains_repositories()
    rust_repositories(version = "nightly", iso_date = "2021-10-16", dev_components = True)
    rust_proto_repositories()
    rules_ruby_dependencies()
    rules_ruby_select_sdk(version = "host")
    rules_foreign_cc_dependencies(register_built_tools = False)

def _gazelle_ignore(**kwargs):
    """Dummy macro which causes gazelle to see a repository as already defined."""

def _proto_dependencies():
    # Rather than pull down the entire Bazel source repository for a single file,
    # just grab the file we need and use it locally.
    maybe(
        build_event_stream_repository,
        name = "build_event_stream_proto",
        revision = "3.7.0",
        sha256s = {
            "build_event_stream.proto": "0127da17b5cd40e61b0dcb9f18bebedd7a4851538fa39627c55ffdb01839bef2",
            "command_line.proto": "a6fb6591aa50794431787169bc4fae16105ef5c401e7c30ecf0f775e0ab25c2c",
            "invocation_policy.proto": "5312a440a5d16e9bd72cd8561ad2f5d2b29579f19df7e13af1517c6ad9e7fa64",
            "option_filters.proto": "e3e8dfa9a4e05683bf1853a0be29fae46c753b18ad3d42b92bedcb412577f20f",
            "failure_details.proto": "0dca56c2f749459d76094af2fb1844e65ff65d11208bcbe7302b0570b5a1d007",
        },
    )

def _cc_dependencies():
    maybe(
        llvm_terminfo_disable,
        name = "llvm_terminfo",
    )

    maybe(
        llvm_zlib_external,
        name = "llvm_zlib",
        external_zlib = "@net_zlib//:zlib",
    )

    maybe(
        llvm_configure,
        name = "llvm-project",
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
        urls = ["https://github.com/souffle-lang/souffle/archive/fbb4c4b967bf58cccb7aca58e3d200a799218d98.zip"],
        build_file = "@io_kythe//third_party:souffle.BUILD",
        sha256 = "654c1b33b2b3f20fdc1f0983dfed562c24a0baa230fd431401cc0004464c6b4d",
        strip_prefix = "souffle-fbb4c4b967bf58cccb7aca58e3d200a799218d98",
        patch_args = ["-p0"],
        patches = [
            "@io_kythe//third_party:souffle_remove_config.patch",
        ],
    )

    maybe(
        http_archive,
        name = "net_zlib",
        build_file = "@io_kythe//third_party:zlib.BUILD",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        urls = [
            "https://mirror.bazel.build/zlib.net/zlib-1.2.11.tar.gz",
            "https://zlib.net/zlib-1.2.11.tar.gz",
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
        commit = "ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9",
        sha256 = "32a00f5834195d6656097c800a773e2fc766741e434d1eff092ed5578a21dd3a",
    )

    maybe(
        github_archive,
        name = "com_google_googletest",
        repo_name = "google/googletest",
        commit = "3005672db1d05f2378f642b61faa96f85498befe",
        sha256 = "d87849e281d376a1c955f867cf10be0d672ff41dbe7fd600bcc2faa9bcb6e23f",
    )

    maybe(
        github_archive,
        name = "com_github_google_glog",
        repo_name = "google/glog",
        commit = "d4e8ebab7e295f20f86cae9557da0d5087a02f73",
        sha256 = "b38713b8189bc621185c1d558f0dbeef6ce821688e0990b8c6d72c703769779c",
        build_file_content = "\n".join([
            "load(\"//:bazel/glog.bzl\", \"glog_library\")",
            "glog_library(with_gflags=0)",
        ]),
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
        sha256 = "fa22ce5dd42712dad6f9d47ffe0d416461ec4f8b8ad7def4fad12dbb0614e59f",
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
        sha256 = "b6d299a2e91ebe78a222e45228449f3ba569f83c6bd59d582fcb3cd425656c38",
        commit = "2020-10-01",
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
        sha256 = "38b4aabf88eb480131ed45bfb89c19ca3e2a62daeb081bdf001cfb17ec4cd303",
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

    lexyacc_configure()
    cxx_extractor_register_toolchains()

def _java_dependencies():
    maybe(
        git_repository,
        name = "io_bazel",
        commit = "20c4596365d6e198ce9e4559a372190ceedff3f5",
        remote = "https://github.com/bazelbuild/bazel",
    )
    maven_install(
        name = "maven",
        artifacts = [
            "com.google.flogger:flogger:0.7.2",
            "com.google.flogger:flogger-system-backend:0.7.2",
            "com.beust:jcommander:1.81",
            "com.google.auto.service:auto-service:1.0",
            "com.google.auto.service:auto-service-annotations:1.0",
            "com.google.auto.value:auto-value:1.8",
            "com.google.auto.value:auto-value-annotations:1.8",
            "com.google.auto:auto-common:1.0",
            "com.google.code.findbugs:jsr305:3.0.2",
            "com.google.code.gson:gson:2.8.6",
            "com.google.common.html.types:types:1.0.8",
            "com.google.errorprone:error_prone_annotations:2.6.0",
            "com.google.guava:guava:30.1.1-jre",
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
    )

def _go_dependencies():
    go_repository(
        name = "co_honnef_go_tools",
        importpath = "honnef.co/go/tools",
        sum = "h1:W18jzjh8mfPez+AwGLxmOImucz/IFjpNlrKVnaj2YVc=",
        version = "v0.0.1-2020.1.6",
    )
    go_repository(
        name = "com_github_antihax_optional",
        importpath = "github.com/antihax/optional",
        sum = "h1:xK2lYat7ZLaVVcIuj82J8kIro4V6kDe0AUDFboUCwcg=",
        version = "v1.0.0",
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

    _gazelle_ignore(
        name = "com_github_bazelbuild_rules_go",
        actual = "io_bazel_rules_go",
        importpath = "github.com/bazelbuild/rules_go",
    )
    go_repository(
        name = "com_github_beevik_etree",
        importpath = "github.com/beevik/etree",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:T0xke/WvNtMoCqgzPhkX2r4rjY3GDZFi+FjpRZY2Jbs=",
        version = "v1.1.0",
    )

    go_repository(
        name = "com_github_burntsushi_toml",
        importpath = "github.com/BurntSushi/toml",
        sum = "h1:WXkYYl6Yr3qBf1K79EBnL4mak0OimBfB0XUf9Vl28OQ=",
        version = "v0.3.1",
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
        sum = "h1:glEXhBS5PSLLv4IXzLA5yPRVX4bilULVyxxbrfOtDAk=",
        version = "v0.2.1",
    )
    _gazelle_ignore(name = "com_github_chzyer_logex")
    _gazelle_ignore(name = "com_github_chzyer_readline")
    _gazelle_ignore(name = "com_github_chzyer_test")
    go_repository(
        name = "com_github_client9_misspell",
        importpath = "github.com/client9/misspell",
        sum = "h1:ta993UF76GwbvJcIo3Y68y/M3WxlpEHPWIGDkJYwzJI=",
        version = "v0.3.4",
    )
    _gazelle_ignore(name = "com_github_cncf_udpa_go")
    go_repository(
        name = "com_github_cncf_xds_go",
        importpath = "github.com/cncf/xds/go",
        sum = "h1:OZmjad4L3H8ncOIR8rnb5MREYqG8ixi5+WbeUsquF0c=",
        version = "v0.0.0-20210312221358-fbca930ec8ed",
    )

    go_repository(
        name = "com_github_creachadair_staticfile",
        importpath = "github.com/creachadair/staticfile",
        sum = "h1:RhyrMgi7IQn3GejgmGtFuCec58vboEMt5CH6N3ulRJk=",
        version = "v0.1.3",
    )
    go_repository(
        name = "com_github_datadog_zstd",
        importpath = "github.com/DataDog/zstd",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Rpmta4xZ/MgZnriKNd24iZMhGpP5dvUcs/uqfBapKZY=",
        version = "v1.4.8",
    )

    go_repository(
        name = "com_github_davecgh_go_spew",
        importpath = "github.com/davecgh/go-spew",
        sum = "h1:vj9j/u1bqnvCEfJOwUhtlOARqs3+rkHYY13jYWTU97c=",
        version = "v1.1.1",
    )
    go_repository(
        name = "com_github_dsnet_compress",
        importpath = "github.com/dsnet/compress",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:PlZu0n3Tuv04TzpfPbrnI0HW/YwodEXDS+oPKahKF0Q=",
        version = "v0.0.1",
    )

    go_repository(
        name = "com_github_dsnet_golib",
        importpath = "github.com/dsnet/golib",
        sum = "h1:tFh1tRc4CA31yP6qDcu+Trax5wW5GuMxvkIba07qVLY=",
        version = "v0.0.0-20171103203638-1ea166775780",
    )
    go_repository(
        name = "com_github_envoyproxy_go_control_plane",
        importpath = "github.com/envoyproxy/go-control-plane",
        sum = "h1:dulLQAYQFYtG5MTplgNGHWuV2D+OBD+Z8lmDBmbLg+s=",
        version = "v0.9.9-0.20210512163311-63b5d3c536b0",
    )
    go_repository(
        name = "com_github_envoyproxy_protoc_gen_validate",
        importpath = "github.com/envoyproxy/protoc-gen-validate",
        sum = "h1:EQciDnbrYxy13PgWoY8AqoxGiPrpgBZ1R8UNe3ddc+A=",
        version = "v0.1.0",
    )

    go_repository(
        name = "com_github_frankban_quicktest",
        importpath = "github.com/frankban/quicktest",
        sum = "h1:2QxQoC1TS09S7fhCPsrvqYdvP1H5M1P1ih5ABm3BTYk=",
        version = "v1.7.2",
    )
    go_repository(
        name = "com_github_fsnotify_fsnotify",
        importpath = "github.com/fsnotify/fsnotify",
        sum = "h1:IXs+QLmnXW2CcXuY+8Mzv/fWEsPGWxqefPtCP5CnV9I=",
        version = "v1.4.7",
    )
    go_repository(
        name = "com_github_ghodss_yaml",
        importpath = "github.com/ghodss/yaml",
        sum = "h1:wQHKEahhL6wmXdzwWG11gIVCkOv05bNOh+Rxn0yngAk=",
        version = "v1.0.0",
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
        name = "com_github_golang_glog",
        importpath = "github.com/golang/glog",
        sum = "h1:VKtxabqXZkF25pY9ekfRL6a582T4P37/31XEstQ5p58=",
        version = "v0.0.0-20160126235308-23def4e6c14b",
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
        sum = "h1:ROPKBNFfQgOUMifHyP+KYbvpjbdoFNs+aK7DXlji0Tw=",
        version = "v1.5.2",
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
        name = "com_github_google_brotli",
        importpath = "github.com/google/brotli",
        sum = "h1:vgeehFs4lfG6xStg/Tr5Kjt4nEHhly0I8te7HKwPUfY=",
        version = "v1.0.9",
    )

    go_repository(
        name = "com_github_google_brotli_go_cbrotli",
        importpath = "github.com/google/brotli/go/cbrotli",
        sum = "h1:Hxy9HjQ09RzDr0nxLehWxyoLmoCsukUAkClLJMLBDE0=",
        version = "v0.0.0-20210804124202-19d86fb9a60a",
    )

    go_repository(
        name = "com_github_google_btree",
        importpath = "github.com/google/btree",
        sum = "h1:0udJVsspx3VBr5FwtLhQQtuAsVc79tTq0ocGIPAU6qo=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_google_go_cmp",
        importpath = "github.com/google/go-cmp",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:BKbKCqvP6I+rmFHt06ZmyQtvB8xAkWdhFyr0ZUNZcxQ=",
        version = "v0.5.6",
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
        sum = "h1:d8MncMlErDFTwQGBK1xhv026j9kqhvw1Qv9IbWT1VLQ=",
        version = "v3.2.1",
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
        sum = "h1:K6RDEckDVWvDI9JAJYCmNdQXq6neHJOYx3V6jnqNEec=",
        version = "v0.0.0-20210720184732-4bb14d4b1be1",
    )
    go_repository(
        name = "com_github_google_renameio",
        importpath = "github.com/google/renameio",
        sum = "h1:GOZbcHa3HfsPKPlmyPyN2KEohoMXOhdMbHrvbpl2QaA=",
        version = "v0.1.0",
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
        sum = "h1:t6JiXgmwXMjEs8VusXIJk2BXHsn+wx8BZdTaoZ5fu7I=",
        version = "v1.3.0",
    )

    go_repository(
        name = "com_github_googleapis_gax_go_v2",
        build_file_proto_mode = "disable",
        importpath = "github.com/googleapis/gax-go/v2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:sjZBwGj9Jlw33ImPtvFviGYvseOtDM7hkSKB7+Tv3SM=",
        version = "v2.0.5",
    )
    go_repository(
        name = "com_github_gorilla_websocket",
        importpath = "github.com/gorilla/websocket",
        sum = "h1:q7AeDBpnBk8AogcD4DSag/Ukw/KV+YhzLj2bP5HvKCM=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_github_grpc_ecosystem_grpc_gateway",
        importpath = "github.com/grpc-ecosystem/grpc-gateway",
        sum = "h1:gmcG1KaJ57LophUzW0Hy8NmPhnMZb4M0+kPpLofRdBo=",
        version = "v1.16.0",
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
        name = "com_github_hpcloud_tail",
        importpath = "github.com/hpcloud/tail",
        sum = "h1:nfCOvKYfkgYP8hkirhJocXT2+zOD8yUNjXaWfTlyFKI=",
        version = "v1.0.0",
    )

    _gazelle_ignore(name = "com_github_ianlancetaylor_demangle")

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
        name = "com_github_jstemmer_go_junit_report",
        importpath = "github.com/jstemmer/go-junit-report",
        sum = "h1:6QPYqodiu3GuPL+7mfx+NwDdp2eTkp9IfEUpgAwUN0o=",
        version = "v0.9.1",
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
        sum = "h1:8VMb5+0wMgdBykOV96DwNwKFQ+WTI4pzYURP99CcB9E=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_github_klauspost_cpuid",
        importpath = "github.com/klauspost/cpuid",
        sum = "h1:NMpwD2G9JSFOE1/TJjGSo5zG7Yb2bTe7eq1jH+irmeE=",
        version = "v1.2.0",
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
        name = "com_github_nwaples_rardecode",
        importpath = "github.com/nwaples/rardecode",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:+HXp/QFE49Q6qJ3xw0rf1owaNcntNr4q+tsHy8qGUdw=",
        version = "v1.1.1",
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
        name = "com_github_pierrec_lz4",
        importpath = "github.com/pierrec/lz4",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:9UY3+iC23yxF0UfGaYrGplQ+79Rg+h/q9FV9ix19jjM=",
        version = "v2.6.1+incompatible",
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
        name = "com_github_rogpeppe_fastuuid",
        importpath = "github.com/rogpeppe/fastuuid",
        sum = "h1:Ppwyp6VYCF1nvBTXL3trRso7mXMlRrw9ooo375wvi2s=",
        version = "v1.2.0",
    )

    go_repository(
        name = "com_github_rogpeppe_go_internal",
        importpath = "github.com/rogpeppe/go-internal",
        sum = "h1:RR9dF3JtopPvtkroDZuVD7qquD0bnHlKSqaQhgwt8yk=",
        version = "v1.3.0",
    )
    go_repository(
        name = "com_github_sergi_go_diff",
        importpath = "github.com/sergi/go-diff",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:XU+rvMAioB0UC3q1MFrIQy4Vo5/4VsRDQQXHsEya6xQ=",
        version = "v1.2.0",
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
        sum = "h1:ohJHjZ+PcaLxDUjqk2NC3tIGsVa5bXThe1ZheSXOjuk=",
        version = "v0.1.0",
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
        sum = "h1:hDPOHmpOpP40lSULcqw7IrRb/u7w6RpDC9399XyoNd0=",
        version = "v1.6.1",
    )
    go_repository(
        name = "com_github_syndtr_goleveldb",
        importpath = "github.com/syndtr/goleveldb",
        sum = "h1:fBdIW9lB4Iz0n9khmH8w27SJ3QEJ7+IgjPEwGSZiFdE=",
        version = "v1.0.0",
    )

    go_repository(
        name = "com_github_ulikunitz_xz",
        importpath = "github.com/ulikunitz/xz",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:t92gobL9l3HE202wg3rlk19F6X+JOxl9BBrCCMYEYd8=",
        version = "v0.5.10",
    )

    go_repository(
        name = "com_github_xi2_xz",
        importpath = "github.com/xi2/xz",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:nIPpBwaJSVYIxUFsDv3M8ofmx9yWTog9BfvIu0q41lo=",
        version = "v0.0.0-20171230120015-48954b6210f8",
    )

    _gazelle_ignore(name = "com_github_yuin_goldmark")

    go_repository(
        name = "com_google_cloud_go",
        importpath = "cloud.google.com/go",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:MjvSkUq8RuAb+2JLDi5VQmmExRJPUQ3JLCWpRB6fmdw=",
        version = "v0.90.0",
    )

    go_repository(
        name = "com_google_cloud_go_bigquery",
        importpath = "cloud.google.com/go/bigquery",
        sum = "h1:PQcPefKFdaIzjQFbiyOgAqyx8q5djaE7x9Sqe712DPA=",
        version = "v1.8.0",
    )
    go_repository(
        name = "com_google_cloud_go_datastore",
        importpath = "cloud.google.com/go/datastore",
        sum = "h1:/May9ojXjRkPBNVrq+oWLqmWCkr4OU5uRY29bu0mRyQ=",
        version = "v1.1.0",
    )
    go_repository(
        name = "com_google_cloud_go_pubsub",
        importpath = "cloud.google.com/go/pubsub",
        sum = "h1:ukjixP1wl0LpnZ6LWtZJ0mX5tBmjp1f8Sqer8Z2OMUU=",
        version = "v1.3.1",
    )
    go_repository(
        name = "com_google_cloud_go_storage",
        importpath = "cloud.google.com/go/storage",
        sum = "h1:1UwAux2OZP4310YXg5ohqBEpV16Y93uZG4+qOX7K2Kg=",
        version = "v1.16.0",
    )
    go_repository(
        name = "com_shuralyov_dmitri_gpu_mtl",
        importpath = "dmitri.shuralyov.com/gpu/mtl",
        sum = "h1:VpgP7xuJadIUuKccphEpTJnWhS2jkQyMt6Y7pJCD7fY=",
        version = "v0.0.0-20190408044501-666a987793e9",
    )
    go_repository(
        name = "in_gopkg_check_v1",
        importpath = "gopkg.in/check.v1",
        sum = "h1:YR8cESwS4TdDjEe65xsg0ogRM/Nc3DYOhEAlW+xobZo=",
        version = "v1.0.0-20190902080502-41f04d3bba15",
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
        sum = "h1:dUUwHk2QECo/6vqA44rthZ8ie2QXMNeKRTHCNY2nXvo=",
        version = "v3.0.0-20200313102051-9f266ea9e77c",
    )

    go_repository(
        name = "io_k8s_sigs_yaml",
        importpath = "sigs.k8s.io/yaml",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:kr/MCeFWJWTwyaHoR9c8EjH9OumOmoF9YGiZd7lFm/Q=",
        version = "v1.2.0",
    )
    go_repository(
        name = "io_opencensus_go",
        importpath = "go.opencensus.io",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:gqCw0LfLxScz8irSi8exQc7fyQ0fKQU/qnC/X8+V/1M=",
        version = "v0.23.0",
    )
    go_repository(
        name = "io_opentelemetry_go_proto_otlp",
        importpath = "go.opentelemetry.io/proto/otlp",
        sum = "h1:rwOQPCuKAKmwGKq2aVNnYIibI6wnV7EvzgfTCzcdGg8=",
        version = "v0.7.0",
    )

    go_repository(
        name = "io_rsc_binaryregexp",
        importpath = "rsc.io/binaryregexp",
        sum = "h1:HfqmD5MEmC0zvwBuF187nq9mdnXjXsSivRiXN7SmRkE=",
        version = "v0.2.0",
    )
    _gazelle_ignore(name = "io_rsc_quote_v3")
    _gazelle_ignore(name = "io_rsc_sampler")

    go_repository(
        name = "org_bitbucket_creachadair_shell",
        importpath = "bitbucket.org/creachadair/shell",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:reJflDbKqnlnqb4Oo2pQ1/BqmY/eCWcNGHrIUO8qIzc=",
        version = "v0.0.6",
    )
    go_repository(
        name = "org_bitbucket_creachadair_stringset",
        importpath = "bitbucket.org/creachadair/stringset",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:L4vld9nzPt90UZNrXjNelTshD74ps4P5NGs3Iq6yN3o=",
        version = "v0.0.9",
    )

    go_repository(
        name = "org_golang_google_api",
        importpath = "google.golang.org/api",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:m5FLEd6dp5CU1F0tMWyqDi2XjchviIz8ntzOSz7w8As=",
        version = "v0.52.0",
    )

    go_repository(
        name = "org_golang_google_appengine",
        importpath = "google.golang.org/appengine",
        sum = "h1:FZR1q0exgwxzPzp/aF+VccGrSfxfPpkBqjIIEq3ru6c=",
        version = "v1.6.7",
    )
    go_repository(
        name = "org_golang_google_genproto",
        importpath = "google.golang.org/genproto",
        sum = "h1:0XmXV/Hi77Rbsx0ADebP/Epagwtf9/OP4FKpu6yZcjU=",
        version = "v0.0.0-20210803142424-70bd63adacf2",
    )
    go_repository(
        name = "org_golang_google_grpc",
        importpath = "google.golang.org/grpc",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Klz8I9kdtkIN6EpHHUOMLCYhTn/2WAe5a0s1hcBkdTI=",
        version = "v1.39.0",
    )
    go_repository(
        name = "org_golang_google_grpc_cmd_protoc_gen_go_grpc",
        importpath = "google.golang.org/grpc/cmd/protoc-gen-go-grpc",
        sum = "h1:M1YKkFIboKNieVO5DLUEVzQfGwJD30Nv2jfUgzb5UcE=",
        version = "v1.1.0",
    )

    go_repository(
        name = "org_golang_google_protobuf",
        importpath = "google.golang.org/protobuf",
        sum = "h1:SnqbnDw1V7RiZcXPx5MEeqPv2s79L9i7BJUlG/+RurQ=",
        version = "v1.27.1",
    )
    go_repository(
        name = "org_golang_x_crypto",
        importpath = "golang.org/x/crypto",
        sum = "h1:psW17arqaxU48Z5kZ0CQnkZWQJsqcURM6tKiBApRjXI=",
        version = "v0.0.0-20200622213623-75b288015ac9",
    )
    go_repository(
        name = "org_golang_x_exp",
        importpath = "golang.org/x/exp",
        sum = "h1:QE6XYQK6naiK1EPAe1g/ILLxN5RBoH5xkJk3CqlMI/Y=",
        version = "v0.0.0-20200224162631-6cc2880d07d6",
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
        sum = "h1:VLliZ0d+/avPrXXH+OakdXhpJuEoBZuwh1m2j7U6Iug=",
        version = "v0.0.0-20210508222113-6edffad5e616",
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
        sum = "h1:Gz96sIWK3OalVv/I/qNygP42zyoKp3xptRVCWRFEBvo=",
        version = "v0.4.2",
    )
    go_repository(
        name = "org_golang_x_net",
        importpath = "golang.org/x/net",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:4CSI6oo7cOjJKajidEljs9h+uP0rRZBPPPhcCbj5mw8=",
        version = "v0.0.0-20210726213435-c6fcb2dbf985",
    )

    go_repository(
        name = "org_golang_x_oauth2",
        importpath = "golang.org/x/oauth2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:3B43BWw0xEBsLZ/NO1VALz6fppU3481pik+2Ksv45z8=",
        version = "v0.0.0-20210628180205-a41e5a781914",
    )

    go_repository(
        name = "org_golang_x_sync",
        importpath = "golang.org/x/sync",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:5KslGYwFpkhGh+Q16bwMP3cOontH8FOep7tGV86Y7SQ=",
        version = "v0.0.0-20210220032951-036812b2e83c",
    )

    go_repository(
        name = "org_golang_x_sys",
        importpath = "golang.org/x/sys",
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        patch_args = ["-p1"],
        version = "v0.0.0-20210630005230-0f9fa26af87c",
        sum = "h1:F1jZWGFhYfh0Ci55sIpILtKKK8p3i2/krTr0H1rg74I=",
    )

    go_repository(
        name = "org_golang_x_term",
        importpath = "golang.org/x/term",
        sum = "h1:v+OssWQX+hTHEmOBgwxdZxK4zHq3yOs8F9J7mk0PY8E=",
        version = "v0.0.0-20201126162022-7de9c90e9dd1",
    )

    go_repository(
        name = "org_golang_x_text",
        importpath = "golang.org/x/text",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:aRYxNxv6iGQlyVaZmk6ZgYEDa+Jg18DxebPSrd6bg1M=",
        version = "v0.3.6",
    )

    go_repository(
        name = "org_golang_x_time",
        importpath = "golang.org/x/time",
        sum = "h1:/5xXl8Y5W96D+TtHSlonuFqGHIWVuyCkGJLwGh9JJFs=",
        version = "v0.0.0-20191024005414-555d28b269f0",
    )

    http_archive(
        name = "org_golang_x_tools",
        # v0.1.8, latest as of 2021-12-15
        urls = [
            "https://mirror.bazel.build/github.com/golang/tools/archive/v0.1.8.zip",
            "https://github.com/golang/tools/archive/v0.1.8.zip",
        ],
        sha256 = "aec8a9ade0974bafc290bad1c53fa2b4d2b87ac8a90bf5340ded216ff81d1b2a",
        strip_prefix = "tools-0.1.8",
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

    go_repository(
        name = "org_golang_x_xerrors",
        importpath = "golang.org/x/xerrors",
        sum = "h1:go1bK/D/BFZV2I8cIQd1NKEZ+0owSTG1fDTci4IqFcE=",
        version = "v0.0.0-20200804184101-5ec99f83aff1",
    )

def _rust_dependencies():
    raze_fetch_remote_crates()

def _js_dependencies():
    npm_install(
        name = "npm",
        package_json = "@io_kythe//:package.json",
        package_lock_json = "@io_kythe//:package-lock.json",
    )

def _bindings():
    maybe(
        native.bind,
        name = "vnames_config",
        actual = "@io_kythe//kythe/data:vnames_config",
    )

    maybe(
        native.bind,
        name = "libuuid",
        actual = "@io_kythe//third_party:libuuid",
    )

    maybe(
        native.bind,
        name = "libmemcached",
        actual = "@org_libmemcached_libmemcached//:libmemcached",
    )

    maybe(
        native.bind,
        name = "guava",  # required by @com_google_protobuf
        actual = "@io_kythe//third_party/guava",
    )

    maybe(
        native.bind,
        name = "gson",  # required by @com_google_protobuf
        actual = "@maven//:com_google_code_gson_gson",
    )

    maybe(
        native.bind,
        name = "zlib",  # required by @com_google_protobuf
        actual = "@net_zlib//:zlib",
    )

    # This binding is needed for protobuf. See https://github.com/protocolbuffers/protobuf/pull/5811
    maybe(
        native.bind,
        name = "error_prone_annotations",
        actual = "@maven//:com_google_errorprone_error_prone_annotations",
    )

def _extractor_image_dependencies():
    """Defines external repositories necessary for extractor images."""
    go_repository(
        name = "com_github_bazelbuild_bazelisk",
        importpath = "github.com/bazelbuild/bazelisk",
        tag = "v1.3.0",
    )
    go_repository(
        name = "com_github_mitchellh_go_homedir",
        importpath = "github.com/mitchellh/go-homedir",
        tag = "v1.1.0",
    )
    go_repository(
        name = "com_github_hashicorp_go_version",
        importpath = "github.com/hashicorp/go-version",
        tag = "v1.1.0",
    )

def _sample_ui_dependencies():
    """Defines external repositories necessary for building the sample UI."""
    lein_repository(
        name = "org_leiningen",
        sha256 = "a0a1f093677045c4e1e40219ccc989acd61433f61c50e098a2185faf4f03553c",
        version = "2.5.3",
    )

def kythe_dependencies(sample_ui = True):
    """Defines external repositories for Kythe dependencies.

    Call this once in your WORKSPACE file to load all @io_kythe dependencies.
    """
    bazel_skylib_workspace()
    _proto_dependencies()
    _cc_dependencies()
    _go_dependencies()
    _java_dependencies()
    _rust_dependencies()
    _js_dependencies()

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    maybe(
        http_archive,
        name = "com_google_protobuf",
        sha256 = "730d43c5460a4448398f06718da075c246eeb16483f2f279b5070f222dabc218",
        strip_prefix = "protobuf-3.18.1",
        urls = [
            "https://mirror.bazel.build/github.com/protocolbuffers/protobuf/archive/v3.18.1.zip",
            "https://github.com/protocolbuffers/protobuf/archive/v3.18.1.zip",
        ],
        repo_mapping = {"@zlib": "@net_zlib"},
    )

    _bindings()
    _rule_dependencies()

    if sample_ui:
        _sample_ui_dependencies()
    _extractor_image_dependencies()
