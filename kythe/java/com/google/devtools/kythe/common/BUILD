load("@io_bazel_rules_java//java:defs.bzl", "java_library", "java_plugin")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "flogger",
    exports = ["@com_google_common_flogger//api"],
    runtime_deps = ["@com_google_common_flogger//api:system_backend"],
)

java_plugin(
    name = "auto_plugin",
    processor_class = "com.google.auto.value.processor.AutoValueProcessor",
    deps = [
        "@com_google_auto_auto_common//jar",
        "@com_google_auto_value_auto_value//jar",
    ],
)

java_library(
    name = "autovalue",
    exported_plugins = [":auto_plugin"],
    exports = [
        "@com_google_auto_value_auto_value//jar",
        "@javax_annotation_jsr250_api//jar",
    ],
)

java_plugin(
    name = "auto_service_plugin",
    processor_class = "com.google.auto.service.processor.AutoServiceProcessor",
    deps = [
        "//third_party/guava",
        "@com_google_auto_auto_common//jar",
        "@com_google_auto_service_auto_service//jar",
    ],
)

java_library(
    name = "autoservice",
    exported_plugins = [":auto_service_plugin"],
    exports = [
        "@com_google_auto_service_auto_service//jar",
        "@javax_annotation_jsr250_api//jar",
    ],
)
