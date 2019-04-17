package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "schema",
    srcs = [":schema_index"],
    deps = [
        "//kythe/proto:schema_java_proto",
        "//third_party/guava",
    ],
)

genrule(
    name = "schema_index",
    srcs = ["//kythe/data:schema_index.textproto"],
    outs = ["Schema.java"],
    cmd = " ".join([
        "$(location //kythe/go/util/schema/mkdata) --language java",
        "--package com.google.devtools.kythe.util.schema",
        "-input '$(location //kythe/data:schema_index.textproto)'",
        "-output '$@'",
    ]),
    tools = ["//kythe/go/util/schema/mkdata"],
    visibility = ["//visibility:private"],
)
