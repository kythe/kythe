licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

load(":proto_verifier_test.bzl", "proto_verifier_test")

proto_verifier_test(
    name = "file_node_existence",
    srcs = ["basic/file_node.proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "default_values",
    srcs = ["basic/default-value.proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "doc_comments",
    srcs = ["basic/doc.proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "enum_basics",
    srcs = ["basic/enums.proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "extensions",
    srcs = [
        "basic/extend.proto",
        "other-package.proto",
    ],
    tags = ["basic"],
)

proto_verifier_test(
    name = "message_fields",
    srcs = ["basic/message-fields.proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "nested_message_fields",
    srcs = [
        "basic/nested-message.proto",
        "basic/nested-message-field.proto",
    ],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "oneof_fields",
    srcs = ["basic/oneof.proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "services",
    srcs = ["basic/services.proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "proto3",
    srcs = ["proto3.proto"],
    tags = ["basic"],
)

test_suite(
    name = "indexer_basic",
    tags = ["basic"],
)

proto_verifier_test(
    name = "import_syntax",
    srcs = [
        "basic/nested-message.proto",
        "basic/oneof.proto",
        "corner_cases/import_syntax.proto",
        "other-package.proto",
    ],
    convert_marked_source = True,
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "tabs",
    srcs = [
        "corner_cases/tabs.proto",
        "other-package.proto",
    ],
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "wide_chars",
    srcs = ["corner_cases/wide-chars.proto"],
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "missing_package",
    srcs = ["corner_cases/nopackage.proto"],
    tags = ["corner_cases"],
)

test_suite(
    name = "indexer_corner_cases",
    tags = ["corner_cases"],
)
