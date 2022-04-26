load("@rules_java//java:defs.bzl", "java_binary", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "analyzer",
    srcs = [
        "JavaEntrySets.java",
        "JavaIndexerConfig.java",
        "JavaNode.java",
        "KytheDocTreeScanner.java",
        "KytheJavacAnalyzer.java",
        "KytheTreeScanner.java",
        "MarkedSources.java",
        "MiniAnchor.java",
        "NameVisitor.java",
        "SourceText.java",
        "TreeContext.java",
    ],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.api=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    deps = [
        ":plugin",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/analyzers/jvm:jvm_graph",
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager:forwarding_standard_java_file_manager",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_beust_jcommander",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "source_text",
    srcs = ["SourceText.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "//kythe/java/com/google/devtools/kythe/util:span",
    ],
)

java_library(
    name = "plugin",
    srcs = ["Plugin.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//kythe/proto:storage_java_proto",
    ],
)

java_binary(
    name = "indexer",
    srcs = ["JavaIndexer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        ":plugin",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:stream_fact_emitter",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/kzip",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_beust_jcommander",
        "@maven//:com_google_flogger_flogger",
    ],
)

java_library(
    name = "auto_value_plugin",
    srcs = [
        "AutoValuePlugin.java",
        "ResolvedAutoValue.java",
    ],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.api=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    deps = [
        ":plugin",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
    ],
)
