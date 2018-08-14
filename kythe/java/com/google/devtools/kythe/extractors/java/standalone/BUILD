package(default_visibility = ["//kythe:default_visibility"])

exports_files(["javac-wrapper.sh"])

java_binary(
    name = "javac_extractor",
    srcs = [
        "AbstractJavacWrapper.java",
        "Javac8Wrapper.java",
    ],
    data = [
        "//third_party/javac:java_compiler_jar",
        "//third_party/javac:jdk_compiler_jar",
    ],
    jvm_flags = [
        "--add-opens=jdk.compiler/com.sun.tools.javac.api=java.compiler",
        "--patch-module=java.compiler=$${RUNPATH}$(location //third_party/javac:java_compiler_jar)",
        "--patch-module=jdk.compiler=$${RUNPATH}$(location //third_party/javac:jdk_compiler_jar)",
    ],
    main_class = "com.google.devtools.kythe.extractors.java.standalone.Javac8Wrapper",
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_protobuf//:protobuf_java",
    ],
)
