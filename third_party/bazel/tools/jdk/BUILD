package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

filegroup(
    name = "java",
    srcs = ["//external:java"],
)

filegroup(
    name = "jar",
    srcs = ["//external:jar"],
)

filegroup(
    name = "javac",
    srcs = ["//external:javac"],
)

filegroup(
    name = "bootclasspath",
    srcs = ["//external:bootclasspath"],
)

filegroup(
    name = "extdir",
    srcs = ["//external:extdir"],
)

filegroup(
    name = "langtools",
    srcs = ["//third_party/java/jdk/langtools:javac_jar"],
)

java_import(
    name = "langtools-neverlink",
    jars = [":langtools"],
    neverlink = 1,
)

# This one is just needed because of how filegroup redirection works.
filegroup(name = "jdk-null")

filegroup(
    name = "jdk",
    srcs = [
        ":jdk-null",
        "//external:jdk-default",
    ],
)

java_toolchain(
    name = "toolchain",
    encoding = "UTF-8",
    jvm_opts = ["-client"],
    source_version = "7",
    target_version = "7",
)

exports_files([
    "JavaBuilder_deploy.jar",
    "SingleJar_deploy.jar",
    "GenClass_deploy.jar",
])

filegroup(
    name = "ijar",
    srcs = ["//third_party/ijar"],
)

filegroup(
    name = "srcs",
    srcs = ["BUILD"],  # Tools are build from the workspace for tests.
)
