load("@bazel_skylib//rules:copy_file.bzl", "copy_file")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":proto_verifier_test.bzl", "proto_verifier_test")

package(features = ["layering_check"])

licenses(["notice"])  # Apache 2.0

proto_verifier_test(
    name = "signatures",
    srcs = [":signatures_proto"],
    resolve_code_facts = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "file_node_existence",
    srcs = [":basic_file_node_proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "default_values",
    srcs = [":basic_default_value_proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "doc_comments",
    srcs = [":basic_doc_proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "enum_basics",
    srcs = [":basic_enums_proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "extensions",
    srcs = [":basic_extend_proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "message_fields",
    srcs = [":basic_message_fields_proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "nested_message_fields",
    srcs = [
        ":basic_nested_message_field_proto",
        ":basic_nested_message_proto",
    ],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "oneof_fields",
    srcs = [":basic_oneof_proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "services",
    srcs = [":basic_services_proto"],
    convert_marked_source = True,
    tags = ["basic"],
)

proto_verifier_test(
    name = "map_test",
    srcs = [":basic_map_proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "proto3",
    srcs = [":proto3_proto"],
    tags = ["basic"],
)

proto_verifier_test(
    name = "deprecated",
    srcs = [":basic_deprecated_proto"],
    tags = ["basic"],
)

test_suite(
    name = "indexer_basic",
    tags = ["basic"],
)

proto_verifier_test(
    name = "import_syntax",
    srcs = [
        ":basic_nested_message_proto",
        ":basic_oneof_proto",
        ":corner_cases_import_syntax_proto",
        ":other_package_proto",
    ],
    convert_marked_source = True,
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "tabs",
    srcs = [
        ":corner_cases_tabs_proto",
        ":other_package_proto",
    ],
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "wide_chars",
    srcs = [":corner_cases_wide_chars_proto"],
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "missing_package",
    srcs = [":corner_cases_nopackage_proto"],
    tags = ["corner_cases"],
)

proto_verifier_test(
    name = "generate_proto",
    srcs = [
        ":gen_proto_dest_proto",
        ":gen_proto_source_proto",
    ],
    tags = ["corner_cases"],
    verifier_opts = ["--show_protos"],
)

test_suite(
    name = "indexer_corner_cases",
    tags = ["corner_cases"],
)

bzl_library(
    name = "proto_verifier_test_bzl",
    srcs = ["proto_verifier_test.bzl"],
)

proto_library(
    name = "basic_default_value_proto",
    srcs = ["basic/default-value.proto"],
)

proto_library(
    name = "basic_deprecated_proto",
    srcs = ["basic/deprecated.proto"],
)

proto_library(
    name = "basic_doc_proto",
    srcs = ["basic/doc.proto"],
)

proto_library(
    name = "basic_enums_proto",
    srcs = ["basic/enums.proto"],
)

proto_library(
    name = "basic_extend_proto",
    srcs = ["basic/extend.proto"],
    deps = [":other_package_proto"],
)

proto_library(
    name = "signatures_proto",
    srcs = ["basic/signatures.proto"],
)

proto_library(
    name = "basic_file_node_proto",
    srcs = ["basic/file_node.proto"],
)

proto_library(
    name = "basic_message_fields_proto",
    srcs = ["basic/message-fields.proto"],
)

proto_library(
    name = "basic_nested_message_field_proto",
    srcs = ["basic/nested-message-field.proto"],
    deps = [":basic_nested_message_proto"],
)

proto_library(
    name = "basic_nested_message_proto",
    srcs = ["basic/nested-message.proto"],
)

proto_library(
    name = "basic_oneof_proto",
    srcs = ["basic/oneof.proto"],
)

proto_library(
    name = "basic_services_proto",
    srcs = ["basic/services.proto"],
)

proto_library(
    name = "basic_map_proto",
    srcs = ["basic/map.proto"],
)

proto_library(
    name = "corner_cases_import_syntax_proto",
    srcs = ["corner_cases/import_syntax.proto"],
    exports = [":basic_oneof_proto"],
    deps = [
        ":basic_nested_message_proto",
        ":basic_oneof_proto",
        ":other_package_proto",
    ],
)

proto_library(
    name = "corner_cases_nopackage_proto",
    srcs = ["corner_cases/nopackage.proto"],
)

proto_library(
    name = "corner_cases_tabs_proto",
    srcs = ["corner_cases/tabs.proto"],
    deps = [":other_package_proto"],
)

proto_library(
    name = "corner_cases_wide_chars_proto",
    srcs = ["corner_cases/wide-chars.proto"],
)

proto_library(
    name = "gen_code_proto",
    srcs = ["gen-code.proto"],
)

proto_library(
    name = "other_package_proto",
    srcs = ["other-package.proto"],
)

proto_library(
    name = "proto3_proto",
    srcs = ["proto3.proto"],
)

proto_library(
    name = "xlang_fields_api2_proto",
    srcs = ["xlang_fields_api2.proto"],
)

proto_library(
    name = "gen_proto_source_proto",
    srcs = ["corner_cases/gen-proto-source.proto"],
)

copy_file(
    name = "gen_proto_fake_gen",
    src = "corner_cases/gen-proto-dest.proto",
    out = "corner_cases/gen-proto-dest.gen.proto",
)

proto_library(
    name = "gen_proto_dest_proto",
    srcs = [":gen_proto_fake_gen"],
    deps = [
        "//kythe/proto:generated_message_info_proto",
    ],
)
