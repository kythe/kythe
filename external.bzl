load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")
load("@rules_java//java:repositories.bzl", "rules_java_dependencies")
load("@rules_jvm_external//:defs.bzl", "maven_install")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@io_kythe//:setup.bzl", "github_archive", "maybe")
load("@io_kythe//tools:build_rules/shims.bzl", "go_repository")
load("@io_kythe//tools/build_rules/llvm:repo.bzl", "git_llvm_repository")
load("@io_kythe//third_party/leiningen:lein_repo.bzl", "lein_repository")
load("@io_kythe//tools/build_rules/lexyacc:lexyacc.bzl", "lexyacc_configure")
load("@io_kythe//tools/build_rules/build_event_stream:repo.bzl", "build_event_stream_repository")
load("@io_kythe//kythe/cxx/extractor:toolchain.bzl", cxx_extractor_register_toolchains = "register_toolchains")
load("@rules_python//python:repositories.bzl", "py_repositories")
load("@bazel_toolchains//repositories:repositories.bzl", bazel_toolchains_repositories = "repositories")
load("@io_bazel_rules_rust//rust:repositories.bzl", "rust_repositories")
load("@io_bazel_rules_rust//proto:repositories.bzl", "rust_proto_repositories")
load("@io_bazel_rules_rust//:workspace.bzl", "bazel_version")
load("@build_bazel_rules_nodejs//:index.bzl", "npm_install")

# The raze macros automatically check for duplicated dependencies so we can
# simply load each macro here.
load("//kythe/rust/cargo:crates.bzl", "raze_fetch_remote_crates")

def _rule_dependencies():
    go_rules_dependencies()
    go_register_toolchains()
    gazelle_dependencies()
    rules_java_dependencies()
    rules_proto_dependencies()
    py_repositories()
    bazel_toolchains_repositories()
    rust_repositories(version = "nightly", iso_date = "2020-06-23", dev_components = True)
    rust_proto_repositories()
    bazel_version(name = "bazel_version")

def _gazelle_ignore(**kwargs):
    """Dummy macro which causes gazelle to see a repository as already defined."""

def _proto_dependencies():
    # Rather than pull down the entire Bazel source repository for a single file,
    # just grab the file we need and use it locally.
    maybe(
        build_event_stream_repository,
        name = "build_event_stream_proto",
        revision = "2.2.0",
        sha256s = {
            "build_event_stream.proto": "aa71ad693b7b474517ee3702318603d76baef35a6c13e9f8980f3962d91c2827",
            "command_line.proto": "a6fb6591aa50794431787169bc4fae16105ef5c401e7c30ecf0f775e0ab25c2c",
            "invocation_policy.proto": "5312a440a5d16e9bd72cd8561ad2f5d2b29579f19df7e13af1517c6ad9e7fa64",
            "option_filters.proto": "e3e8dfa9a4e05683bf1853a0be29fae46c753b18ad3d42b92bedcb412577f20f",
        },
    )

