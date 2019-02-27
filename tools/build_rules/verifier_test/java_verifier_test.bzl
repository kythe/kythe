# Copyright 2019 The Kythe Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load(
    ":verifier_test.bzl",
    "KytheVerifierSources",
    "extract",
    "index_compilation",
    "verifier_test",
)
load(
    "@io_kythe_lang_proto//kythe/cxx/indexer/proto/testdata:proto_verifier_test.bzl",
    "proto_extract_kzip",
)

KytheGeneratedSourcesInfo = provider(
    doc = "Generated Java source directory and jar.",
    fields = {
        "srcjar": "Source jar of generated files.",
        "dir": "Directory of unpacked files in srcjar.",
    },
)

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)

def _filter_java_sources(src):
    if type(src) != "File":
        return src
    src = src.path
    if src.endswith(".java"):
        return src

def _java_extract_kzip_impl(ctx):
    deps = []
    for dep in ctx.attr.deps:
        deps += [dep[JavaInfo]]

    srcs = []
    srcjars = []
    dirs = []
    for src in ctx.attr.srcs:
        if KytheGeneratedSourcesInfo in src:
            srcjars += [src[KytheGeneratedSourcesInfo].srcjar]
            dirs += [src[KytheGeneratedSourcesInfo].dir]
        else:
            srcs += [src.files]
    srcs = depset(transitive = srcs).to_list()

    # Actually compile the sources to be used as a dependency for other tests
    jar = ctx.actions.declare_file(ctx.outputs.kzip.basename + ".jar", sibling = ctx.outputs.kzip)

    # Use find_java_toolchain / find_java_runtime_toolchain after the next Bazel release,
    # see: https://github.com/bazelbuild/bazel/issues/7186
    if hasattr(java_common, "JavaToolchainInfo"):
        java_toolchain = ctx.attr._java_toolchain[java_common.JavaToolchainInfo]
        host_javabase = ctx.attr._host_javabase[java_common.JavaRuntimeInfo]
    else:
        java_toolchain = ctx.attr._java_toolchain
        host_javabase = ctx.attr._host_javabase

    java_info = java_common.compile(
        ctx,
        javac_opts = ctx.attr.opts,
        java_toolchain = java_toolchain,
        host_javabase = host_javabase,
        source_jars = srcjars,
        source_files = srcs,
        output = jar,
        deps = deps,
    )

    jars = depset(transitive = [dep.compile_jars for dep in deps]).to_list()

    args = ctx.actions.args()
    args.add_all(ctx.attr.opts + ["-encoding", "utf-8"])
    args.add_joined("-cp", jars, join_with = ":")
    args.add_all(dirs, map_each = _filter_java_sources, expand_directories = True)

    extract(
        srcs = srcs,
        ctx = ctx,
        extractor = ctx.executable.extractor,
        kzip = ctx.outputs.kzip,
        mnemonic = "JavaExtractKZip",
        opts = args,
        vnames_config = ctx.file.vnames_config,
        deps = jars + ctx.files.data + dirs,
    )
    return [
        java_info,
        KytheVerifierSources(files = depset(srcs)),
    ]

java_extract_kzip = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
        ),
        "data": attr.label_list(
            allow_files = True,
        ),
        "extractor": attr.label(
            default = Label("@io_kythe//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor"),
            executable = True,
            cfg = "host",
        ),
        "opts": attr.string_list(),
        "vnames_config": attr.label(
            default = Label("//external:vnames_config"),
            allow_single_file = True,
        ),
        "deps": attr.label_list(
            providers = [JavaInfo],
        ),
        "_host_javabase": attr.label(
            cfg = "host",
            default = Label("@bazel_tools//tools/jdk:current_java_runtime"),
        ),
        "_java_toolchain": attr.label(
            default = Label("@bazel_tools//tools/jdk:toolchain"),
        ),
    },
    fragments = ["java"],
    host_fragments = ["java"],
    outputs = {"kzip": "%{name}.kzip"},
    implementation = _java_extract_kzip_impl,
)

_default_java_extractor_opts = [
    "-source",
    "9",
    "-target",
    "9",
]

def java_verifier_test(
        name,
        srcs,
        meta = [],
        deps = [],
        size = "small",
        tags = [],
        extractor = None,
        extractor_opts = _default_java_extractor_opts,
        indexer_opts = ["--verbose"],
        verifier_opts = ["--ignore_dups"],
        load_plugin = None,
        extra_goals = [],
        vnames_config = None,
        visibility = None):
    """Extract, analyze, and verify a Java compilation.

    Args:
      srcs: The compilation's source file inputs; each file's verifier goals will be checked
      deps: Optional list of java_verifier_test targets to be used as Java compilation dependencies
      meta: Optional list of Kythe metadata files
      extractor: Executable extractor tool to invoke (defaults to javac_extractor)
      extractor_opts: List of options passed to the extractor tool
      indexer_opts: List of options passed to the indexer tool
      verifier_opts: List of options passed to the verifier tool
      load_plugin: Optional Java analyzer plugin to load
      extra_goals: List of text files containing verifier goals additional to those in srcs
      vnames_config: Optional path to a VName configuration file
    """
    kzip = _invoke(
        java_extract_kzip,
        name = name + "_kzip",
        testonly = True,
        srcs = srcs,
        data = meta,
        extractor = extractor,
        opts = extractor_opts,
        tags = tags,
        visibility = visibility,
        vnames_config = vnames_config,
        # This is a hack to depend on the .jar producer.
        deps = [d + "_kzip" for d in deps],
    )
    indexer = "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer"
    tools = []
    if load_plugin:
        # If loaded plugins have deps, those must be included in the loaded jar
        native.java_binary(
            name = name + "_load_plugin",
            main_class = "not.Used",
            runtime_deps = [load_plugin],
        )
        load_plugin_deploy_jar = ":{}_load_plugin_deploy.jar".format(name)
        indexer_opts = indexer_opts + [
            "--load_plugin",
            "$(location {})".format(load_plugin_deploy_jar),
        ]
        tools += [load_plugin_deploy_jar]

    entries = _invoke(
        index_compilation,
        name = name + "_entries",
        testonly = True,
        indexer = indexer,
        opts = indexer_opts,
        tags = tags,
        tools = tools,
        visibility = visibility,
        deps = [kzip],
    )
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        srcs = [entries] + extra_goals,
        opts = verifier_opts,
        tags = tags,
        visibility = visibility,
        deps = [entries],
    )

