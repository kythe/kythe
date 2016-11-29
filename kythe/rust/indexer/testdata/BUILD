package(default_visibility = ["//kythe:default_visibility"])

load(":rust_indexer_test.bzl", "rust_indexer_test")

test_suite(
    name = "testdata",
    tags = ["manual", "arc-ignore"],
    tests = [
        ":basic",
        ":function",
        ":variants",
    ],
)

test_suite(
    name = "basic",
    tags = ["manual", "arc-ignore"],
    tests = [
        ":const",
        ":file_content",
        ":file_node",
        ":static",
        ":var_ref",
        ":vardecl_local_anchor",
        ":vardecl_local_destructure",
        ":vardecl_local_shadow",
    ],
)

test_suite(
    name = "function",
    tags = ["manual", "arc-ignore"],
    tests = [
        ":function_call",
        ":function_decl",
        ":function_for_loop",
        ":function_params",
        ":function_reference",
        ":function_unnamed",
        ":impl_method_call",
        ":method_instance_call",
        ":method_static_call",
    ],
)

test_suite(
    name = "variants",
    tags = ["manual", "arc-ignore"],
    tests = [
        ":enum_decl",
        ":enum_pat",
        ":struct_record",
        ":struct_record_pat",
        ":struct_tuple",
        ":struct_tuple_pat",
        ":struct_unit",
    ],
)

rust_indexer_test(
    name = "const",
    tags = ["arc-ignore"],
    srcs = ["basic/const.rs"],
)

rust_indexer_test(
    name = "file_node",
    tags = ["arc-ignore"],
    srcs = ["basic/file_node.rs"],
)

rust_indexer_test(
    name = "file_content",
    tags = ["arc-ignore"],
    # buildifier: leave-alone
    srcs = [
        "basic/file_content.rs",
        "basic/a.rs",
    ],
)

rust_indexer_test(
    name = "static",
    tags = ["arc-ignore"],
    srcs = ["basic/static.rs"],
)

rust_indexer_test(
    name = "var_ref",
    tags = ["arc-ignore"],
    srcs = ["basic/var_ref.rs"],
)

rust_indexer_test(
    name = "vardecl_local_anchor",
    tags = ["arc-ignore"],
    srcs = ["basic/vardecl_local_anchor.rs"],
)

rust_indexer_test(
    name = "vardecl_local_shadow",
    tags = ["arc-ignore"],
    srcs = ["basic/vardecl_local_shadow.rs"],
    expect_fail_verify = True,
)

rust_indexer_test(
    name = "vardecl_local_destructure",
    tags = ["arc-ignore"],
    srcs = ["basic/vardecl_local_destructure.rs"],
)

rust_indexer_test(
    name = "function_call",
    tags = ["arc-ignore"],
    srcs = ["function/function_call.rs"],
)

rust_indexer_test(
    name = "function_decl",
    tags = ["arc-ignore"],
    srcs = ["function/function_decl.rs"],
)

rust_indexer_test(
    name = "function_for_loop",
    tags = ["arc-ignore"],
    srcs = ["function/function_for_loop.rs"],
)

rust_indexer_test(
    name = "function_params",
    tags = ["arc-ignore"],
    srcs = ["function/function_params.rs"],
)

rust_indexer_test(
    name = "function_reference",
    tags = ["arc-ignore"],
    srcs = ["function/function_reference.rs"],
)

rust_indexer_test(
    name = "function_unnamed",
    tags = ["arc-ignore"],
    srcs = ["function/function_unnamed.rs"],
)

rust_indexer_test(
    name = "impl_method_call",
    tags = ["arc-ignore"],
    srcs = ["function/impl_method_call.rs"],
)

rust_indexer_test(
    name = "method_instance_call",
    tags = ["arc-ignore"],
    srcs = ["function/method_instance_call.rs"],
)

rust_indexer_test(
    name = "method_static_call",
    tags = ["arc-ignore"],
    srcs = ["function/method_static_call.rs"],
)

rust_indexer_test(
    name = "enum_decl",
    tags = ["arc-ignore"],
    srcs = ["variants/enum_decl.rs"],
)

rust_indexer_test(
    name = "enum_pat",
    tags = ["arc-ignore"],
    srcs = ["variants/enum_pat.rs"],
)

rust_indexer_test(
    name = "struct_record",
    tags = ["arc-ignore"],
    srcs = ["variants/struct_record.rs"],
)

rust_indexer_test(
    name = "struct_record_pat",
    tags = ["arc-ignore"],
    srcs = ["variants/struct_record_pat.rs"],
)

rust_indexer_test(
    name = "struct_tuple",
    tags = ["arc-ignore"],
    srcs = ["variants/struct_tuple.rs"],
)

rust_indexer_test(
    name = "struct_tuple_pat",
    tags = ["arc-ignore"],
    srcs = ["variants/struct_tuple_pat.rs"],
)

rust_indexer_test(
    name = "struct_unit",
    tags = ["arc-ignore"],
    srcs = ["variants/struct_unit.rs"],
)
