load("//tools:build_rules/kythe.bzl", "java_verifier_test", "kythe_integration_test")

ticket_root = "kythe://kythe?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg"

kythe_integration_test(
    name = "cross_file_integration_test",
    srcs = [
        ":cross_file_tests",
        ":files_tests",
    ],
    file_tickets = [
        ticket_root + "/Files.java",
        ticket_root + "/CrossFile.java",
    ],
)

java_verifier_test(
    name = "annotations_tests",
    size = "small",
    srcs = ["Annotations.java"],
)

java_verifier_test(
    name = "callgraph_tests",
    size = "small",
    srcs = ["Callgraph.java"],
)

java_verifier_test(
    name = "definitions_tests",
    size = "small",
    srcs = ["Definitions.java"],
)

java_verifier_test(
    name = "jvm_definitions_tests",
    size = "small",
    srcs = ["Definitions.java"],
    indexer_opts = [
        "--verbose",
        "--emit_jvm_signatures",
    ],
)

java_verifier_test(
    name = "selectors_tests",
    size = "small",
    srcs = ["Selectors.java"],
)

java_verifier_test(
    name = "comments_tests",
    size = "small",
    srcs = ["Comments.java"],
)

java_verifier_test(
    name = "classes_tests",
    size = "small",
    srcs = ["Classes.java"],
)

java_verifier_test(
    name = "enums_tests",
    size = "small",
    srcs = ["Enums.java"],
)

java_verifier_test(
    name = "interfaces_tests",
    size = "small",
    srcs = ["Interfaces.java"],
)

java_verifier_test(
    name = "inheritance_tests",
    size = "small",
    srcs = ["Inheritance.java"],
)

java_verifier_test(
    name = "imports_tests",
    size = "small",
    srcs = [
        "Imports.java",
        "StaticMethods.java",
    ],
)

java_verifier_test(
    name = "files_tests",
    size = "small",
    srcs = ["Files.java"],
)

java_verifier_test(
    name = "cross_file_tests",
    size = "small",
    srcs = ["CrossFile.java"],
    deps = [":files_tests"],
)

java_verifier_test(
    name = "jvm_files_tests",
    size = "small",
    srcs = ["Files.java"],
    indexer_opts = [
        "--verbose",
        "--emit_jvm_signatures",
    ],
)

java_verifier_test(
    name = "jvm_cross_file_tests",
    size = "small",
    srcs = ["CrossFile.java"],
    indexer_opts = [
        "--verbose",
        "--emit_jvm_signatures",
    ],
    deps = [":jvm_files_tests"],
)

# TODO This test currently fails: https://phabricator-dot-kythe-repo.appspot.com/T185
java_verifier_test(
    name = "generic_class_tests",
    size = "small",
    srcs = ["GenericClass.java"],
    tags = [
        "arc-ignore",
        "manual",
    ],
)

java_verifier_test(
    name = "generic_comment_tests",
    size = "small",
    srcs = ["GenericComments.java"],
)

java_verifier_test(
    name = "generic_method_tests",
    size = "small",
    srcs = ["GenericMethod.java"],
)

# TODO This test currently fails: https://phabricator-dot-kythe-repo.appspot.com/T185
java_verifier_test(
    name = "generic_method_ref_tests",
    size = "small",
    srcs = ["GenericMethodRef.java"],
    tags = [
        "arc-ignore",
        "manual",
    ],
)

java_verifier_test(
    name = "generics_tests",
    size = "small",
    srcs = ["Generics.java"],
    visibility = ["//kythe/go/serving/tools/testdata:__pkg__"],
)

java_verifier_test(
    name = "names_tests",
    size = "small",
    srcs = ["Names.java"],
)

java_verifier_test(
    name = "marked_source_tests",
    size = "small",
    srcs = ["MarkedSource.java"],
    verifier_opts = [
        "--ignore_dups",
        "--convert_marked_source",
    ],
)

java_verifier_test(
    name = "methods_tests",
    size = "small",
    srcs = ["Methods.java"],
)

java_verifier_test(
    name = "metadata_tests",
    srcs = [
        "Metadata.java",
    ],
    meta = [
        "Metadata.java.meta",
    ],
)

java_verifier_test(
    name = "protobuf_metadata_tests",
    srcs = [
        "ProtobufMetadata.java",
    ],
    indexer_opts = [
        "--verbose",
        "--default_metadata_corpus=default",
    ],
    meta = [
        "ProtobufMetadata.java.pb.meta",
    ],
    vnames_config = "protobuf_vnames.json",
)

java_verifier_test(
    name = "variables_tests",
    size = "small",
    srcs = ["Variables.java"],
)

java_verifier_test(
    name = "snippets_tests",
    size = "small",
    srcs = ["Snippets.java"],
)

java_verifier_test(
    name = "lambdas_tests",
    size = "small",
    srcs = ["Lambdas.java"],
)

java_verifier_test(
    name = "member_references_tests",
    size = "small",
    srcs = ["MemberReferences.java"],
)

java_verifier_test(
    name = "wildcard_method_tests",
    size = "small",
    srcs = ["WildcardMethod.java"],
)
