package(default_visibility = ["//kythe:default_visibility"])

licenses(["notice"])

cc_library(
    name = "scope_guard",
    hdrs = ["scope_guard.h"],
)

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
        ":status_or",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:cc_wkt_protos",
        "@com_google_protobuf//:protobuf",
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
    deprecation = "Please use :kzip_{reader,writer}",
    deps = [
        ":lib",
        "//external:libuuid",
        "//external:zlib",
        "//kythe/proto:analysis_cc_proto",
        "@boringssl//:crypto",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "kythe_uri",
    srcs = ["kythe_uri.cc"],
    hdrs = ["kythe_uri.h"],
    deps = [
        ":vname_ordering",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "path_utils",
    srcs = ["path_utils.cc"],
    hdrs = ["path_utils.h"],
    deps = [
        ":lib",
    ],
)

cc_library(
    name = "vname_ordering",
    hdrs = ["vname_ordering.h"],
    deps = [
        "//kythe/proto:storage_cc_proto",
    ],
)

cc_library(
    name = "cxx_details",
    srcs = [
        "cxx_details.cc",
    ],
    hdrs = [
        "cxx_details.h",
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
        "//kythe/proto:cxx_cc_proto",
        "//third_party/llvm",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "file_vname_generator.cc",
        "kythe_metadata_file.cc",
        "path_utils.cc",
        "protobuf_metadata_file.cc",
    ],
    hdrs = [
        "file_vname_generator.h",
        "kythe_metadata_file.h",
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
    ],
    deps = [
        ":json_proto",
        "//external:zlib",
        "//kythe/cxx/common/schema:edges",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//third_party/llvm",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:cc_wkt_protos",
        "@com_google_protobuf//:protobuf",
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
        "//external:zlib",
        "//kythe/proto:graph_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//kythe/proto:xref_cc_proto",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_protobuf//:protobuf",
        "@se_haxx_curl//:curl",
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
        "//third_party:gtest",
        "@com_google_protobuf//:protobuf",
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
        ":path_utils",
        "//third_party:gtest",
        "//third_party/llvm",
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
    srcs = ["index_pack_test.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":index_pack",
        "//external:zlib",
        "//third_party:gtest",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "index_pack_test",
    size = "small",
    deps = [":index_pack_testlib"],
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
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gtest",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "json_proto_test",
    size = "small",
    deps = [":json_proto_testlib"],
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
        "//external:zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "net_client_test",
    testonly = 1,
    deps = [":net_client_testlib"],
)

cc_library(
    name = "kythe_uri_testlib",
    testonly = 1,
    srcs = ["kythe_uri_test.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":kythe_uri",
        "//third_party:gtest",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "kythe_uri_test",
    size = "small",
    deps = [":kythe_uri_testlib"],
)

cc_library(
    name = "supported_language",
    srcs = ["language.cc"],
    hdrs = ["language.h"],
    deps = ["//third_party/llvm"],
)

cc_library(
    name = "index_writer",
    srcs = ["index_writer.cc"],
    hdrs = ["index_writer.h"],
    deps = [
        ":status_or",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "index_reader",
    srcs = ["index_reader.cc"],
    hdrs = ["index_reader.h"],
    deps = [
        ":status_or",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "kzip_reader",
    srcs = ["kzip_reader.cc"],
    hdrs = ["kzip_reader.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":index_reader",
        ":json_proto",
        ":libzip/error",
        ":status_or",
        "//kythe/proto:analysis_cc_proto",
        "@boringssl//:crypto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@org_libzip//:zip",
    ],
)

cc_test(
    name = "kzip_reader_test",
    srcs = ["kzip_reader_test.cc"],
    data = [
        "//kythe/cxx/common/testdata:empty.kzip",
        "//kythe/cxx/common/testdata:malformed.kzip",
        "//kythe/cxx/common/testdata:stringset.kzip",
    ],
    deps = [
        ":kzip_reader",
        ":libzip/error",
        ":testutil",
        "//kythe/proto:go_cc_proto",  # Used in stringset.kzip.
        "//third_party:gtest_main",
    ],
)

cc_library(
    name = "kzip_writer",
    srcs = ["kzip_writer.cc"],
    hdrs = ["kzip_writer.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":index_writer",
        ":json_proto",
        ":libzip/error",
        "//kythe/proto:analysis_cc_proto",
        "@boringssl//:crypto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@org_libzip//:zip",
    ],
)

cc_test(
    name = "kzip_writer_test",
    srcs = ["kzip_writer_test.cc"],
    data = [
        "//kythe/cxx/common/testdata:stringset.kzip",
    ],
    deps = [
        ":kzip_reader",
        ":kzip_writer",
        ":libzip/error",
        ":testutil",
        "//kythe/proto:go_cc_proto",  # Used in stringset.kzip.
        "//third_party:gtest_main",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "status",
    srcs = ["status.cc"],
    hdrs = ["status.h"],
    visibility = ["//visibility:private"],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "status_or",
    hdrs = ["status_or.h"],
    visibility = ["//visibility:private"],
    deps = [
        ":status",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "libzip/error",
    srcs = ["libzip/error.cc"],
    hdrs = ["libzip/error.h"],
    deps = [
        ":status",
        "@org_libzip//:zip",
    ],
)

cc_library(
    name = "testutil",
    testonly = 1,
    srcs = ["testutil.cc"],
    hdrs = ["testutil.h"],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)
