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

load(
    "@io_kythe//tools/build_rules/verifier_test:verifier_test.bzl",
    "KytheVerifierSources",
    "extract",
    "index_compilation",
    "verifier_test",
)
load("//kythe/cxx/indexer/proto/testdata:proto_verifier_test.bzl", "proto_extract_kzip")

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)

def _textproto_extract_kzip_impl(ctx):
    extract(
        srcs = ctx.files.srcs,
        ctx = ctx,
        extractor = ctx.executable.extractor,
        kzip = ctx.outputs.kzip,
        mnemonic = "TextprotoExtractKZip",
        opts = ["--", "--proto_path", ctx.label.package] + ctx.attr.opts,
        vnames_config = ctx.file.vnames_config,
        deps = ctx.files.deps,
    )
    return [KytheVerifierSources(files = depset(ctx.files.srcs))]

textproto_extract_kzip = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
        ),
        "deps": attr.label_list(allow_files = True),
        "extractor": attr.label(
            default = Label("//kythe/cxx/extractor/textproto:textproto_extractor"),
            executable = True,
            cfg = "host",
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
        textproto,
        protos,
        deps = [],
        size = "small",
        tags = [],
        extractor = None,
        extractor_opts = [],
        proto_extractor_opts = [],
        indexer_opts = [],
        verifier_opts = [],
        expect_success = True,
        convert_marked_source = False,
        vnames_config = None,
        visibility = None):
    """Extract, analyze, and verify a textproto compilation.

    Args:
      srcs: The compilation's source file inputs; each file's verifier goals will be checked
      deps: Optional list of textproto_verifier_test targets to be used as proto compilation dependencies
      extractor: Executable extractor tool to invoke (defaults to protoc_extractor)
      extractor_opts: List of options passed to the extractor tool
      proto_extractor_opts: List of options passed to the proto extractor tool
      indexer_opts: List of options passed to the indexer tool
      verifier_opts: List of options passed to the verifier tool
      vnames_config: Optional path to a VName configuration file
    """

    # extract textproto
    textproto_kzip = _invoke(
        textproto_extract_kzip,
        name = name + "_kzip",
        testonly = True,
        srcs = [textproto],
        extractor = extractor,
        opts = extractor_opts,
        tags = tags,
        visibility = visibility,
        vnames_config = vnames_config,
        deps = deps + protos,
    )

    # index textproto
    entries = _invoke(
        index_compilation,
        name = name + "_entries",
        testonly = True,
        indexer = "//kythe/cxx/indexer/textproto:textproto_indexer",
        opts = indexer_opts + ["--index_file"],
        tags = tags,
        visibility = visibility,
        deps = [textproto_kzip],
    )

    # extract proto(s)
    proto_kzip = _invoke(
        proto_extract_kzip,
        name = name + "_protos_kzip",
        testonly = True,
        srcs = protos,
        tags = tags,
        opts = proto_extractor_opts,
        visibility = visibility,
        vnames_config = vnames_config,
        deps = deps,
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

    vopts = verifier_opts + ["--ignore_dups", "--show_goals", "--goal_regex=\"\s*(?:#|//)-(.*)\""]
    if convert_marked_source:
        vopts += ["--convert_marked_source"]
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        srcs = [entries, proto_entries],
        expect_success = expect_success,
        opts = vopts,
        tags = tags,
        visibility = visibility,
        deps = [entries, proto_entries],
    )
