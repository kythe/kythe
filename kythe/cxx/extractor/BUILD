package(default_visibility = ["//visibility:public"])

cc_library(
    name = "lib",
    srcs = [
        "cxx_extractor.cc",
    ],
    hdrs = [
        "cxx_extractor.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/llvm/src:preprocessor_util_lib",
        "//third_party/proto:protobuf",
        "//third_party/re2",
        "//third_party/zlib",
    ],
)

cc_library(
    name = "cmdlib",
    srcs = [
        "cxx_extractor_main.cc",
    ],
    hdrs = [
        "cxx_extractor.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/re2",
        "//third_party/zlib",
    ],
)

cc_library(
    name = "bazelcmdlib",
    srcs = [
        "cxx_extractor_bazel_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/bazel:extra_actions_base_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/re2",
        "//third_party/zlib",
    ],
)

action_listener(
    name = "extract_kindex",
    extra_actions = [":extra_action"],
    mnemonics = ["CppCompile"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :cxx_extractor_bazel) $(EXTRA_ACTION_FILE) $(output $(ACTION_ID).cpp.kindex) $(location //kythe/data:vnames_config)",
    data = ["//kythe/data:vnames_config"],
    out_templates = ["$(ACTION_ID).cpp.kindex"],
    tools = [":cxx_extractor_bazel"],
)

cc_binary(
    name = "cxx_extractor",
    deps = [
        ":cmdlib",
    ],
)

cc_binary(
    name = "cxx_extractor_bazel",
    deps = [
        ":bazelcmdlib",
    ],
)
