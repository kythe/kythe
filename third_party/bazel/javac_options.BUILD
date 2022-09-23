java_library(
    name = "javac_options",
    srcs = [
        "javac/JavacOptions.java",
        "javac/WerrorCustomOption.java",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":autovalue",
        "@io_kythe//third_party/guava",
    ],
)

java_library(
    name = "autovalue",
    exported_plugins = [":auto_plugin"],
    exports = [
        "@maven//:com_google_auto_value_auto_value_annotations",
        "@maven//:org_apache_tomcat_tomcat_annotations_api",
    ],
)

java_plugin(
    name = "auto_plugin",
    processor_class = "com.google.auto.value.processor.AutoValueProcessor",
    deps = [
        "@maven//:com_google_auto_auto_common",
        "@maven//:com_google_auto_value_auto_value",
    ],
)
