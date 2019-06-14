load(":asciidoc_with_verifier.bzl", "asciidoc_with_verifier", "build_example_sh")

package(default_visibility = ["//kythe:default_visibility"])

build_example_sh()

asciidoc_with_verifier(
    name = "schema",
    src = "schema.txt",
    tags = ["manual"],
)

asciidoc_with_verifier(
    name = "callgraph",
    src = "callgraph.txt",
)

asciidoc_with_verifier(
    name = "verifierstyle",
    src = "verifierstyle.txt",
)

asciidoc_with_verifier(
    name = "indexing-generated-code",
    src = "indexing-generated-code.txt",
)

asciidoc_with_verifier(
    name = "writing-an-indexer",
    src = "writing-an-indexer.txt",
)

asciidoc_with_verifier(
    name = "marked-source",
    src = "marked-source.txt",
)
