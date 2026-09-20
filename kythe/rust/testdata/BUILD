rust_verifier_test(
    name = "modules_test",
    srcs = [
        "external.rs",
        "modules.rs",
    ],
    crate_root = "modules.rs",
)

rust_verifier_test(
    name = "fn_items_test",
    srcs = ["fn_items.rs"],
)

rust_verifier_test(
    name = "struct_items_test",
    srcs = ["struct_items.rs"],
)

rust_verifier_test(
    name = "enum_items_test",
    srcs = ["enum_items.rs"],
)

rust_verifier_test(
    name = "union_items_test",
    srcs = ["union_items.rs"],
)

rust_verifier_test(
    name = "const_items_test",
    srcs = ["const_items.rs"],
)

rust_verifier_test(
    name = "static_items_test",
    srcs = ["static_items.rs"],
)

rust_verifier_test(
    name = "trait_items_test",
    srcs = ["trait_items.rs"],
)

rust_verifier_test(
    name = "trait_alias_items_test",
    srcs = ["trait_alias_items.rs"],
)

rust_verifier_test(
    name = "type_alias_items_test",
    srcs = ["type_alias_items.rs"],
)

rust_verifier_test(
    name = "inherent_impl_items_test",
    srcs = ["inherent_impl_items.rs"],
)

rust_verifier_test(
    name = "trait_impl_items_test",
    srcs = ["trait_impl_items.rs"],
)

rust_verifier_test(
    name = "macro_rules_items_test",
    srcs = ["macro_rules_items.rs"],
)

rust_verifier_test(
    name = "generic_params_test",
    srcs = ["generic_params.rs"],
)

rust_verifier_test(
    name = "macro_expn_test",
    srcs = ["macro_expn_test.rs"],
)

rust_verifier_test(
    name = "nested_items_test",
    srcs = ["nested_items.rs"],
)

rust_verifier_test(
    name = "scope_vnames_test",
    srcs = ["scope_vnames.rs"],
)

rust_verifier_test(
    name = "diagnostics_test",
    srcs = ["diagnostics.rs"],
)

# buildifier: disable=no-effect
[
    rust_verifier_test(
        name = "std_test",
        srcs = ["std.rs"],
        deps = [":core_lib"],
    ),
    rust_library(
        name = "core_lib",
        srcs = ["core.rs"],
    ),
]

rust_verifier_test(
    name = "namespaces_test",
    srcs = ["namespaces.rs"],
)

rust_verifier_test(
    name = "patterns_test",
    srcs = ["patterns.rs"],
)

rust_verifier_test(
    name = "operators_test",
    srcs = ["operators.rs"],
)

rust_verifier_test(
    name = "extern_c_test",
    srcs = ["extern_c.rs"],
)

rust_verifier_test(
    name = "attrs_test",
    srcs = ["attrs.rs"],
)

rust_verifier_test(
    name = "doc_comments_test",
    srcs = ["doc_comments.rs"],
)

[
    rust_verifier_test(
        name = "transitive_deps_test",
        srcs = ["transitive_deps.rs"],
        deps = [":transitive_1"],
    ),
    rust_library(
        name = "transitive_1",
        srcs = ["transitive_1.rs"],
        deps = [":transitive_2"],
    ),
    rust_library(
        name = "transitive_2",
        srcs = ["transitive_2.rs"],
    ),
]

[
    rust_verifier_test(
        name = "reexport_test",
        srcs = ["reexport.rs"],
        deps = [":reexport_imported"],
    ),
    rust_library(
        name = "reexport_imported",
        srcs = ["reexport_imported.rs"],
    ),
]

rust_verifier_test(
    name = "import_test",
    srcs = ["import.rs"],
    deps = [":dep"],
)

rust_library(
    name = "dep",
    srcs = ["dep.rs"],
)