def _generate_java_proto_impl(ctx):
    # Generate the Java protocol buffer sources into a directory.
    # Note: out contains .meta files with annotations for cross-language xrefs.
    out = ctx.actions.declare_directory(ctx.label.name)
    protoc = ctx.executable._protoc
    ctx.actions.run_shell(
        outputs = [out],
        inputs = ctx.files.srcs,
        tools = [protoc],
        command = "\n".join([
            "#/bin/bash",
            "set -e",
            # Creating the declared directory in this action is necessary for
            # remote execution environments.  This differs from local execution
            # where Bazel will create the directory before this action is
            # executed.
            "mkdir -p " + out.path,
            " ".join([
                protoc.path,
                "--java_out=annotate_code:" + out.path,
            ] + [src.path for src in ctx.files.srcs]),
        ]),
    )

    # Produce a source jar file for the native Java compilation in the java_extract_kzip rule.
    # Note: we can't use java_common.pack_sources because our input is a directory.
    srcjar = ctx.actions.declare_file(ctx.label.name + ".srcjar")
    args = ctx.actions.args()
    args.add_all(["--output", srcjar])
    args.add_all(["--resources", out], map_each = _filter_java_sources, expand_directories = True)
    ctx.actions.run(
        outputs = [srcjar],
        inputs = [out],
        executable = ctx.executable._singlejar,
        arguments = [args],
    )

    return [
        DefaultInfo(files = depset([out, srcjar])),
        KytheGeneratedSourcesInfo(dir = out, srcjar = srcjar),
    ]

_generate_java_proto = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
            providers = [JavaInfo],
        ),
        "_protoc": attr.label(
            default = Label("@com_google_protobuf//:protoc"),
            executable = True,
            cfg = "host",
        ),
        "_singlejar": attr.label(
            default = Label("@bazel_tools//tools/jdk:singlejar"),
            executable = True,
            cfg = "host",
        ),
    },
    implementation = _generate_java_proto_impl,
)

def java_proto_verifier_test(
        name,
        srcs,
        size = "small",
        proto_srcs = [],
        tags = [],
        java_extractor_opts = _default_java_extractor_opts,
        verifier_opts = ["--ignore_dups"],
        vnames_config = None,
        visibility = None):
    """Verify cross-language references between Java and Proto.

    Args:
      name: Name of the test.
      size: Size of the test.
      tags: Test target tags.
      visibility: Visibility of the test target.
      srcs: The compilation's Java source files; each file's verifier goals will be checked
      proto_srcs: The compilation's proto source files; each file's verifier goals will be checked
      verifier_opts: List of options passed to the verifier tool
      vnames_config: Optional path to a VName configuration file

    Returns: the label of the test.
    """
    proto_kzip = _invoke(
        proto_extract_kzip,
        name = name + "_proto_kzip",
        srcs = proto_srcs,
        tags = tags,
        visibility = visibility,
        vnames_config = vnames_config,
    )
    proto_entries = _invoke(
        index_compilation,
        name = name + "_proto_entries",
        testonly = True,
        indexer = "@io_kythe_lang_proto//kythe/cxx/indexer/proto:indexer",
        opts = ["--index_file"],
        tags = tags,
        visibility = visibility,
        deps = [proto_kzip],
    )

    _generate_java_proto(
        name = name + "_gensrc",
        srcs = proto_srcs,
    )

    kzip = _invoke(
        java_extract_kzip,
        name = name + "_java_kzip",
        srcs = srcs + [":" + name + "_gensrc"],
        opts = java_extractor_opts,
        tags = tags,
        visibility = visibility,
        vnames_config = vnames_config,
        deps = [
            "@com_google_protobuf//:protobuf_java",
            "@javax_annotation_jsr250_api//jar",
        ],
    )

    entries = _invoke(
        index_compilation,
        name = name + "_java_entries",
        testonly = True,
        indexer = "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
        opts = ["--verbose"],
        tags = tags,
        visibility = visibility,
        deps = [kzip],
    )
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        srcs = [entries, proto_entries] + proto_srcs,
        opts = verifier_opts,
        tags = tags,
        visibility = visibility,
        deps = [entries],
    )
