load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "schema",
    srcs = [
        "Nodes.java",
        ":schema_index",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/proto:internal_java_proto",
        "//kythe/proto:schema_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
    ],
)

genrule(
    name = "schema_index",
    outs = ["Schema.java"],
    cmd = " ".join([
        "$(location //kythe/go/util/schema/mkdata) --language java",
        "--package com.google.devtools.kythe.util.schema",
        "-output '$@'",
    ]),
    tools = ["//kythe/go/util/schema/mkdata"],
    visibility = ["//visibility:private"],
)