def _cc_dependencies():
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
        sha256 = "a5d22f0c87a2625450eaa5e10db18b8ee4ef17042102d04c62e311993a2ba363",
        strip_prefix = "libzip-rel-1-5-1",
        urls = [
            # Bazel does not like the official download link at libzip.org,
            # so use the GitHub release tag.
            "https://mirror.bazel.build/github.com/nih-at/libzip/archive/rel-1-5-1.zip",
            "https://github.com/nih-at/libzip/archive/rel-1-5-1.zip",
        ],
    )

    maybe(
        git_repository,
        name = "boringssl",
        # Use the github mirror because the official source at
        # https://boringssl.googlesource.com/boringssl does not allow
        # unauthenticated git clone and the archives suffer from
        # https://github.com/google/gitiles/issues/84 preventing the use of
        # sha256sum on archives.
        remote = "https://github.com/google/boringssl",
        # Commits must come from the master-with-bazel branch.
        # branch = "master-with-bazel",
        commit = "e0c35d6c06fd800de1092f0b4d4326570ca2617a",
        shallow_since = "1566966435 +0000",
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
        commit = "0e9921b75a0fdd639a504ec8443fc1fe801becd7",
        sha256 = "8061df0ebbd3f599bcd3f5e57fb8003564d50a9b6a81a7f968fb0196b952365d",
    )

    maybe(
        github_archive,
        name = "com_google_googletest",
        repo_name = "google/googletest",
        sha256 = "4a4cbf4bb09606f42a0cdd6f0893fbf1e257243fda64bc5b585d027808a3a64b",
        commit = "61f010d703b32de9bfb20ab90ece38ab2f25977f",
    )

    maybe(
        http_archive,
        name = "com_github_google_glog",
        strip_prefix = "glog-ba8a9f6952d04d1403b97df24e6836227751454e",
        sha256 = "9b4867ab66c33c41e2672b5de7e3133d38411cdb75eeb0d2b72c88bb10375c71",
        urls = [
            "https://mirror.bazel.build/github.com/google/glog/archive/ba8a9f6952d04d1403b97df24e6836227751454e.zip",
            "https://github.com/google/glog/archive/ba8a9f6952d04d1403b97df24e6836227751454e.zip",
        ],
        build_file_content = "\n".join([
            "load(\"//:bazel/glog.bzl\", \"glog_library\")",
            "glog_library(with_gflags=0)",
        ]),
    )

    maybe(
        http_archive,
        name = "org_brotli",
        sha256 = "4c61bfb0faca87219ea587326c467b95acb25555b53d1a421ffa3c8a9296ee2c",
        strip_prefix = "brotli-1.0.7",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party:brotli/brotli-1.0.7-int-float-conversion.patch",
        ],
        urls = [
            "https://mirror.bazel.build/github.com/google/brotli/archive/v1.0.7.tar.gz",
            "https://github.com/google/brotli/archive/v1.0.7.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "com_google_riegeli",
        sha256 = "762b838bcf3ddc02e1b334103ef21f02316e57be373444ff7c7461781935c8b6",
        strip_prefix = "riegeli-a624e7f8e98aff394904685ecbba2e5ee664606a",
        urls = [
            "https://mirror.bazel.build/github.com/google/riegeli/archive/a624e7f8e98aff394904685ecbba2e5ee664606a.zip",
            "https://github.com/google/riegeli/archive/a624e7f8e98aff394904685ecbba2e5ee664606a.zip",
        ],
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
        http_archive,
        name = "com_googlesource_code_re2",
        sha256 = "ae9b962dbd6427565efd3e9503acb40a1385b21962c29050546c9347ac7fa93f",
        strip_prefix = "re2-2019-01-01",
        urls = [
            "https://mirror.bazel.build/github.com/google/re2/archive/2019-01-01.zip",
            "https://github.com/google/re2/archive/2019-01-01.zip",
        ],
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
        http_archive,
        name = "com_github_google_snappy",
        build_file = "@io_kythe//third_party:snappy.BUILD",
        sha256 = "61e05a0295fd849072668b1f3494801237d809427cfe8fd014cda455036c3ef7",
        strip_prefix = "snappy-1.1.7",
        urls = [
            "https://mirror.bazel.build/github.com/google/snappy/archive/1.1.7.zip",
            "https://github.com/google/snappy/archive/1.1.7.zip",
        ],
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
        git_llvm_repository,
        name = "org_llvm",
    )

    lexyacc_configure()
    cxx_extractor_register_toolchains()

def _java_dependencies():
    maybe(
        # For @com_google_common_flogger
        http_archive,
        name = "google_bazel_common",
        strip_prefix = "bazel-common-b3778739a9c67eaefe0725389f03cf821392ac67",
        sha256 = "4ae0fd0af627be9523a166b88d1298375335f418dcc13a82e9e77a0089a4d254",
        urls = [
            "https://mirror.bazel.build/github.com/google/bazel-common/archive/b3778739a9c67eaefe0725389f03cf821392ac67.zip",
            "https://github.com/google/bazel-common/archive/b3778739a9c67eaefe0725389f03cf821392ac67.zip",
        ],
    )
    maybe(
        git_repository,
        name = "com_google_common_flogger",
        commit = "ca8ad22bc1479b5675118308f88ef3fff7d26c1f",
        remote = "https://github.com/google/flogger",
    )
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
            "com.google.jimfs:jimfs:1.1",
            "com.google.re2j:re2j:1.2",
            "com.google.truth:truth:1.0",
            "com.googlecode.java-diff-utils:diffutils:1.3.0",
            "org.apache.tomcat:tomcat-annotations-api:9.0.34",
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
        generate_compat_repositories = True,  # Required by bazel-common's dependencies
        version_conflict_policy = "pinned",
    )

