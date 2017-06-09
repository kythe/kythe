workspace(name = "io_kythe")

load("//:version.bzl", "check_version")

# Check that the user has a version between our minimum supported version of
# Bazel and our maximum supported version of Bazel.
check_version("0.4.5", "0.5.1")

load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

load("//tools/build_rules/config:system.bzl", "cc_system_package")

cc_system_package(
    name = "libuuid",
    default = "/usr/local/opt/ossp-uuid",
    envvar = "UUID_HOME",
    modname = "uuid",
)

new_http_archive(
    name = "org_libmemcached_libmemcached",
    build_file = "third_party/libmemcached.BUILD",
    sha256 = "e22c0bb032fde08f53de9ffbc5a128233041d9f33b5de022c0978a2149885f82",
    strip_prefix = "libmemcached-1.0.18",
    url = "https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz",
)

bind(
    name = "libmemcached",
    actual = "@org_libmemcached_libmemcached//:libmemcached",
)

http_archive(
    name = "boringssl",  # Must match upstream workspace name.
    # Gitiles creates gzip files with an embedded timestamp, so we cannot use
    # sha256 to validate the archives.  We must rely on the commit hash and https.
    # Commits must come from the master-with-bazel branch.
    url = "https://boringssl.googlesource.com/boringssl/+archive/9612e1d2ce16a1bd67fbbe6ce969839af4d84a29.tar.gz",
)

new_http_archive(
    name = "com_github_google_googletest",
    build_file = "third_party/googletest.BUILD",
    sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
    strip_prefix = "googletest-release-1.8.0",
    url = "https://github.com/google/googletest/archive/release-1.8.0.zip",
)

new_http_archive(
    name = "com_github_gflags_gflags",
    build_file = "third_party/googleflags.BUILD",  # Upstream's BUILD file doesn't quite work.
    sha256 = "659de3fab5ba5a0376e3c2da333e4ecec9c8a4b41709861765e28e02dd562f7a",
    strip_prefix = "gflags-cce68f0c9c5d054017425e6e6fd54f696d36e8ee",
    url = "https://github.com/gflags/gflags/archive/cce68f0c9c5d054017425e6e6fd54f696d36e8ee.zip",
)

git_repository(
    name = "com_googlesource_code_re2",
    commit = "fc6337a382bfd4f7c861abea08f872d3c85b31da",
    remote = "https://code.googlesource.com/re2",
)

new_http_archive(
    name = "com_github_google_glog",
    build_file = "third_party/googlelog.BUILD",
    sha256 = "8fd1eca8e8e24d7240a106cf8183221f5319b6b7b69bcc1bb5f3826ade2bb4cd",
    strip_prefix = "glog-cf36dabd8e24469c1b16748711f38c0d08085b36",
    url = "https://github.com/google/glog/archive/cf36dabd8e24469c1b16748711f38c0d08085b36.zip",
)

maven_jar(
    name = "com_google_code_gson_gson",
    artifact = "com.google.code.gson:gson:2.8.0",
    sha1 = "c4ba5371a29ac9b2ad6129b1d39ea38750043eff",
)

maven_jar(
    name = "com_google_guava_guava",
    artifact = "com.google.guava:guava:21.0",
    sha1 = "3a3d111be1be1b745edfa7d91678a12d7ed38709",
)

maven_jar(
    name = "junit_junit",
    artifact = "junit:junit:4.12",
    sha1 = "2973d150c0dc1fefe998f834810d68f278ea58ec",
)

maven_jar(
    name = "com_google_re2j_re2j",
    artifact = "com.google.re2j:re2j:1.1",
    sha1 = "d716952ab58aa4369ea15126505a36544d50a333",
)

maven_jar(
    name = "com_beust_jcommander",
    artifact = "com.beust:jcommander:1.48",
    sha1 = "bfcb96281ea3b59d626704f74bc6d625ff51cbce",
)

maven_jar(
    name = "com_google_truth_truth",
    artifact = "com.google.truth:truth:0.27",
    sha1 = "bd17774d2dc0fffa884d42c07d2537e86c67acd6",
)

maven_jar(
    name = "io_netty_netty_all",
    artifact = "io.netty:netty-all:4.1.3.Final",
    sha1 = "5304532edd11da8ab899baeab80aaf36ccf89d6e",
)

maven_jar(
    name = "io_grpc_grpc_core",
    artifact = "io.grpc:grpc-core:1.0.3",
    sha1 = "104435f03a23f5680a392a9586d03995e45b9d35",
)

maven_jar(
    name = "io_grpc_grpc_context",
    artifact = "io.grpc:grpc-context:1.0.3",
    sha1 = "fc29af09029527ac52bc9bb97ef27a645709aeea",
)

maven_jar(
    name = "io_grpc_grpc_stub",
    artifact = "io.grpc:grpc-stub:1.0.3",
    sha1 = "4af1464041224cdef2efbeafbe948f942a3fb028",
)

maven_jar(
    name = "io_grpc_grpc_netty",
    artifact = "io.grpc:grpc-netty:1.0.3",
    sha1 = "d473fe54759e38cd2e88e2994817421e193adebe",
)

maven_jar(
    name = "io_grpc_grpc_protobuf",
    artifact = "io.grpc:grpc-protobuf:1.0.3",
    sha1 = "330c3da988bdc9bf336218ae12b2b0ee86767775",
)

