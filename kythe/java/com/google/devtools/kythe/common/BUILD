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
    deps = ["@com_google_auto_value//jar"],
)

java_library(
    name = "autovalue",
    exported_plugins = [":auto_plugin"],
    exports = ["@com_google_auto_value//jar"],
)
