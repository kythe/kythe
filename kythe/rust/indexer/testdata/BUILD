package(default_visibility = ["//kythe:default_visibility"])

load("rust_indexer_test", "rust_indexer_test")

test_suite(
    name = "testdata",
    tags = ["manual"],
    tests = [
        ":basic",
        ":function",
        ":variants",
    ],
)

test_suite(
    name = "basic",
    tags = ["manual"],
    tests = [
        ":const",
        ":file_content",
        ":file_node",
        ":static",
        ":var_ref",
        ":vardecl_local_anchor",
        ":vardecl_local_shadow",
        ":vardecl_local_destructure",
    ],
)

test_suite(
    name = "function",
    tags = ["manual"],
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
    tags = ["manual"],
    tests = [
        ":enum_decl",
        ":enum_pat",
        ":struct_record",
        ":struct_record_pat",
        ":struct_unit",
        ":struct_tuple",
        ":struct_tuple_pat",
    ],
)

rust_indexer_test(
    name = "const",
    srcs = ["basic/const.rs"],
)

rust_indexer_test(
    name = "file_node",
    srcs = ["basic/file_node.rs"],
)

rust_indexer_test(
    name = "file_content",
    # buildifier: leave-alone
    srcs = [
        "basic/file_content.rs",
        "basic/a.rs",
    ],
)

rust_indexer_test(
    name = "static",
    srcs = ["basic/static.rs"],
)


rust_indexer_test(
    name = "var_ref",
    srcs = ["basic/var_ref.rs"],
)

rust_indexer_test(
    name = "vardecl_local_anchor",
    srcs = ["basic/vardecl_local_anchor.rs"],
)

rust_indexer_test(
    name = "vardecl_local_shadow",
    expect_fail_verify = True,
    srcs = ["basic/vardecl_local_shadow.rs"],
)

rust_indexer_test(
    name = "vardecl_local_destructure",
    srcs = ["basic/vardecl_local_destructure.rs"],
)

rust_indexer_test(
    name = "function_call",
    srcs = ["function/function_call.rs"],
)

rust_indexer_test(
    name = "function_decl",
    srcs = ["function/function_decl.rs"],
)

rust_indexer_test(
    name = "function_for_loop",
    srcs = ["function/function_for_loop.rs"],
)

rust_indexer_test(
    name = "function_params",
    srcs = ["function/function_params.rs"],
)

rust_indexer_test(
    name = "function_reference",
    srcs = ["function/function_reference.rs"],
)

rust_indexer_test(
    name = "function_unnamed",
    srcs = ["function/function_unnamed.rs"],
)

rust_indexer_test(
    name = "impl_method_call",
    srcs = ["function/impl_method_call.rs"],
)

rust_indexer_test(
    name = "method_instance_call",
    srcs = ["function/method_instance_call.rs"],
)

rust_indexer_test(
    name = "method_static_call",
    srcs = ["function/method_static_call.rs"],
)

rust_indexer_test(
    name = "enum_decl",
    srcs = ["variants/enum_decl.rs"],
)

rust_indexer_test(
    name = "enum_pat",
    srcs = ["variants/enum_pat.rs"],
)

rust_indexer_test(
    name = "struct_record",
    srcs = ["variants/struct_record.rs"],
)

rust_indexer_test(
    name = "struct_record_pat",
    srcs = ["variants/struct_record_pat.rs"],
)

rust_indexer_test(
    name = "struct_tuple",
    srcs = ["variants/struct_tuple.rs"],
)

rust_indexer_test(
    name = "struct_tuple_pat",
    srcs = ["variants/struct_tuple_pat.rs"],
)

rust_indexer_test(
    name = "struct_unit",
    srcs = ["variants/struct_unit.rs"],
)
