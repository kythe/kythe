"""Rules for verifying proto indexer output"""

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
    "@io_kythe//tools/build_rules/verifier_test:verifier_test.bzl",
    "KytheVerifierSources",
    "extract",
    "index_compilation",
    "verifier_test",
)
load("@rules_proto//proto:defs.bzl", "ProtoInfo")

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)

def get_proto_files_and_proto_paths(protolibs):
    """Given a list of proto_library targets, returns:
      * a list of top-level .proto files
      * a depset of all transitively-included .proto files
      * a depset of --proto_path locations
    """
    toplevel_srcs = []
    for lib in protolibs:
        info = lib[ProtoInfo]
        for src in info.direct_sources:
            toplevel_srcs.append(src)
    all_srcs = depset([], transitive = [lib[ProtoInfo].transitive_sources for lib in protolibs])
    proto_paths = depset(
        transitive = [lib[ProtoInfo].transitive_proto_path for lib in protolibs] +
                     # Workaround for https://github.com/bazelbuild/bazel/issues/7964.
                     # Since we can't rely on ProtoInfo to provide accurate roots, generate them here.
                     [depset([
                         src.root.path
                         for src in depset(toplevel_srcs, transitive = [all_srcs], order = "postorder").to_list()
                         if src.root.path
                     ])],
        order = "postorder",
    )
    return toplevel_srcs, all_srcs, proto_paths

def _proto_extract_kzip_impl(ctx):
    toplevel_srcs, all_srcs, pathopt = get_proto_files_and_proto_paths(ctx.attr.srcs)

    args = ctx.actions.args()
    args.add("--")
    args.add_all(ctx.attr.opts)
    args.add_all(pathopt, before_each = "--proto_path")

    extract(
        srcs = toplevel_srcs,
        ctx = ctx,
        extractor = ctx.executable.extractor,
        kzip = ctx.outputs.kzip,
        mnemonic = "ProtoExtractKZip",
        opts = args,
        vnames_config = ctx.file.vnames_config,
        deps = all_srcs,
    )
    return [KytheVerifierSources(files = depset(toplevel_srcs))]

proto_extract_kzip = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
            providers = [ProtoInfo],
        ),
        "extractor": attr.label(
            default = Label("//kythe/cxx/extractor/proto:proto_extractor"),
            executable = True,
            cfg = "exec",
        ),
        "opts": attr.string_list(),
        "vnames_config": attr.label(
            default = Label("//external:vnames_config"),
            allow_single_file = True,
        ),
    },
    outputs = {"kzip": "%{name}.kzip"},
    implementation = _proto_extract_kzip_impl,
)

def proto_verifier_test(
        name,
        srcs,
        deps = [],
        size = "small",
        tags = [],
        extractor = None,
        extractor_opts = [],
        indexer_opts = [],
        verifier_opts = [],
        convert_marked_source = False,
        resolve_code_facts = False,
        vnames_config = None,
        visibility = None):
    """Extract, analyze, and verify a proto compilation.

    Args:
      name: Name of the test
      srcs: The compilation's source file inputs; each file's verifier goals will be checked
      deps: Optional list of proto_verifier_test targets to be used as proto compilation dependencies
      size: Test size
      tags: Test tags
      extractor: Executable extractor tool to invoke (defaults to protoc_extractor)
      extractor_opts: List of options passed to the extractor tool
      indexer_opts: List of options passed to the indexer tool
      verifier_opts: List of options passed to the verifier tool
      convert_marked_source: Whether the verifier should convert marked source.
      vnames_config: Optional path to a VName configuration file
      visibility: Visibility of underlying build targets
    Returns:
      Name of the test rule
    """
    kzip = _invoke(
        proto_extract_kzip,
        name = name + "_kzip",
        testonly = True,
        srcs = srcs,
        extractor = extractor,
        opts = extractor_opts,
        tags = tags,
        visibility = visibility,
        vnames_config = vnames_config,
    )
    entries = _invoke(
        index_compilation,
        name = name + "_entries",
        testonly = True,
        indexer = "//kythe/cxx/indexer/proto:indexer",
        opts = indexer_opts + ["--index_file"],
        tags = tags,
        visibility = visibility,
        deps = [kzip],
    )
    vopts = ["--ignore_dups"] + verifier_opts
    if convert_marked_source:
        vopts.append("--convert_marked_source")
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        srcs = [entries],
        opts = vopts,
        tags = tags,
        resolve_code_facts = resolve_code_facts,
        visibility = visibility,
        deps = [entries],
    )
