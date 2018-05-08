package(default_visibility = ["//kythe:default_visibility"])

java_binary(
    name = "mvn_pom_preprocessor",
    srcs = [
        "MvnPomPreprocessor.java",
    ],
    main_class = "com.google.devtools.kythe.platform.tools.MvnPomPreprocessor",
    resources = ["pom_compiler_plugin_fragment.xml"],
    deps = [
        "@com_beust_jcommander//jar",
        "@com_google_guava_guava//jar",
    ],
)
