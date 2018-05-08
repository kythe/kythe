package(default_visibility = ["//kythe:default_visibility"])

java_binary(
    name = "mvn_pom_preprocessor",
    srcs = [
        "MvnPomPreprocessor.java",
    ],
    main_class = "com.google.devtools.kythe.platform.tools.MvnPomPreprocessor",
    resources = ["pom_compiler_plugin_fragment.xml"],
    deps = [
        "//third_party/guava",
        "@com_beust_jcommander//jar",
    ],
)
