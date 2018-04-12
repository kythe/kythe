package(default_visibility = ["//kythe:default_visibility"])

load(":rust_indexer_test.bzl", "rust_indexer_test")

test_suite(
    name = "testdata",
    tags = [
        "arc-ignore",
        "manual",
    ],
    tests = [
        ":basic",
        ":function",
        ":variants",
    ],
)

test_suite(
    name = "basic",
    tags = [
        "arc-ignore",
        "manual",
    ],
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
    tags = [
        "arc-ignore",
        "manual",
    ],
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
    tags = [
        "arc-ignore",
        "manual",
    ],
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
    srcs = ["basic/const.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "file_node",
    srcs = ["basic/file_node.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "file_content",
    # buildifier: leave-alone
    srcs = [
        "basic/file_content.rs",
        "basic/a.rs",
    ],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "static",
    srcs = ["basic/static.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "var_ref",
    srcs = ["basic/var_ref.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "vardecl_local_anchor",
    srcs = ["basic/vardecl_local_anchor.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "vardecl_local_shadow",
    srcs = ["basic/vardecl_local_shadow.rs"],
    expect_fail_verify = True,
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "vardecl_local_destructure",
    srcs = ["basic/vardecl_local_destructure.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "function_call",
    srcs = ["function/function_call.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "function_decl",
    srcs = ["function/function_decl.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "function_for_loop",
    srcs = ["function/function_for_loop.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "function_params",
    srcs = ["function/function_params.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "function_reference",
    srcs = ["function/function_reference.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "function_unnamed",
    srcs = ["function/function_unnamed.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "impl_method_call",
    srcs = ["function/impl_method_call.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "method_instance_call",
    srcs = ["function/method_instance_call.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "method_static_call",
    srcs = ["function/method_static_call.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "enum_decl",
    srcs = ["variants/enum_decl.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "enum_pat",
    srcs = ["variants/enum_pat.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "struct_record",
    srcs = ["variants/struct_record.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "struct_record_pat",
    srcs = ["variants/struct_record_pat.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "struct_tuple",
    srcs = ["variants/struct_tuple.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "struct_tuple_pat",
    srcs = ["variants/struct_tuple_pat.rs"],
    tags = ["arc-ignore"],
)

rust_indexer_test(
    name = "struct_unit",
    srcs = ["variants/struct_unit.rs"],
    tags = ["arc-ignore"],
)
