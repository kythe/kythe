licenses(["notice"])

package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "json_proto",
    srcs = [
        "json_proto.cc",
    ],
    hdrs = [
        "json_proto.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//third_party/proto:cc_wkt_protos",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
        "@boringssl//:crypto",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "index_pack",
    srcs = ["index_pack.cc"],
    hdrs = ["index_pack.h"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":lib",
        "//external:libuuid",
        "//kythe/proto:analysis_proto_cc",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "CommandLineUtils.cc",
        "cxx_details.cc",
        "file_vname_generator.cc",
        "kythe_metadata_file.cc",
        "kythe_uri.cc",
        "path_utils.cc",
        "protobuf_metadata_file.cc",
    ],
    hdrs = [
        "CommandLineUtils.h",
        "cxx_details.h",
        "file_vname_generator.h",
        "kythe_metadata_file.h",
        "kythe_uri.h",
        "path_utils.h",
        "proto_conversions.h",
        "protobuf_metadata_file.h",
        "vname_ordering.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":json_proto",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
        "//third_party/proto:cc_wkt_protos",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "net_client",
    srcs = [
        "net_client.cc",
    ],
    hdrs = [
        "net_client.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/proto:graph_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//kythe/proto:xref_proto_cc",
        "//third_party:libcurl",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "commandline_testlib",
    testonly = 1,
    srcs = [
        "CommandLineUtilsTest.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//third_party/llvm",
        "//third_party:gtest",
    ],
)

cc_test(
    name = "commandline_test",
    size = "small",
    deps = [
        ":commandline_testlib",
    ],
)

cc_library(
    name = "file_vname_generator_testlib",
    testonly = 1,
    srcs = [
        "file_vname_generator_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//third_party/proto:protobuf",
        "//third_party:gtest",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "file_vname_generator_test",
    size = "small",
    deps = [
        ":file_vname_generator_testlib",
    ],
)

cc_library(
    name = "path_utils_testlib",
    testonly = 1,
    srcs = [
        "path_utils_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//third_party/llvm",
        "//third_party:gtest",
    ],
)

cc_test(
    name = "path_utils_test",
    size = "small",
    deps = [
        ":path_utils_testlib",
    ],
)

cc_library(
    name = "index_pack_testlib",
    testonly = 1,
    srcs = [
        "index_pack_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":index_pack",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "//third_party:gtest",
    ],
)

cc_test(
    name = "index_pack_test",
    size = "small",
    deps = [
        ":index_pack_testlib",
    ],
)

cc_library(
    name = "json_proto_testlib",
    testonly = 1,
    srcs = [
        "json_proto_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":json_proto",
        "//kythe/proto:analysis_proto_cc",
        "//third_party/proto:protobuf",
        "//third_party:gtest",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "json_proto_test",
    size = "small",
    deps = [
        ":json_proto_testlib",
    ],
)

cc_library(
    name = "net_client_testlib",
    testonly = 1,
    srcs = [
        "net_client_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":net_client",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "net_client_test",
    testonly = 1,
    deps = [
        ":net_client_testlib",
    ],
)

cc_library(
    name = "kythe_uri_testlib",
    testonly = 1,
    srcs = [
        "kythe_uri_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//third_party/proto:protobuf",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "//third_party:gtest",
    ],
)

cc_test(
    name = "kythe_uri_test",
    size = "small",
    deps = [
        ":kythe_uri_testlib",
    ],
)
