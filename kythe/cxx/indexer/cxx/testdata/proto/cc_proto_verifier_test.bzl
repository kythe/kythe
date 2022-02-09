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
"""C++ protocol buffer metadata test rules."""

load("//tools/build_rules/verifier_test:cc_indexer_test.bzl", "cc_extract_kzip", "cc_index", "cc_kythe_proto_library")
load("//tools/build_rules/verifier_test:verifier_test.bzl", "index_compilation", "verifier_test")
load(
    "//kythe/cxx/indexer/proto/testdata:proto_verifier_test.bzl",
    "proto_extract_kzip",
)

def cc_proto_verifier_test(
        name,
        srcs,
        proto_libs,
        verifier_opts = [
            "--ignore_dups",
            # Else the verifier chokes on the inconsistent marked source from the protobuf headers.
            "--convert_marked_source",
        ],
        size = "small",
        experimental_guess_proto_semantics = False,
        experimental_record_dataflow_edges = False):
    """Verify cross-language references between C++ and Proto.

    Args:
      name: Name of the test.
      srcs: The compilation's C++ source files; each file's verifier goals will be checked
      proto_libs: A list of proto_library targets
      verifier_opts: List of options passed to the verifier tool
      size: Size of the test.
      experimental_guess_proto_semantics: guess proto semantics?
      experimental_record_dataflow_edges: record dataflow edges?

    Returns:
      The label of the test.
    """
    proto_kzip = _invoke(
        proto_extract_kzip,
        name = name + "_proto_kzip",
        srcs = proto_libs,
    )

    proto_entries = _invoke(
        index_compilation,
        name = name + "_proto_entries",
        indexer = "//kythe/cxx/indexer/proto:indexer",
        opts = ["--index_file"],
        deps = [proto_kzip],
    )

    cc_proto_libs = [
        _invoke(
            cc_kythe_proto_library,
            name = name + "_cc_proto",
            deps = proto_libs,
        ),
    ]

    cc_kzip = _invoke(
        cc_extract_kzip,
        name = name + "_cc_kzip",
        srcs = srcs,
        deps = cc_proto_libs,
    )

    guess_opt = []
    if experimental_guess_proto_semantics:
        guess_opt = ["--experimental_guess_proto_semantics"]
    df_opt = []
    if experimental_record_dataflow_edges:
        df_opt = ["--experimental_record_dataflow_edges"]

    cc_entries = _invoke(
        cc_index,
        name = name + "_cc_entries",
        srcs = [cc_kzip],
        opts = [
            # Avoid emitting some nodes with conflicting facts.
            "--experimental_index_lite",
            # Try to reduce the graph size to make life easier for the verifier.
            "--test_claim",
            "--noindex_template_instantiations",
            "--experimental_drop_instantiation_independent_data",
            "--noemit_anchors_on_builtins",
        ] + guess_opt + df_opt,
    )

    return _invoke(
        verifier_test,
        name = name,
        srcs = srcs + [proto_entries],
        opts = verifier_opts,
        size = size,
        deps = [
            cc_entries,
            proto_entries,
        ],
    )

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)
