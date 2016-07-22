package(default_visibility = ["//kythe:default_visibility"])

load("rust_indexer_test", "rust_indexer_test")

test_suite(
    name = "testdata",
    tags = ["manual"],
    tests = [
        ":basic",
        ":function",
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
        ":impl_method_call",
        ":method_instance_call",
        ":method_static_call",
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
