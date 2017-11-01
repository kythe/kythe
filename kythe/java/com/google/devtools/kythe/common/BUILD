package(default_visibility = ["//kythe:default_visibility"])

# TODO(jrtom): rename this to "logger" or something less generic
java_library(
    name = "common",
    srcs = ["FormattingLogger.java"],
    deps = ["@com_google_guava_guava//jar"],
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
    exports = ["@com_google_auto_value_auto_value//jar"],
)

java_plugin(
    name = "auto_service_plugin",
    processor_class = "com.google.auto.service.processor.AutoServiceProcessor",
    deps = [
        "@com_google_auto_auto_common//jar",
        "@com_google_auto_service_auto_service//jar",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "autoservice",
    exported_plugins = [":auto_service_plugin"],
    exports = ["@com_google_auto_service_auto_service//jar"],
)
