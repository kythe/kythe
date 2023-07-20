"""Rules for verifying textproto indexer output"""

# copied from proto_verifier_test.bzl.
# TODO(justbuchanan): refactor

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

load("@bazel_skylib//lib:paths.bzl", "paths")
load(
    "@io_kythe//tools/build_rules/verifier_test:verifier_test.bzl",
    "KytheVerifierSources",
    "extract",
    "index_compilation",
    "verifier_test",
)
load("//kythe/cxx/indexer/proto/testdata:proto_verifier_test.bzl", "get_proto_files_and_proto_paths", "proto_extract_kzip")

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)

def _textproto_extract_kzip_impl(ctx):
    toplevel_proto_srcs, all_proto_srcs, pathopt = get_proto_files_and_proto_paths(ctx.attr.protos)

    args = ctx.actions.args()
    args.add_all(ctx.attr.opts)
    args.add("--")
    args.add_all(pathopt, before_each = "--proto_path")

    extract(
        srcs = ctx.files.srcs,
        ctx = ctx,
        extractor = ctx.executable.extractor,
        kzip = ctx.outputs.kzip,
        mnemonic = "TextprotoExtractKZip",
        opts = args,
        vnames_config = ctx.file.vnames_config,
        deps = all_proto_srcs,
    )
    return [KytheVerifierSources(files = depset(ctx.files.srcs))]

textproto_extract_kzip = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
            providers = [ProtoInfo],
        ),
        "protos": attr.label_list(mandatory = True, allow_empty = False, allow_files = False),
        "extractor": attr.label(
            default = Label("//kythe/cxx/extractor/textproto:textproto_extractor"),
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
    implementation = _textproto_extract_kzip_impl,
)

def textproto_verifier_test(
        name,
        textprotos,
        protos,
        size = "small",
        tags = [],
        extractor_opts = [],
        indexer_opts = [],
        verifier_opts = [],
        convert_marked_source = False,
        vnames_config = None,
        visibility = None):
    """Extract, analyze, and verify a textproto compilation.

    Args:
      name: Name of the test
      textprotos: Textproto files being tested
      protos: Proto libraries that define the textproto's schema
      size: Test size
      tags: Test tags
      extractor_opts: List of options passed to the textproto extractor
      indexer_opts: List of options passed to the textproto indexer
      verifier_opts: List of options passed to the verifier tool
      convert_marked_source: Whether the verifier should convert marked source.
      vnames_config: Optional path to a VName configuration file
      visibility: Visibility of underlying build targets
    Returns:
      Name of the test rule
    """

    # extract and index each textproto
    textproto_entries = []
    for textproto in textprotos:
        rule_prefix = name + "_" + paths.replace_extension(textproto, "")

        # extract textproto
        textproto_kzip = _invoke(
            textproto_extract_kzip,
            name = rule_prefix + "_kzip",
            testonly = True,
            srcs = [textproto],
            tags = tags,
            visibility = visibility,
            vnames_config = vnames_config,
            protos = protos,
            opts = extractor_opts,
        )

        # index textproto
        entries = _invoke(
            index_compilation,
            name = rule_prefix + "_entries",
            testonly = True,
            indexer = "//kythe/cxx/indexer/textproto:textproto_indexer",
            opts = indexer_opts + ["--index_file"],
            tags = tags,
            visibility = visibility,
            deps = [textproto_kzip],
        )

        textproto_entries.append(entries)

    # extract proto(s)
    proto_kzip = _invoke(
        proto_extract_kzip,
        name = name + "_protos_kzip",
        testonly = True,
        srcs = protos,
        tags = tags,
        visibility = visibility,
        vnames_config = vnames_config,
    )

    # index proto(s)
    proto_entries = _invoke(
        index_compilation,
        name = name + "_proto_entries",
        testonly = True,
        indexer = "//kythe/cxx/indexer/proto:indexer",
        opts = ["--index_file"],
        tags = tags,
        visibility = visibility,
        deps = [proto_kzip],
    )

    vopts = verifier_opts + ["--ignore_dups", "--show_goals", "--goal_regex=\"\\s*(?:#|//)-(.*)\""]
    if convert_marked_source:
        vopts.append("--convert_marked_source")
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        opts = vopts,
        tags = tags,
        visibility = visibility,
        deps = textproto_entries + [proto_entries],
    )
