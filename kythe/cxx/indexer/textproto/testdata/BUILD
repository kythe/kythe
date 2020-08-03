load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":textproto_verifier_test.bzl", "textproto_verifier_test")

textproto_verifier_test(
    name = "basics_test",
    protos = ["example.proto"],
    textproto = "basics.pbtxt",
)

textproto_verifier_test(
    name = "nested_message_test",
    protos = ["example.proto"],
    textproto = "nested_message.pbtxt",
)

textproto_verifier_test(
    name = "repeated_field_test",
    protos = ["example.proto"],
    textproto = "repeated_field.pbtxt",
)

textproto_verifier_test(
    name = "inline_repeated_field_test",
    protos = ["example.proto"],
    textproto = "inline_repeated_field.pbtxt",
)

textproto_verifier_test(
    name = "default_value_test",
    protos = ["example.proto"],
    textproto = "default_value.pbtxt",
)

textproto_verifier_test(
    name = "imports_test",
    protos = [
        "example.proto",
        "proto_with_imports.proto",
    ],
    textproto = "imports.pbtxt",
)

textproto_verifier_test(
    name = "extensions_test",
    protos = ["extensions.proto"],
    textproto = "extensions.pbtxt",
)

textproto_verifier_test(
    name = "any_type_test",
    extractor_opts = [
        "--proto_path",
        "external/com_google_protobuf/src",
    ],
    proto_extractor_opts = [
        "--proto_path",
        "external/com_google_protobuf/src",
    ],
    protos = [
        "any_type.proto",
        "@com_google_protobuf//:well_known_protos",
    ],
    textproto = "any_type.pbtxt",
)

textproto_verifier_test(
    name = "repeated_extension_test",
    protos = ["repeated_extension.proto"],
    textproto = "repeated_extension.pbtxt",
)

textproto_verifier_test(
    name = "schema_comments_test",
    extractor_opts = [
        "--proto_path",
        "external/com_google_protobuf/src",
    ],
    proto_extractor_opts = [
        "--proto_path",
        "external/com_google_protobuf/src",
    ],
    protos = [
        "example.proto",
    ],
    textproto = "schema_comments.pbtxt",
    deps = ["@com_google_protobuf//:well_known_protos"],
)

bzl_library(
    name = "textproto_verifier_test_bzl",
    srcs = ["textproto_verifier_test.bzl"],
)
