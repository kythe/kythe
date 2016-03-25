package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "metaindexlib",
    srcs = [
        "commonjs_package_registry.cc",
        "meta_index.cc",
        "meta_index_main.cc",
        "vcs.cc",
    ],
    hdrs = [
        "commonjs_package_registry.h",
        "meta_index.h",
        "utils.h",
        "vcs.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//external:rapidjson",
        "//external:re2",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
    ],
)

cc_binary(
    name = "meta_index",
    deps = [
        ":metaindexlib",
    ],
)
