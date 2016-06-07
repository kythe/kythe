licenses(["notice"])

package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "markup_handler",
    srcs = [
        "javadoxygen_markup_handler.cc",
        "markup_handler.cc",
    ],
    hdrs = [
        "javadoxygen_markup_handler.h",
        "markup_handler.h",
    ],
    deps = [
        "//external:glog",
        "//kythe/proto:xref_proto_cc",
    ],
)

cc_library(
    name = "markup_handler_testlib",
    testonly = 1,
    srcs = [
        "markup_handler_test.cc",
    ],
    deps = [
        ":markup_handler",
        "//external:googletest",
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
        "//external:glog",
        "//kythe/proto:xref_proto_cc",
    ],
)

cc_library(
    name = "html_renderer_testlib",
    testonly = 1,
    srcs = [
        "html_renderer_test.cc",
    ],
    deps = [
        ":html_renderer",
        "//external:googletest",
    ],
)

cc_test(
    name = "html_renderer_test",
    size = "small",
    deps = [
        ":html_renderer_testlib",
    ],
)

cc_library(
    name = "doclib",
    srcs = [
        "doc.cc",
    ],
    deps = [
        ":html_renderer",
        ":markup_handler",
        "//external:gflags",
        "//external:glog",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
    ],
)

cc_binary(
    name = "doc",
    deps = [
        ":doclib",
    ],
)
