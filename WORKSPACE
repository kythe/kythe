workspace(name = "io_kythe")

load("//:version.bzl", "check_version")

check_version("0.3.1")

load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

load("//tools/build_rules/config:system.bzl", "cc_system_package")

cc_system_package(
    name = "libuuid",
    default = "/usr/local/opt/ossp-uuid",
    envvar = "UUID_HOME",
    modname = "uuid",
)

cc_system_package(
    name = "libmemcached",
    default = "/usr/local/opt/libmemcached",
    envvar = "MEMCACHED_HOME",
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
    artifact = "com.google.code.gson:gson:2.7",
    sha1 = "751f548c85fa49f330cecbb1875893f971b33c4e",
)

maven_jar(
    name = "com_google_guava_guava",
    artifact = "com.google.guava:guava:19.0",
    sha1 = "6ce200f6b23222af3d8abb6b6459e6c44f4bb0e9",
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
    artifact = "io.grpc:grpc-core:1.0.1",
    sha1 = "dce1c939c2c6110ac571d99f8d2a29b19bdad4db",
)

maven_jar(
    name = "io_grpc_grpc_context",
    artifact = "io.grpc:grpc-context:1.0.1",
    sha1 = "9d308f2b616044ddd380866b4e6c23b5b4020963",
)

maven_jar(
    name = "io_grpc_grpc_stub",
    artifact = "io.grpc:grpc-stub:1.0.1",
    sha1 = "a875969bf700b0d25dc8b7febe42bfb253ca5b3b",
)

maven_jar(
    name = "io_grpc_grpc_netty",
    artifact = "io.grpc:grpc-netty:1.0.1",
    sha1 = "1e4628b96434fcd6fbe519e7a3dbcc1ec5ac2c14",
)

maven_jar(
    name = "io_grpc_grpc_protobuf",
    artifact = "io.grpc:grpc-protobuf:1.0.1",
    sha1 = "17222b03c64a65eb05de5ab266c920fca8c90fab",
)

maven_jar(
    name = "io_grpc_grpc_protobuf_lite",
    artifact = "io.grpc:grpc-protobuf-lite:1.0.1",
    sha1 = "b28a07b56ed2e66088221cbaf1228fa4e9669166",
)

maven_jar(
    name = "com_google_code_findbugs_jsr305",
    artifact = "com.google.code.findbugs:jsr305:3.0.1",
    sha1 = "f7be08ec23c21485b9b5a1cf1654c2ec8c58168d",
)

git_repository(
    name = "io_bazel_rules_go",
    remote = "https://github.com/bazelbuild/rules_go.git",
    tag = "0.2.0",
)

load("@io_bazel_rules_go//go:def.bzl", "go_repositories")

go_repositories()

http_archive(
    name = "io_bazel",
    sha256 = "9b2fffa18d0e87baded2dc4aba470f6274fb66e2b2c6119b93426c47ab19fb40",
    strip_prefix = "bazel-0.4.0",
    url = "https://github.com/bazelbuild/bazel/archive/0.4.0.zip",
)

new_git_repository(
    name = "go_gogo_protobuf",
    build_file = "third_party/go/gogo_protobuf.BUILD",
    commit = "43a2e0b1c32252bfbbdf81f7faa7a88fb3fa4028",
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
    commit = "b5ed9db83fbf41b937824795876668fc8d31335a",
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
    name = "go_grpc",
    build_file = "third_party/go/grpc.BUILD",
    commit = "2700f043b937c2b59b4a520bc6ddbb440a2de20e",
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
    commit = "09623af4dbc0fdd85acd9ed0da7f5028aa03c139",
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
    commit = "1f49d83d9aa00e6ce4fc8258c71cc7786aec968a",
    remote = "https://github.com/golang/protobuf.git",
)

new_git_repository(
    name = "go_levigo",
    build_file = "third_party/go/levigo.BUILD",
    commit = "1ddad808d437abb2b8a55a950ec2616caa88969b",
    remote = "https://github.com/jmhodges/levigo.git",
)

maven_jar(
    name = "com_google_auto_value",
    artifact = "com.google.auto.value:auto-value:1.3",
    sha1 = "4961194f62915eb45e21940537d60ac53912c57d",
)