maven_jar(
    name = "io_grpc_grpc_protobuf_lite",
    artifact = "io.grpc:grpc-protobuf-lite:1.0.3",
    sha1 = "016a1669179ab06c7b74d3b1c5959b28f1caa6a1",
)

maven_jar(
    name = "com_google_code_findbugs_jsr305",
    artifact = "com.google.code.findbugs:jsr305:3.0.1",
    sha1 = "f7be08ec23c21485b9b5a1cf1654c2ec8c58168d",
)

maven_jar(
    name = "com_google_auto_value",
    artifact = "com.google.auto.value:auto-value:1.4.1",
    sha1 = "8172ebbd7970188aff304c8a420b9f17168f6f48",
)

git_repository(
    name = "io_bazel_rules_go",
    remote = "https://github.com/bazelbuild/rules_go.git",
    tag = "0.4.3",
)

load("@io_bazel_rules_go//go:def.bzl", "go_repositories")

go_repositories(go_version = "1.8.1")

new_git_repository(
    name = "go_errors",
    build_file = "third_party/go/errors.BUILD",
    commit = "ff09b135c25aae272398c51a07235b90a75aa4f0",
    remote = "https://github.com/pkg/errors.git",
)

new_git_repository(
    name = "go_gogo_protobuf",
    build_file = "third_party/go/gogo_protobuf.BUILD",
    commit = "f9114dace7bd920b32f943b3c73fafbcbab2bf31",
    remote = "https://github.com/gogo/protobuf.git",
)

new_git_repository(
    name = "go_gcloud",
    build_file = "third_party/go/gcloud.BUILD",
    commit = "fe3d41e1ecb2ce36ad3a979037c9b9a2b726226f",
    remote = "https://github.com/GoogleCloudPlatform/google-cloud-go.git",
)

new_git_repository(
    name = "go_x_net",
    build_file = "third_party/go/x_net.BUILD",
    commit = "de35ec43e7a9aabd6a9c54d2898220ea7e44de7d",
    remote = "https://github.com/golang/net.git",
)

new_git_repository(
    name = "go_x_text",
    build_file = "third_party/go/x_text.BUILD",
    commit = "d5d7737684e596dbabf914ecf946d2783f35bdc2",
    remote = "https://github.com/golang/text.git",
)

new_git_repository(
    name = "go_x_tools",
    build_file = "third_party/go/x_tools.BUILD",
    commit = "5682db0e919ed9cfc6f52ac32e170511a106eb3b",
    remote = "https://go.googlesource.com/tools",
)

new_git_repository(
    name = "go_x_oauth2",
    build_file = "third_party/go/oauth2.BUILD",
    commit = "3c3a985cb79f52a3190fbc056984415ca6763d01",
    remote = "https://github.com/golang/oauth2.git",
)

new_git_repository(
    name = "go_gapi",
    build_file = "third_party/go/gapi.BUILD",
    commit = "0637df23b94dd27d09659ae7d9052b6c8d6fc1a0",
    remote = "https://github.com/google/google-api-go-client.git",
)

new_git_repository(
    name = "go_subcommands",
    build_file = "third_party/go/subcommands.BUILD",
    commit = "ce3d4cfc062faac7115d44e5befec8b5a08c3faa",
    remote = "https://github.com/google/subcommands.git",
)

new_git_repository(
    name = "go_grpc",
    build_file = "third_party/go/grpc.BUILD",
    commit = "cb653e4b6150b81ba5157618b57c6f910a6a99f7",
    remote = "https://github.com/grpc/grpc-go.git",
)

new_git_repository(
    name = "go_shell",
    build_file = "third_party/go/shell.BUILD",
    commit = "4e4a4403205db46f1ef0590e98dc814a38d2ea63",
    remote = "https://bitbucket.org/creachadair/shell.git",
)

new_git_repository(
    name = "go_stringset",
    build_file = "third_party/go/stringset.BUILD",
    commit = "cca3c3baee12304723dbf81064dfe387b8e4f61f",
    remote = "https://bitbucket.org/creachadair/stringset.git",
)

new_git_repository(
    name = "go_pq",
    build_file = "third_party/go/pq.BUILD",
    commit = "4dd446efc17690bc53e154025146f73203b18309",
    remote = "https://github.com/lib/pq.git",
)

new_git_repository(
    name = "go_diff",
    build_file = "third_party/go/diff.BUILD",
    commit = "ec7fdbb58eb3e300c8595ad5ac74a5aa50019cc7",
    remote = "https://github.com/sergi/go-diff.git",
)

new_git_repository(
    name = "go_uuid",
    build_file = "third_party/go/uuid.BUILD",
    commit = "c55201b036063326c5b1b89ccfe45a184973d073",
    remote = "https://github.com/pborman/uuid.git",
)

new_git_repository(
    name = "go_snappy",
    build_file = "third_party/go/snappy.BUILD",
    commit = "d9eb7a3d35ec988b8585d4a0068e462c27d28380",
    remote = "https://github.com/golang/snappy.git",
)

new_git_repository(
    name = "go_protobuf",
    build_file = "third_party/go/protobuf.BUILD",
    commit = "8ee79997227bf9b34611aee7946ae64735e6fd93",
    remote = "https://github.com/golang/protobuf.git",
)

new_git_repository(
    name = "go_levigo",
    build_file = "third_party/go/levigo.BUILD",
    commit = "1ddad808d437abb2b8a55a950ec2616caa88969b",
    remote = "https://github.com/jmhodges/levigo.git",
)
