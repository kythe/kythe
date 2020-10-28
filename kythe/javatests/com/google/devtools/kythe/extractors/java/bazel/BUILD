load("//kythe/java/com/google/devtools/kythe/extractors/java/bazel:aspect.bzl", "extract_java")

# Run the Bazel Java extractor over itself as a spot check.
extract_java(
    name = "java_extractor_test",
    testonly = True,
    compilation = "//kythe/java/com/google/devtools/kythe/extractors/java/bazel:java_extractor",
)
