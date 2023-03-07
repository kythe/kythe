load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":textproto_verifier_test.bzl", "textproto_verifier_test")

package(features = ["layering_check"])

textproto_verifier_test(
    name = "basics_test",
    protos = [":example_proto"],
    textprotos = ["basics.pbtxt"],
)

textproto_verifier_test(
    name = "nested_message_test",
    protos = [":example_proto"],
    textprotos = ["nested_message.pbtxt"],
)

textproto_verifier_test(
    name = "repeated_field_test",
    protos = [":example_proto"],
    textprotos = ["repeated_field.pbtxt"],
)

textproto_verifier_test(
    name = "inline_repeated_field_test",
    protos = [":example_proto"],
    textprotos = ["inline_repeated_field.pbtxt"],
)

textproto_verifier_test(
    name = "default_value_test",
    protos = [":example_proto"],
    textprotos = ["default_value.pbtxt"],
)

textproto_verifier_test(
    name = "enum_value_test",
    protos = [":example_proto"],
    textprotos = ["enum_value.pbtxt"],
)

textproto_verifier_test(
    name = "imports_test",
    protos = [
        ":example_proto",
        ":proto_with_imports_proto",
    ],
    textprotos = ["imports.pbtxt"],
)

textproto_verifier_test(
    name = "imports_generated_test",
    protos = [
        ":generated_proto",
        ":imports_generated_proto",
    ],
    textprotos = ["generated_imports.pbtxt"],
)

textproto_verifier_test(
    name = "extensions_test",
    protos = [":extensions_proto"],
    textprotos = ["extensions.pbtxt"],
)

textproto_verifier_test(
    name = "any_type_test",
    protos = [
        ":any_type_proto",
        "@com_google_protobuf//:any_proto",
    ],
    textprotos = ["any_type.pbtxt"],
)

textproto_verifier_test(
    name = "repeated_extension_test",
    protos = [":repeated_extension_proto"],
    textprotos = ["repeated_extension.pbtxt"],
)

textproto_verifier_test(
    name = "schema_comments_test",
    protos = [
        ":example_proto",
    ],
    textprotos = ["schema_comments.pbtxt"],
)

textproto_verifier_test(
    name = "recordio_basics_test",
    protos = [":example_proto"],
    textprotos = ["recordio_basics.recordiotxt"],
    extractor_opts = ["--record_separator", "@@@@@@@@@@@@@@@@"],
)

textproto_verifier_test(
    name = "recordio_emptyline_delimiter_test",
    protos = [":example_proto"],
    textprotos = ["recordio_emptyline_delimiter.recordiotxt"],
    extractor_opts = ["--record_separator", "\n"],
)

bzl_library(
    name = "textproto_verifier_test_bzl",
    srcs = ["textproto_verifier_test.bzl"],
)

proto_library(
    name = "any_type_proto",
    srcs = ["any_type.proto"],
    deps = [
        "@com_google_protobuf//:any_proto",
    ],
)

proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
)

proto_library(
    name = "extensions_proto",
    srcs = ["extensions.proto"],
)

proto_library(
    name = "proto_with_imports_proto",
    srcs = ["proto_with_imports.proto"],
    deps = [":example_proto"],
)

proto_library(
    name = "repeated_extension_proto",
    srcs = ["repeated_extension.proto"],
)

genrule(
    name = "generate_proto",
    srcs = ["example.proto"],
    outs = ["generated.proto"],
    cmd = "cp $< $@",
)

proto_library(
    name = "generated_proto",
    srcs = [":generated.proto"],
)

proto_library(
    name = "imports_generated_proto",
    srcs = ["imports_generated.proto"],
    deps = [":generated_proto"],
)
