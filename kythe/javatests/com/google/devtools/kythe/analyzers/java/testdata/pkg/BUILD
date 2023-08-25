load("@rules_java//java:defs.bzl", "java_library")
load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_verifier_test")
load("//kythe/java/com/google/devtools/kythe/extractors/java/bazel:aspect.bzl", "extract_java")
load("//kythe/go/test/tools/empty_corpus_checker:empty_corpus_test.bzl", "empty_corpus_test")

java_library(
    name = "pkg",
    srcs = glob(
        ["**/*.java"],
        exclude = [
            # Exclude meta-data tests
            "Metadata.java",
            "ProtobufImplicitMetadata.java",
            "ProtobufMetadata.java",
            "ProcessingMetadata.java",
            "ProcessingProtobufMetadata.java",
            # Exclude tests of features >= Java 9 as java_library is compiled as Java 8.
            "DiamondOperations.java",
            "LocalInference.java",
            "PrivateInterfaceMethods.java",
        ],
    ),
    javacopts = [
        "-Xep:MultipleTopLevelClasses:OFF",
        "-Xep:PrivateConstructorForUtilityClass:OFF",
        "-Xep:WildcardImport:OFF",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "@maven//:com_google_code_findbugs_jsr305",
    ],
)

extract_java(
    name = "pkg_kzip",
    compilation = ":pkg",
    visibility = ["//visibility:public"],
)

# TODO(schroederc): requires related nodes generated in pipeline
# ticket_root = "kythe://kythe?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg"
# kythe_integration_test(
#     name = "cross_file_integration_test",
#     srcs = [
#         ":cross_file_tests",
#         ":files_tests",
#     ],
#     file_tickets = [
#         ticket_root + "/Files.java",
#         ticket_root + "/CrossFile.java",
#     ],
# )

java_verifier_test(
    name = "annotations_tests",
    size = "small",
    srcs = ["Annotations.java"],
)

java_verifier_test(
    name = "annotation_comments_tests",
    size = "small",
    srcs = ["AnnotationComments.java"],
)

java_verifier_test(
    name = "callgraph_tests",
    size = "small",
    srcs = ["Callgraph.java"],
)

java_verifier_test(
    name = "callgraph_ident_tests",
    size = "small",
    srcs = ["CallgraphIdent.java"],
    indexer_opts = [
        "--verbose",
        "--emit_ref_call_over_identifier",
    ],
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
    name = "scopes_tests",
    size = "small",
    srcs = ["Scopes.java"],
    indexer_opts = ["--emit_anchor_scopes"],
)

java_verifier_test(
    name = "selectors_tests",
    size = "small",
    srcs = ["Selectors.java"],
)

java_verifier_test(
    name = "deprecation_tests",
    size = "small",
    srcs = ["Deprecation.java"],
)

java_verifier_test(
    name = "deprecation_annotation_tests",
    size = "small",
    srcs = ["DeprecationAnnotations.java"],
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
    name = "jdk_tests",
    size = "small",
    srcs = ["JDK.java"],
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
    verifier_deps = [":files_tests"],
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
    verifier_deps = [":jvm_files_tests"],
)

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

# TODO(#1501): This test currently fails
java_verifier_test(
    name = "generic_method_ref_tests",
    size = "small",
    srcs = ["GenericMethodRef.java"],
)

java_verifier_test(
    name = "generics_tests",
    size = "small",
    srcs = ["Generics.java"],
    indexer_opts = [
        "--verbose",
    ],
    visibility = ["//kythe/go/serving/tools/testdata:__pkg__"],
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
    name = "rendered_code_tests",
    size = "small",
    srcs = ["RenderedCode.java"],
    resolve_code_facts = True,
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
    deps = ["@maven//:org_apache_tomcat_tomcat_annotations_api"],
)

java_verifier_test(
    name = "inline_metadata_tests",
    srcs = [
        "InlineMetadata.java",
    ],
    indexer_opts = [
        "--verbose",
        "--default_metadata_corpus=default",
    ],
    deps = ["@maven//:org_apache_tomcat_tomcat_annotations_api"],
)

