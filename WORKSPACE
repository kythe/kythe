workspace(name = "io_kythe")

load("//:version.bzl", "check_version")

check_version("0.2.3")

load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

load("//tools/go:go_configure.bzl", "go_configure")

go_configure()

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
    name = "maven_guava",
    artifact = "com.google.guava:guava:19.0",
    sha1 = "6ce200f6b23222af3d8abb6b6459e6c44f4bb0e9",
)

http_file(
    name = "com_apache_org_license_2_0",
    sha256 = "cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30",
    url = "http://www.apache.org/licenses/LICENSE-2.0.txt",
)

bind(
    name = "guava",
    actual = "@maven_guava//jar",
)

bind(
    name = "guava/license",
    actual = "@com_apache_org_license_2_0//file",
)

bind(
    name = "libcurl",
    actual = "//third_party:libcurl",
)

bind(
    name = "junit4",
    actual = "//third_party:junit4",
)

bind(
    name = "zlib",
    actual = "//third_party/zlib",
)

bind(
    name = "re2j",
    actual = "//third_party/re2j",
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

bind(
    name = "grpc-java",
    actual = "//third_party/grpc-java",
)

bind(
    name = "rapidjson",
    actual = "//third_party/rapidjson",
)

bind(
    name = "gson",
    actual = "//third_party/gson",
)

bind(
    name = "gson/proto",
    actual = "//third_party/gson:proto",
)

bind(
    name = "jcommander",
    actual = "//third_party/jcommander",
)

bind(
    name = "jq",
    actual = "//third_party/jq",
)

bind(
    name = "truth",
    actual = "//third_party/truth",
)

bind(
    name = "go_package_prefix",
    actual = "//:go_package_prefix",
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
    commit = "0529c5393e499f4f538b167ec5b85e74a58a02f8",
    remote = "https://github.com/GoogleCloudPlatform/gcloud-golang.git",
)

new_git_repository(
    name = "go_x_net",
    build_file = "third_party/go/x_net.BUILD",
    commit = "3797cd8864994d713d909eda5e61ede8683fdc12",
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
    commit = "a8702432015187171c5120cbf5020bfca7be35b6",
    remote = "https://github.com/golang/oauth2.git",
)

new_git_repository(
    name = "go_gapi",
    build_file = "third_party/go/gapi.BUILD",
    commit = "c13a21ee847eca050f08db8373d8737494a1170e",
    remote = "https://github.com/google/google-api-go-client.git",
)

new_git_repository(
    name = "go_grpc",
    build_file = "third_party/go/grpc.BUILD",
    commit = "b062a3c003c22bfef58fa99d689e6a892b408f9d",
    remote = "https://github.com/grpc/grpc-go.git",
)

new_git_repository(
    name = "go_shell",
    build_file = "third_party/go/shell.BUILD",
    commit = "4e4a4403205db46f1ef0590e98dc814a38d2ea63",
    remote = "https://bitbucket.org/creachadair/shell.git",
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
    remote = "https://github.com/sergi/go-diff",
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
    commit = "874264fbbb43f4d91e999fecb4b40143ed611400",
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
