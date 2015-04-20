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
    linkopts = [
        "-lcrypto",
    ],
    deps = [
        "//kythe/proto:any_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "CommandLineUtils.cc",
        "cxx_details.cc",
        "file_vname_generator.cc",
        "index_pack.cc",
        "kythe_uri.cc",
        "path_utils.cc",
    ],
    hdrs = [
        "CommandLineUtils.h",
        "cxx_details.h",
        "file_vname_generator.h",
        "index_pack.h",
        "kythe_uri.h",
        "path_utils.h",
        "proto_conversions.h",
        "vname_ordering.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    linkopts = [
        "-luuid",
    ],
    visibility = [
        "//kythe/cxx:__subpackages__",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":json_proto",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
        "//third_party/re2",
        "//third_party/zlib",
    ],
)
