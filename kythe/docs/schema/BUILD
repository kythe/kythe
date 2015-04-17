load("//tools/build_rules/doc", "asciidoc")

package(default_visibility = ["//visibility:public"])

asciidoc(
    name = "schema",
    src = "schema.txt",
    confs = [
        "kythe-filter.conf",
    ],
    data = [
        "example.sh",
        "example-cxx.sh",
        "example-java.sh",
        "java-schema-file-data-template.FileData",
        "java-schema-unit-template.CompilationUnit",
    ],
    tools = [
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/verifier",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
    ],
)
