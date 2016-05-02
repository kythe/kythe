licenses(["notice"])

package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "html_renderer",
    srcs = [
        "html_renderer.cc",
    ],
    hdrs = [
        "html_renderer.h",
    ],
    deps = [
        "//external:glog",
        "//kythe/proto:xref_proto_cc",
    ],
)

cc_test(
    name = "html_renderer_test",
    srcs = [
        "html_renderer_test.cc",
    ],
    deps = [
        ":html_renderer",
        "//external:googletest",
    ],
)

cc_library(
    name = "doclib",
    srcs = [
        "doc.cc",
    ],
    deps = [
        ":html_renderer",
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