java_verifier_test(
    name = "processing_metadata_tests",
    srcs = [
        "ProcessingMetadata.java",
    ],
    meta = [
        "ProcessingMetadata.java.meta",
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
    deps = ["@maven//:org_apache_tomcat_tomcat_annotations_api"],
)

java_verifier_test(
    name = "protobuf_implicit_metadata_tests",
    srcs = [
        "ProtobufImplicitMetadata.java",
    ],
    indexer_opts = [
        "--verbose",
        "--default_metadata_corpus=default",
    ],
    meta = [
        "ProtobufImplicitMetadata.java.pb.meta",
    ],
    vnames_config = "protobuf_vnames.json",
)

java_verifier_test(
    name = "processing_protobuf_metadata_tests",
    srcs = [
        "ProcessingProtobufMetadata.java",
    ],
    indexer_opts = [
        "--verbose",
        "--default_metadata_corpus=default",
    ],
    meta = [
        "ProcessingProtobufMetadata.java.pb.meta",
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

java_verifier_test(
    name = "plugin_tests",
    size = "small",
    srcs = ["PluginTests.java"],
    load_plugin = "//kythe/javatests/com/google/devtools/kythe/analyzers/java:test_plugin",
)

java_verifier_test(
    name = "static_variable_tests",
    size = "small",
    srcs = ["StaticVariables.java"],
)

java_verifier_test(
    name = "visibility_tests",
    size = "small",
    srcs = ["Visibility.java"],
)

java_verifier_test(
    name = "modifier_tests",
    size = "small",
    srcs = ["Modifiers.java"],
)

java_verifier_test(
    name = "package_tests",
    size = "small",
    srcs = [
        "Packages.java",
        "package-info.java",
    ],
    deps = ["@maven//:com_google_code_findbugs_jsr305"],
)

java_verifier_test(
    name = "jvm_tests",
    size = "small",
    srcs = ["Jvm.java"],
    extra_goals = ["//kythe/javatests/com/google/devtools/kythe/analyzers/jvm/testdata:jvm_nodes.kythe_verifier.txt"],
    verifier_opts = [
        "--ignore_dups",
        "--convert_marked_source",
        "--nofile_vnames",
    ],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/jvm/testdata:__pkg__"],
)

java_verifier_test(
    name = "jvm_reference_tests",
    timeout = "long",
    srcs = ["JvmCrossFile.java"],
    verifier_deps = [":files_tests"],
)

java_verifier_test(
    name = "diamond_operator_test",
    size = "small",
    srcs = ["DiamondOperations.java"],
    extractor_opts = [
        "-source",
        "9",
        "-target",
        "9",
    ],
)

java_verifier_test(
    name = "local_inference_test",
    size = "small",
    srcs = ["LocalInference.java"],
    extractor_opts = [
        "-source",
        "11",
        "-target",
        "11",
    ],
    # TODO(#3746): We don't emit any edges for inferred locals.
    tags = ["manual"],
)

java_verifier_test(
    name = "private_interface_methods_test",
    size = "small",
    srcs = ["PrivateInterfaceMethods.java"],
    extractor_opts = [
        "-source",
        "9",
        "-target",
        "9",
    ],
)

java_verifier_test(
    name = "release9_test",
    size = "small",
    srcs = [
        "Classes.java",
        "Definitions.java",
    ],
)

# Duplicate a few tests to extract/verifier native Bazel Java compilation targets
java_library(
    name = "classes",
    srcs = ["Classes.java"],
)

java_verifier_test(
    name = "classes_xa_test",
    compilation = ":classes",
)

java_library(
    name = "files",
    srcs = ["Files.java"],
)

java_verifier_test(
    name = "files_xa_test",
    compilation = ":files",
)

java_library(
    name = "cross_file",
    srcs = ["CrossFile.java"],
    javacopts = [
        "-Xep:PrivateConstructorForUtilityClass:OFF",
        "-Xep:WildcardImport:OFF",
    ],
    deps = [":files"],
)

java_verifier_test(
    name = "cross_file_xa_test",
    compilation = ":cross_file",
    verifier_deps = [":files_xa_test"],
)

java_library(
    name = "auto_values",
    srcs = ["AutoValues.java"],
    deps = ["//kythe/java/com/google/devtools/kythe/common:autovalue"],
)

java_verifier_test(
    name = "auto_values_test",
    compilation = ":auto_values",
    load_plugin = "//kythe/java/com/google/devtools/kythe/analyzers/java:auto_value_plugin",
)

java_library(
    name = "special_members",
    srcs = ["SpecialMembers.java"],
)

java_verifier_test(
    name = "special_members_test",
    compilation = ":special_members",
)

java_library(
    name = "abstract_class",
    srcs = ["AbstractClass.java"],
)

java_verifier_test(
    name = "abstract_class_test",
    compilation = ":abstract_class",
)

java_library(
    name = "default_corpus",
    srcs = ["DefaultCorpus.java"],
)

java_verifier_test(
    name = "default_corpus_test",
    compilation = ":default_corpus",
    indexer_opts = [
        "--verbose",
        "--use_compilation_corpus_as_default",
    ],
)

java_library(
    name = "writes",
    srcs = ["Writes.java"],
)

java_verifier_test(
    name = "writes_test",
    compilation = ":writes",
)

empty_corpus_test(
    name = "empty_corpus_test",
    # TODO(justinbuchanan): adjust extractor to assign 'kythe' instead of
    # 'bazel_tools' corpus to files such java/lang/Object.java, etc.
    allowed_corpora = [
        "bazel_tools",
        "kythe",
    ],
    entries = ":default_corpus_test_entries.entries.gz",
)
