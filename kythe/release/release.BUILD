package(default_visibility = ["//visibility:public"])

exports_files([
    "LICENSE",
    "extractors/*",
    "indexers/*",
    "proto/*",
    "tools/*",
])

load(":extractors.bzl", "extractor_action")
load(":vnames.bzl", "construct_vnames_config")

config_setting(
    name = "assign_external_projects_to_separate_corpora",
    values = {
        "define": "kythe_assign_external_projects_to_separate_corpora=true",
    },
)

construct_vnames_config(
    name = "vnames_config",
    srcs = select({
        "//conditions:default": [
            # by default, the simple vname rules are used, which map everything
            # to the corpus set via `--define kythe_corpus=<my corpus>`.
            "simple_vnames.json",
        ],
        ":assign_external_projects_to_separate_corpora": [
            "vnames.cxx.json",
            "vnames.go.json",
            "vnames.java.json",
            "vnames.json",
        ],
    }),
)

# Clone of default Java proto toolchain with "annotate_code" enabled for
# cross-language metadata file generation.
proto_lang_toolchain(
    name = "java_proto_toolchain",
    command_line = "--java_out=annotate_code,shared,immutable:$(OUT)",
    runtime = ":protobuf",
)

java_import(
    name = "jsr250",
    jars = ["jsr250-api-1.0.jar"],
)

java_library(
    name = "protobuf",
    visibility = ["//visibility:private"],
    exports = [
        ":jsr250",
        "@com_google_protobuf//:protobuf_java",
    ],
    runtime_deps = [
        "@com_google_protobuf//:protobuf_java",
    ],
)

# Clone of default C++ proto toolchain with "annotate_headers" enabled for
# cross-language metadata file generation.
proto_lang_toolchain(
    name = "cc_proto_toolchain",
    blacklisted_protos = [
        "@com_google_protobuf//:well_known_protos",
    ],
    command_line = "--$(PLUGIN_OUT)=:$(OUT)",
    plugin = ":cc_proto_metadata_plugin",
    runtime = "@com_google_protobuf//:protobuf",
)

cc_binary(
    name = "cc_proto_metadata_plugin",
    srcs = ["tools/proto_metadata_plugin.cc"],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ]
)

filegroup(
    name = "bazel_cxx_extractor",
    srcs = ["extractors/bazel_cxx_extractor"],
)

java_binary(
    name = "bazel_java_extractor",
    main_class = "com.google.devtools.kythe.extractors.java.bazel.JavaExtractor",
    runtime_deps = [
        "extractors/bazel_java_extractor.jar",
        "jsr250-api-1.0.jar",
    ],
)

java_binary(
    name = "bazel_jvm_extractor",
    main_class = "com.google.devtools.kythe.extractors.jvm.bazel.BazelJvmExtractor",
    runtime_deps = ["extractors/bazel_jvm_extractor.jar"],
)

filegroup(
    name = "bazel_go_extractor",
    srcs = ["extractors/bazel_go_extractor"],
)

filegroup(
    name = "bazel_extract_kzip",
    srcs = ["extractors/bazel_extract_kzip"],
)

filegroup(
    name = "bazel_proto_extractor",
    srcs = ["extractors/bazel_proto_extractor"],
)

extractor_action(
    name = "extract_kzip_cxx",
    args = [
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID).cxx.kzip)",
        "$(location :vnames_config)",
    ],
    data = [":vnames_config"],
    extractor = ":bazel_cxx_extractor",
    mnemonics = ["CppCompile"],
    output = "$(ACTION_ID).cxx.kzip",
)

extractor_action(
    name = "extract_kzip_java",
    args = [
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID).java.kzip)",
        "$(location :vnames_config)",
    ],
    data = [":vnames_config"],
    extractor = ":bazel_java_extractor",
    mnemonics = ["Javac"],
    output = "$(ACTION_ID).java.kzip",
)

extractor_action(
    name = "extract_kzip_jvm",
    args = [
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID).jvm.kzip)",
        "$(location :vnames_config)",
    ],
    data = [":vnames_config"],
    extractor = ":bazel_jvm_extractor",
    mnemonics = ["JavaIjar"],
    output = "$(ACTION_ID).jvm.kzip",
)

# We only support Bazel rules_go 0.19.0 and up.
extractor_action(
    name = "extract_kzip_go",
    args = [
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID).go.kzip)",
        "$(location :vnames_config)",
    ],
    data = [":vnames_config"],
    extractor = ":bazel_go_extractor",
    mnemonics = ["GoCompilePkg"],
    output = "$(ACTION_ID).go.kzip",
)

extractor_action(
    name = "extract_kzip_typescript",
    args = [
        "--extra_action=$(EXTRA_ACTION_FILE)",
        "--include='\\.(js|json|tsx?|d\\.ts)$$'",
        "--language=typescript",
        "--output=$(output $(ACTION_ID).typescript.kzip)",
        "--rules=$(location :vnames_config)",
        "--scoped=true",
        "--source='\\.ts$$'",
    ],
    data = [":vnames_config"],
    extractor = ":bazel_extract_kzip",
    mnemonics = [
        "TypeScriptCompile",
        "AngularTemplateCompile",
    ],
    output = "$(ACTION_ID).typescript.kzip",
)

extractor_action(
    name = "extract_kzip_protobuf",
    args = [
        "--extra_action=$(EXTRA_ACTION_FILE)",
        "--language=protobuf",
        "--rules=$(location :vnames_config)",
        "--output=$(output $(ACTION_ID).protobuf.kzip)",
    ],
    data = [":vnames_config"],
    extractor = ":bazel_proto_extractor",
    mnemonics = ["GenProtoDescriptorSet"],
    output = "$(ACTION_ID).protobuf.kzip",
)
