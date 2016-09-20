workspace(name = "io_kythe")

load("//:version.bzl", "check_version")

check_version("0.3.0-2016-07-20")

load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

load("//tools:node_configure.bzl", "node_configure")

node_configure()

load("//tools/build_rules/config:system.bzl", "cc_system_package")

cc_system_package(
    name = "libcrypto",
    default = "/usr/local/opt/openssl",
    envvar = "OPENSSL_HOME",
)

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

new_git_repository(
    name = "com_github_google_googletest",
    build_file = "third_party/googletest.BUILD",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.7.0",
)

bind(
    name = "googletest",
    actual = "@com_github_google_googletest//:googletest",
)

bind(
    name = "googletest/license",
    actual = "@com_github_google_googletest//:license",
)

new_git_repository(
    name = "com_github_gflags_gflags",
    build_file = "third_party/googleflags.BUILD",
    commit = "58345b18d92892a170d61a76c5dd2d290413bdd7",
    remote = "https://github.com/gflags/gflags.git",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

bind(
    name = "gflags/license",
    actual = "@com_github_gflags_gflags//:license",
)

git_repository(
    name = "com_googlesource_code_re2",
    commit = "fc6337a382bfd4f7c861abea08f872d3c85b31da",
    remote = "https://code.googlesource.com/re2",
)

bind(
    name = "re2",
    actual = "@com_googlesource_code_re2//:re2",
)

bind(
    name = "re2/license",
    actual = "@com_googlesource_code_re2//:LICENSE",
)

new_git_repository(
    name = "com_github_google_glog",
    build_file = "third_party/googlelog/BUILD.remote",
    commit = "1b0b08c8dda1659027677966b03a3ff3c488e549",
    remote = "https://github.com/google/glog.git",
)

bind(
    name = "glog",
    actual = "@com_github_google_glog//:glog",
)

bind(
    name = "glog/license",
    actual = "@com_github_google_glog//:license",
)

maven_jar(
    name = "com_google_code_gson_gson",
    artifact = "com.google.code.gson:gson:2.7",
)

bind(
    name = "gson",
    actual = "@com_google_code_gson_gson//jar",
)

bind(
    name = "gson/license",
    actual = "@com_apache_org_license_2_0//file",
)

# TODO(shahms): See if we can use upstream (apparently github only for gson-proto)
bind(
    name = "gson/proto",
    actual = "//third_party/gson:proto",
)

maven_jar(
    name = "com_google_guava_guava",
    artifact = "com.google.guava:guava:19.0",
    sha1 = "6ce200f6b23222af3d8abb6b6459e6c44f4bb0e9",
)

http_file(
    name = "com_apache_org_license_2_0",
    sha256 = "cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30",
    url = "http://www.apache.org/licenses/LICENSE-2.0.txt",
)

http_file(
    name = "org_opensource_licenses_bsd_3_clause",
    sha256 = "8ace8cd6c04c30234a573632af05bbb8eab01094130b0330834603741fc1f6af",
    url = "https://opensource.org/licenses/BSD-3-Clause",
)

bind(
    name = "guava",
    actual = "@com_google_guava_guava//jar",
)

bind(
    name = "guava/license",
    actual = "@com_apache_org_license_2_0//file",
)

http_file(
    name = "junit_junit_license",
    sha256 = "9648bb2891b9813970bddb68d4be8a5e6ec8280d0180a53dfb29236b579c55bb",
    url = "https://raw.githubusercontent.com/junit-team/junit4/master/LICENSE-junit.txt",
)

maven_jar(
    name = "junit_junit",
    artifact = "junit:junit:4.12",
    sha1 = "2973d150c0dc1fefe998f834810d68f278ea58ec",
)

bind(
    name = "junit4",
    actual = "@junit_junit//jar",
)

bind(
    name = "junit4/license",
    actual = "@junit_junit_license//file",
)

maven_jar(
    name = "com_google_re2j_re2j",
    artifact = "com.google.re2j:re2j:1.1",
    sha1 = "d716952ab58aa4369ea15126505a36544d50a333",
)

bind(
    name = "re2j",
    actual = "@com_google_re2j_re2j//jar",
)

bind(
    name = "re2j/license",
    actual = "@org_opensource_licenses_bsd_3_clause//file",
)

maven_jar(
    name = "com_beust_jcommander",
    artifact = "com.beust:jcommander:1.48",
)

bind(
    name = "jcommander",
    actual = "@com_beust_jcommander//jar",
)

bind(
    name = "jcommander/license",
    actual = "@com_apache_org_license_2_0//file",
)

maven_jar(
    name = "com_google_truth_truth",
    artifact = "com.google.truth:truth:0.27",
)

bind(
    name = "truth",
    actual = "@com_google_truth_truth//jar",
)

bind(
    name = "truth/license",
    actual = "@com_apache_org_license_2_0//file",
)

bind(
    name = "proto/protobuf",
    actual = "//third_party/proto:protobuf",
)

bind(
    name = "proto/protobuf_java",
    actual = "//third_party/proto:protobuf_java",
)

bind(
    name = "proto/any_proto",
    actual = "//third_party/proto:any_proto",
)

bind(
    name = "proto/any_proto_cc",
    actual = "//third_party/proto:any_proto_cc",
)

bind(
    name = "proto/any_proto_java",
    actual = "//third_party/proto:any_proto_java",
)

bind(
    name = "proto/any_proto_go",
    actual = "//third_party/proto:any_proto_go",
)

maven_jar(
    name = "io_netty_netty_all",
    artifact = "io.netty:netty-all:4.1.3.Final",
)

bind(
    name = "netty/license",
    actual = "@com_apache_org_license_2_0//file",
)

maven_jar(
    name = "io_grpc_grpc_core",
    artifact = "io.grpc:grpc-core:1.0.1",
)

maven_jar(
    name = "io_grpc_grpc_context",
    artifact = "io.grpc:grpc-context:1.0.1",
)

maven_jar(
    name = "io_grpc_grpc_stub",
    artifact = "io.grpc:grpc-stub:1.0.1",
)

maven_jar(
    name = "io_grpc_grpc_netty",
    artifact = "io.grpc:grpc-netty:1.0.1",
)

maven_jar(
    name = "io_grpc_grpc_protobuf",
    artifact = "io.grpc:grpc-protobuf:1.0.1",
)

maven_jar(
    name = "io_grpc_grpc_protobuf_lite",
    artifact = "io.grpc:grpc-protobuf-lite:1.0.1",
)

bind(
    name = "grpc-java",
    actual = "//third_party/grpc-java",
)

maven_jar(
    name = "com_google_code_findbugs_jsr305",
    artifact = "com.google.code.findbugs:jsr305:3.0.1",
)

bind(
    name = "jsr305",
    actual = "@com_google_code_findbugs_jsr305//jar",
)

bind(
    name = "jsr305/license",
    actual = "@com_apache_org_license_2_0//file",
)

bind(
    name = "rapidjson",
    actual = "//third_party/rapidjson",
)

bind(
    name = "jq",
    actual = "//third_party/jq",
)

bind(
    name = "libcurl",
    actual = "//third_party:libcurl",
)

bind(
    name = "zlib",
    actual = "//third_party/zlib",
)

git_repository(
    name = "io_bazel_rules_go",
    commit = "a9fc44f15240198a34c09d7df96cf04509b33960",
    remote = "https://github.com/bazelbuild/rules_go.git",
)

load("@io_bazel_rules_go//go:def.bzl", "go_repositories")

go_repositories()

new_git_repository(
    name = "go_gogo_protobuf",
    build_file = "third_party/go/gogo_protobuf.BUILD",
    commit = "43a2e0b1c32252bfbbdf81f7faa7a88fb3fa4028",
    remote = "https://github.com/gogo/protobuf.git",
)

new_git_repository(
    name = "go_gcloud",
    build_file = "third_party/go/gcloud.BUILD",
    commit = "64568e4898e813ad748cf9523d56a65209c7dc18",
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
    commit = "f2932db7c0155d2ea19373270a3fa937349ac375",
    remote = "https://github.com/golang/tools.git",
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
    commit = "f8c796889b53ece50aada924fbbd0f98cf684de4",
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

bind(
    name = "android/sdk",
    actual = "//:nothing",
)
