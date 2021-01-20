load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":asciidoc.bzl", "asciidoc")

package(default_visibility = ["//kythe:default_visibility"])

asciidoc(
    name = "influences-relation",
    src = "influences-relation.txt",
)

asciidoc(
    name = "kythe-compatible-compilers",
    src = "kythe-compatible-compilers.txt",
)

asciidoc(
    name = "kythe-overview",
    src = "kythe-overview.txt",
)

asciidoc(
    name = "kythe-storage",
    src = "kythe-storage.txt",
)

asciidoc(
    name = "kythe-uri-spec",
    src = "kythe-uri-spec.txt",
)

asciidoc(
    name = "kythe-verifier",
    src = "kythe-verifier.txt",
)

asciidoc(
    name = "kythe-compilation-database",
    src = "kythe-compilation-database.txt",
)

asciidoc(
    name = "exploring-kythe-sample-web-ui",
    src = "exploring-kythe-sample-web-ui.txt",
)

asciidoc(
    name = "kythes-command-line-tool",
    src = "kythes-command-line-tool.txt",
)

asciidoc(
    name = "modeling-libraries",
    src = "modeling-libraries.txt",
)

asciidoc(
    name = "kythe-configurable-extraction",
    src = "kythe-configurable-extraction.txt",
)

asciidoc(
    name = "kythe-kzip",
    src = "kythe-kzip.txt",
)

asciidoc(
    name = "schema-overview",
    src = "schema-overview.txt",
)

bzl_library(
    name = "asciidoc_bzl",
    srcs = ["asciidoc.bzl"],
)
