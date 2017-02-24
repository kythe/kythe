package(default_visibility = ["//kythe:default_visibility"])

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
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:supported_language",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:buildinfo_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/llvm/src:preprocessor_util_lib",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
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
        "//kythe/cxx/common:supported_language",
        "//kythe/proto:analysis_proto_cc",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
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
        "//kythe/cxx/common:supported_language",
        "//third_party/bazel:extra_actions_base_proto_cc",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "objcbazelcmdlib",
    srcs = [
        "objc_extractor_bazel_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        ":objc_bazel_support_library",
        "//kythe/cxx/common:supported_language",
        "//third_party/bazel:extra_actions_base_proto_cc",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "objc_bazel_support_library",
    srcs = ["objc_bazel_support.cc"],
    hdrs = ["objc_bazel_support.h"],
    visibility = ["//visibility:private"],
    deps = [
        "//third_party/bazel:extra_actions_base_proto_cc",
        "//third_party/llvm",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "objc_bazel_support_library_test",
    srcs = ["objc_bazel_support_test.cc"],
    deps = [
        ":objc_bazel_support_library",
        "//third_party:gtest",
        "@com_github_google_glog//:glog",
    ],
)

action_listener(
    name = "extract_kindex",
    extra_actions = [":extra_action"],
    mnemonics = ["CppCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :cxx_extractor_bazel) \
        $(EXTRA_ACTION_FILE) \
        $(output $(ACTION_ID).c++.kindex) \
        $(location //kythe/data:vnames_config)",
    data = ["//kythe/data:vnames_config"],
    out_templates = ["$(ACTION_ID).c++.kindex"],
    tools = [":cxx_extractor_bazel"],
)

action_listener(
    name = "extract_kindex_objc",
    extra_actions = [":extra_action_objc"],
    mnemonics = ["ObjcCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action_objc",
    cmd = "$(location :objc_extractor_bazel) \
        $(EXTRA_ACTION_FILE) \
        $(output $(ACTION_ID).objc.kindex) \
        $(location //kythe/data:vnames_config) \
        $(location //third_party/bazel:get_devdir) \
        $(location //third_party/bazel:get_sdkroot)",
    data = [
        "//kythe/data:vnames_config",
        "//third_party/bazel:get_devdir",
        "//third_party/bazel:get_sdkroot",
    ],
    out_templates = ["$(ACTION_ID).objc.kindex"],
    tools = [":objc_extractor_bazel"],
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

cc_binary(
    name = "objc_extractor_bazel",
    deps = [
        ":objcbazelcmdlib",
    ],
)

cc_library(
    name = "testlib",
    testonly = 1,
    srcs = [
        "cxx_extractor_test.cc",
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
        "//third_party:gtest",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "test",
    size = "small",
    deps = [
        ":testlib",
    ],
)
