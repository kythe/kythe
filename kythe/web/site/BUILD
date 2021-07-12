load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@bazelruby_rules_ruby//ruby:defs.bzl", "ruby_binary")
load(":site_docs.bzl", "jekyll_build", "site_docs")

ruby_binary(
    name = "doc_header",
    srcs = ["doc_header.rb"],
    tags = ["manual"],
    visibility = ["//visibility:public"],
    deps = ["@website_bundle//:asciidoctor"],
)

site_docs(
    name = "_docs",
    srcs = [
        "//kythe/docs:exploring-kythe-sample-web-ui",
        "//kythe/docs:kythe-compatible-compilers",
        "//kythe/docs:kythe-compilation-database",
        "//kythe/docs:kythe-configurable-extraction",
        "//kythe/docs:kythe-kzip",
        "//kythe/docs:kythe-overview",
        "//kythe/docs:kythe-storage",
        "//kythe/docs:kythe-uri-spec",
        "//kythe/docs:kythe-verifier",
        "//kythe/docs:kythes-command-line-tool",
        "//kythe/docs:modeling-libraries",
        "//kythe/docs:schema-overview",
    ],
    tags = ["manual"],
    deps = [":schema"],
)

site_docs(
    name = "schema",
    srcs = [
        "//kythe/docs/schema",
        "//kythe/docs/schema:callgraph",
        "//kythe/docs/schema:indexing-generated-code",
        "//kythe/docs/schema:influences-relation",
        "//kythe/docs/schema:marked-source",
        "//kythe/docs/schema:modules",
        "//kythe/docs/schema:verifierstyle",
        "//kythe/docs/schema:writing-an-indexer",
    ],
    rename_files = {
        "schema.html": "index.html",
    },
    tags = ["manual"],
)

jekyll_build(
    name = "site",
    srcs = [":site_sources"],
    tags = ["manual"],
)

filegroup(
    name = "site_sources",
    srcs = glob(
        ["**"],
        exclude = [
            "README.adoc",
            "Gemfile.lock",
            "Gemfile",
            "build.sh",
            "sync_docs.sh",
            "check_links*.sh",
            "site_docs.bzl",
            "BUILD",
            "doc_header.rb",
            "_site*/**",
            "_vendor/**",
            "_docs/**",
            ".bundle/**",
            "asciidoc-pygments.css",
        ],
    ) + [":_docs"],
    tags = ["manual"],
)

filegroup(
    name = "site_files",
    srcs = [
        ":site",
        ":site_sources",
    ],
    tags = ["manual"],
)

sh_test(
    name = "check_links_test",
    srcs = ["check_links_test.sh"],
    data = [
        ":site_files",
        # Similarly :bin/jekyll is a plain file, :jekyll is a ruby library
        # and we need the toolchain.
        "@org_ruby_lang_ruby_toolchain//:ruby_bin",
        "@website_bundle//:bin/jekyll",
        "@website_bundle//:jekyll",
    ],
    env = {
        "JEKYLL_BIN": "$(location @website_bundle//:bin/jekyll)",
    },
    tags = [
        "arc-ignore",
        "local",
        "manual",
    ],
)

# Can be run via `bazel run //kythe/website:serve`
# to manually verify the website contents.
sh_binary(
    name = "serve",
    srcs = ["serve.sh"],
    data = [
        ":site_files",
        # Similarly :bin/jekyll is a plain file, :jekyll is a ruby library
        # and we need the toolchain.
        "@org_ruby_lang_ruby_toolchain//:ruby_bin",
        "@website_bundle//:bin/jekyll",
        "@website_bundle//:jekyll",
    ],
    env = {
        "JEKYLL_BIN": "$(location @website_bundle//:bin/jekyll)",
    },
    tags = ["manual"],
)

bzl_library(
    name = "site_docs_bzl",
    srcs = ["site_docs.bzl"],
    deps = [
        "//kythe/docs:asciidoc_bzl",
        "@bazel_skylib//lib:paths",
        "@bazel_skylib//lib:shell",
    ],
)