def _go_dependencies():
    go_repository(
        name = "com_github_golang_protobuf",
        build_file_proto_mode = "disable_global",
        importpath = "github.com/golang/protobuf",
        patch_args = ["-p1"],
        patches = [
            "@io_bazel_rules_go//third_party:com_github_golang_protobuf-extras.patch",
            "@io_kythe//third_party/go:new_export_license.patch",
        ],
        sum = "h1:ZFgWrT+bLgsYPirOnRfKLYJLvssAegOj/hgyMFdJZe0=",
        version = "v1.4.1",
    )

    go_repository(
        name = "com_github_google_uuid",
        importpath = "github.com/google/uuid",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Gkbcsh/GbpXz7lPftLA3P6TYMwjCLYm83jiFQZF/3gY=",
        version = "v1.1.1",
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
        name = "com_github_google_go_cmp",
        importpath = "github.com/google/go-cmp",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:xsAVV57WRhGj6kEIi8ReJzQlHHqcBYCElAvkovg3B/4=",
        version = "v0.4.0",
    )

    go_repository(
        name = "org_golang_x_sync",
        importpath = "golang.org/x/sync",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:vcxGaoTs7kV8m5Np9uUNQin4BrLOthgV7252N8V+FwY=",
        version = "v0.0.0-20190911185100-cd5d95a43a6e",
    )

    go_repository(
        name = "com_github_sourcegraph_jsonrpc2",
        importpath = "github.com/sourcegraph/jsonrpc2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:5VGNYxMxzZ8Jb2bARgVl1DNg8vpcd9S8b4MbbjWQ8/w=",
        version = "v0.0.0-20191222043438-96c4efab7ee2",
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
        name = "com_github_golang_snappy",
        importpath = "github.com/golang/snappy",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Qgr9rKW7uDUkrbSmQeiDsGa8SjGyCOGtuasMWwvp2P4=",
        version = "v0.0.1",
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
        name = "com_github_sergi_go_diff",
        importpath = "github.com/sergi/go-diff",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:we8PVUC3FE2uYfodKH/nBHMSetSfHDR6scGdBi+erh0=",
        version = "v1.1.0",
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

    http_archive(
        name = "org_golang_x_tools",
        urls = [
            "https://mirror.bazel.build/github.com/golang/tools/archive/2bc93b1c0c88b2406b967fcd19a623d1ff9ea0cd.zip",
            "https://github.com/golang/tools/archive/2bc93b1c0c88b2406b967fcd19a623d1ff9ea0cd.zip",
        ],
        sha256 = "b05c5b5b9091a35ecb433227ea30aa75cb6b9d9409b308bc75d0975d4a291912",
        strip_prefix = "tools-2bc93b1c0c88b2406b967fcd19a623d1ff9ea0cd",
        patches = [
            # deletegopls removes the gopls subdirectory. It contains a nested
            # module with additional dependencies. It's not needed by rules_go.
            "@io_bazel_rules_go//third_party:org_golang_x_tools-deletegopls.patch",
            # gazelle args: -repo_root . -go_prefix golang.org/x/tools
            "@io_bazel_rules_go//third_party:org_golang_x_tools-gazelle.patch",
            # extras adds go_tool_library rules for packages under
            # go/analysis/passes and their dependencies. These are needed by
            # nogo.
            "@io_bazel_rules_go//third_party:org_golang_x_tools-extras.patch",
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        patch_args = ["-p1"],
    )

    go_repository(
        name = "org_golang_x_text",
        importpath = "golang.org/x/text",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:tW2bmiBqwgJj/UpqtC8EpXEZVYOwU0yG4iWbprSVAcs=",
        version = "v0.3.2",
    )

    go_repository(
        name = "org_golang_x_net",
        importpath = "golang.org/x/net",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:GuSPYbZzB5/dcLNCwLQLsg3obCJtX9IJhpXkvY7kzk0=",
        version = "v0.0.0-20200301022130-244492dfa37a",
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
        name = "org_bitbucket_creachadair_stringset",
        importpath = "bitbucket.org/creachadair/stringset",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:gQqe4vs8XWgMyijfyKE6K8o4TcyGGrRXe0JvHgx5H+M=",
        version = "v0.0.8",
    )

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
        name = "org_golang_google_grpc",
        importpath = "google.golang.org/grpc",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:bO/TA4OxCOummhSf10siHuG7vJOiwh7SpRpFZDkOgl4=",
        version = "v1.28.0",
    )

    go_repository(
        name = "org_golang_x_oauth2",
        importpath = "golang.org/x/oauth2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:TzXSXBo42m9gQenoE3b9BGiEpg5IG2JkU5FkPIawgtw=",
        version = "v0.0.0-20200107190931-bf48bf16ab8d",
    )

    go_repository(
        name = "com_github_apache_beam",
        build_file_proto_mode = "disable",
        importpath = "github.com/apache/beam",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:Dm4cSxkGeH8l9OG2jkOodp2Dg6uZjorV0XU8vDy/fa4=",
        version = "v2.19.0+incompatible",
    )

    go_repository(
        name = "org_golang_google_api",
        importpath = "google.golang.org/api",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:jz2KixHX7EcCPiQrySzPdnYT7DbINAypCqKZ1Z7GM40=",
        version = "v0.20.0",
    )

    go_repository(
        name = "com_google_cloud_go",
        importpath = "cloud.google.com/go",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:3ithwDMr7/3vpAMXiH+ZQnYbuIsh+OPhUPMFC9enmn0=",
        version = "v0.54.0",
    )

    go_repository(
        name = "io_opencensus_go",
        importpath = "go.opencensus.io",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:8sGtKOrtQqkN1bp2AtX+misvLIlOmsEsNd+9NIcPEm8=",
        version = "v0.22.3",
    )

    go_repository(
        name = "com_github_syndtr_goleveldb",
        importpath = "github.com/syndtr/goleveldb",
        sum = "h1:fBdIW9lB4Iz0n9khmH8w27SJ3QEJ7+IgjPEwGSZiFdE=",
        version = "v1.0.0",
    )

    go_repository(
        name = "com_github_minio_highwayhash",
        importpath = "github.com/minio/highwayhash",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:iMSDhgUILCr0TNm8LWlSjF8N0ZIj2qbO8WHp6Q/J2BA=",
        version = "v1.0.0",
    )

    go_repository(
        name = "org_golang_x_sys",
        importpath = "golang.org/x/sys",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:uYVVQ9WP/Ds2ROhcaGPeIdVq0RIXVLwsHlnvJ+cT1So=",
        version = "v0.0.0-20200302150141-5c8b2ff67527",
    )

    go_repository(
        name = "com_github_datadog_zstd",
        importpath = "github.com/DataDog/zstd",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:+IawcoXhCBylN7ccwdwf8LOH2jKq7NavGpEPanrlTzE=",
        version = "v1.4.4",
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
        name = "com_github_google_orderedcode",
        importpath = "github.com/google/orderedcode",
        sum = "h1:UzfcAexk9Vhv8+9pNOgRu41f16lHq725vPwnSeiG/Us=",
        version = "v0.0.1",
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
        name = "in_gopkg_yaml_v2",
        importpath = "gopkg.in/yaml.v2",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:obN1ZagJSUGI0Ek/LBmuj4SNLPfIny3KsKFopxRdj10=",
        version = "v2.2.8",
    )

    go_repository(
        name = "com_github_mholt_archiver",
        importpath = "github.com/mholt/archiver",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:1dCVxuqs0dJseYEhi5pl7MYPH9zDa1wBi7mF09cbNkU=",
        version = "v3.1.1+incompatible",
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
        name = "com_github_nwaples_rardecode",
        importpath = "github.com/nwaples/rardecode",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:vSxaY8vQhOcVr4mm5e8XllHWTiM4JF507A0Katqw7MQ=",
        version = "v1.1.0",
    )

    go_repository(
        name = "com_github_pierrec_lz4",
        importpath = "github.com/pierrec/lz4",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:mFe7ttWaflA46Mhqh+jUfjp2qTbPYxLB2/OyBppH9dg=",
        version = "v2.4.1+incompatible",
    )

    go_repository(
        name = "com_github_ulikunitz_xz",
        importpath = "github.com/ulikunitz/xz",
        patch_args = ["-p1"],
        patches = [
            "@io_kythe//third_party/go:add_export_license.patch",
        ],
        sum = "h1:YvTNdFzX6+W5m9msiYg/zpkSURPPtOlzbqYjrFn7Yt4=",
        version = "v0.5.7",
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

    maybe(
        http_archive,
        name = "org_brotli_go",
        sha256 = "4c61bfb0faca87219ea587326c467b95acb25555b53d1a421ffa3c8a9296ee2c",
        strip_prefix = "brotli-1.0.7/go",
        urls = [
            "https://mirror.bazel.build/github.com/google/brotli/archive/v1.0.7.tar.gz",
            "https://github.com/google/brotli/archive/v1.0.7.tar.gz",
        ],
    )
    _gazelle_ignore(
        name = "com_github_bazelbuild_rules_go",
        actual = "io_bazel_rules_go",
        importpath = "github.com/bazelbuild/rules_go",
    )
    _gazelle_ignore(
        name = "com_github_google_brotli",
        actual = "org_brotli_go",
        importpath = "github.com/google/brotli",
    )
    go_repository(
        name = "co_honnef_go_tools",
        importpath = "honnef.co/go/tools",
        sum = "h1:sXmLre5bzIR6ypkjXCDI3jHPssRhc8KD/Ome589sc3U=",
        version = "v0.0.1-2020.1.3",
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
    go_repository(
        name = "com_github_client9_misspell",
        importpath = "github.com/client9/misspell",
        sum = "h1:ta993UF76GwbvJcIo3Y68y/M3WxlpEHPWIGDkJYwzJI=",
        version = "v0.3.4",
    )
    go_repository(
        name = "com_github_creachadair_staticfile",
        importpath = "github.com/creachadair/staticfile",
        sum = "h1:QG0u27/Ietu0UVOk1aMbF6jrWrEzPIdZP4ju3c1PPfY=",
        version = "v0.1.2",
    )
    go_repository(
        name = "com_github_davecgh_go_spew",
        importpath = "github.com/davecgh/go-spew",
        sum = "h1:vj9j/u1bqnvCEfJOwUhtlOARqs3+rkHYY13jYWTU97c=",
        version = "v1.1.1",
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
        sum = "h1:rEvIZUSZ3fx39WIi3JkQqQBitGwpELBIYWeBVh6wn+E=",
        version = "v0.9.4",
    )
    go_repository(
        name = "com_github_envoyproxy_protoc_gen_validate",
        importpath = "github.com/envoyproxy/protoc-gen-validate",
        sum = "h1:EQciDnbrYxy13PgWoY8AqoxGiPrpgBZ1R8UNe3ddc+A=",
        version = "v0.1.0",
    )
    go_repository(
        name = "com_github_fsnotify_fsnotify",
        importpath = "github.com/fsnotify/fsnotify",
        sum = "h1:IXs+QLmnXW2CcXuY+8Mzv/fWEsPGWxqefPtCP5CnV9I=",
        version = "v1.4.7",
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
        sum = "h1:1r7pUrabqp18hOBcwBwiTsbnFeTZHV9eER/QT5JVZxY=",
        version = "v0.0.0-20200121045136-8c9f03a8e57e",
    )

    go_repository(
        name = "com_github_golang_mock",
        importpath = "github.com/golang/mock",
        sum = "h1:ocYkMQY5RrXTYgXl7ICpV0IXwlEQGwKIsery4gyXa1U=",
        version = "v1.4.1",
    )
    go_repository(
        name = "com_github_google_btree",
        importpath = "github.com/google/btree",
        sum = "h1:0udJVsspx3VBr5FwtLhQQtuAsVc79tTq0ocGIPAU6qo=",
        version = "v1.0.0",
    )
    go_repository(
        name = "com_github_google_martian",
        importpath = "github.com/google/martian",
        sum = "h1:/CP5g8u/VJHijgedC/Legn3BAbAaWPgecwXBIDzw5no=",
        version = "v2.1.0+incompatible",
    )
    go_repository(
        name = "com_github_google_pprof",
        importpath = "github.com/google/pprof",
        sum = "h1:SRgJV+IoxM5MKyFdlSUeNy6/ycRUF2yBAKdAQswoHUk=",
        version = "v0.0.0-20200229191704-1ebb73c60ed3",
    )
    go_repository(
        name = "com_github_google_renameio",
        importpath = "github.com/google/renameio",
        sum = "h1:GOZbcHa3HfsPKPlmyPyN2KEohoMXOhdMbHrvbpl2QaA=",
        version = "v0.1.0",
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
        name = "com_github_rogpeppe_go_internal",
        importpath = "github.com/rogpeppe/go-internal",
        sum = "h1:RR9dF3JtopPvtkroDZuVD7qquD0bnHlKSqaQhgwt8yk=",
        version = "v1.3.0",
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
        sum = "h1:2E4SXV/wtOkTonXsotYi4li6zVWxYlZuYNCXe9XRJyk=",
        version = "v1.4.0",
    )
    go_repository(
        name = "com_google_cloud_go_bigquery",
        importpath = "cloud.google.com/go/bigquery",
        sum = "h1:xE3CPsOgttP4ACBePh79zTKALtXwn/Edhcr16R5hMWU=",
        version = "v1.4.0",
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
        sum = "h1:Lpy6hKgdcl7a3WGSfJIFmxmcdjSpP6OmBEfcOv1Y680=",
        version = "v1.2.0",
    )
    go_repository(
        name = "com_google_cloud_go_storage",
        importpath = "cloud.google.com/go/storage",
        sum = "h1:UDpwYIwla4jHGzZJaEJYx1tOejbgSoNqsAfHAUYe2r8=",
        version = "v1.6.0",
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
        name = "io_rsc_binaryregexp",
        importpath = "rsc.io/binaryregexp",
        sum = "h1:HfqmD5MEmC0zvwBuF187nq9mdnXjXsSivRiXN7SmRkE=",
        version = "v0.2.0",
    )
    go_repository(
        name = "org_golang_google_appengine",
        importpath = "google.golang.org/appengine",
        sum = "h1:tycE03LOZYQNhDpS27tcQdAzLCVMaj7QT2SXxebnpCM=",
        version = "v1.6.5",
    )
    go_repository(
        name = "org_golang_google_genproto",
        importpath = "google.golang.org/genproto",
        sum = "h1:pyQjO6BnPvrPMldYxgDlXq9PLahtc0EKnUTYX1pWwXU=",
        version = "v0.0.0-20200313141609-30c55424f95d",
    )
    go_repository(
        name = "org_golang_x_crypto",
        importpath = "golang.org/x/crypto",
        sum = "h1:ObdrDkeb4kJdCP557AjRjq69pTHfNouLtWZG7j9rPN8=",
        version = "v0.0.0-20191011191535-87dc89f01550",
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
        sum = "h1:KU7oHjnv3XNWfa5COkzUifxZmxp1TyI7ImMXqFxLwvQ=",
        version = "v0.2.0",
    )
    go_repository(
        name = "org_golang_x_time",
        importpath = "golang.org/x/time",
        sum = "h1:/5xXl8Y5W96D+TtHSlonuFqGHIWVuyCkGJLwGh9JJFs=",
        version = "v0.0.0-20191024005414-555d28b269f0",
    )
    go_repository(
        name = "org_golang_x_xerrors",
        importpath = "golang.org/x/xerrors",
        sum = "h1:E7g+9GITq07hpfrRu66IVDexMakfv52eLZ2CXBWiKr4=",
        version = "v0.0.0-20191204190536-9bdfabe68543",
    )
    go_repository(
        name = "com_github_frankban_quicktest",
        importpath = "github.com/frankban/quicktest",
        sum = "h1:2QxQoC1TS09S7fhCPsrvqYdvP1H5M1P1ih5ABm3BTYk=",
        version = "v1.7.2",
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
        sha256 = "1c744a6a1f2c901e68c5521bc275e22bdc66256eeb605c2781923365b7087e5f",
        strip_prefix = "protobuf-3.13.0",
        urls = [
            "https://mirror.bazel.build/github.com/protocolbuffers/protobuf/archive/v3.13.0.zip",
            "https://github.com/protocolbuffers/protobuf/archive/v3.13.0.zip",
        ],
        repo_mapping = {"@zlib": "@net_zlib"},
    )

    maybe(
        http_archive,
        name = "io_kythe_llvmbzlgen",
        sha256 = "6d077cfe818d08ea9184d71f73581135b69c379692771afd88392fa1fee018ac",
        urls = [
            "https://mirror.bazel.build/github.com/kythe/llvmbzlgen/archive/435bad1d07f7a8d32979d66cd5547e1b32dca812.zip",
            "https://github.com/kythe/llvmbzlgen/archive/435bad1d07f7a8d32979d66cd5547e1b32dca812.zip",
        ],
        strip_prefix = "llvmbzlgen-435bad1d07f7a8d32979d66cd5547e1b32dca812",
    )

    _bindings()
    _rule_dependencies()

    if sample_ui:
        _sample_ui_dependencies()
    _extractor_image_dependencies()
