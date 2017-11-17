package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "dagger-kythe-plugin",
    srcs = glob(["*.java"]),
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:plugin",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//kythe/proto:common_proto_java",
        "//kythe/proto:storage_proto_java",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        "@com_google_auto_auto_common//jar",
        "@com_google_dagger//java/dagger:core",
        "@com_google_dagger//java/dagger/internal/codegen:base",
        "@com_google_dagger//java/dagger/internal/codegen:binding",
        "@com_google_dagger//java/dagger/internal/codegen:processor",
        "@com_google_dagger//java/dagger/internal/codegen:validation",
        "@com_google_dagger//java/dagger/producers",
        "@com_google_guava_guava//jar",
        "@com_squareup_javapoet//jar",
    ],
)
