package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

licenses(["notice"])

cc_library(
    name = "markup_handler",
    srcs = [
        "html_markup_handler.cc",
        "javadoxygen_markup_handler.cc",
        "markup_handler.cc",
    ],
    hdrs = [
        "html_markup_handler.h",
        "javadoxygen_markup_handler.h",
        "markup_handler.h",
    ],
    deps = [
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:xref_cc_proto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "markup_handler_testlib",
    testonly = 1,
    srcs = [
        "markup_handler_test.cc",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":markup_handler",
        "//third_party:gtest",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:initialize",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "markup_handler_test",
    size = "small",
    deps = [
        ":markup_handler_testlib",
    ],
)

cc_library(
    name = "html_renderer",
    srcs = [
        "html_renderer.cc",
    ],
    hdrs = [
        "html_renderer.h",
    ],
    deps = [
        ":markup_handler",
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:lib",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:xref_cc_proto",
        "@com_google_absl//absl/log",
    ],
)

cc_library(
    name = "html_renderer_testlib",
    testonly = 1,
    srcs = [
        "html_renderer_test.cc",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":html_renderer",
        ":markup_handler",
        "//kythe/proto:common_cc_proto",
        "//third_party:gtest",
        "@com_google_absl//absl/log:initialize",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "html_renderer_test",
    size = "small",
    deps = [
        ":html_renderer_testlib",
    ],
)

cc_binary(
    name = "doc",
    srcs = [
        "doc.cc",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":html_renderer",
        ":markup_handler",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
        "//kythe/cxx/common/schema:edges",
        "//kythe/cxx/common/schema:facts",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)
