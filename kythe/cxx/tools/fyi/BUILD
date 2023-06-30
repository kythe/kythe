package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

cc_binary(
    name = "fyi",
    srcs = [
        "fyi.cc",
        "fyi.h",
        "fyi_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
        "//kythe/cxx/common/schema:edges",
        "//kythe/cxx/common/schema:facts",
        "//kythe/cxx/indexer/cxx:clang_utils",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:parse",
        "@llvm-project//clang:rewrite",
        "@llvm-project//clang:sema",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)
