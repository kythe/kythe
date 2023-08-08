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
        cc_deps = [],
        cc_indexer = "//kythe/cxx/indexer/cxx:indexer",
        verifier_opts = [
            "--ignore_dups",
            # Else the verifier chokes on the inconsistent marked source from the protobuf headers.
            "--convert_marked_source",
        ],
        size = "small",
        experimental_set_aliases_as_writes = False,
        minimal_claiming = True):
    """Verify cross-language references between C++ and Proto.

    Args:
      name: Name of the test.
      srcs: The compilation's C++ source files; each file's verifier goals will be checked
      proto_libs: A list of proto_library targets
      cc_deps: Additional cc deps needed by the cc test file
      cc_indexer: The cc indexer to use
      verifier_opts: List of options passed to the verifier tool
      size: Size of the test.
      experimental_set_aliases_as_writes: Set protobuf aliases as writes.
      minimal_claiming: If true, only index the `srcs` and protobuf header files.

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
            enable_proto_static_reflection = False,
        ),
    ]

    cc_kzip = _invoke(
        cc_extract_kzip,
        name = name + "_cc_kzip",
        srcs = srcs,
        deps = cc_proto_libs + cc_deps,
    )

    claim_file = None
    if minimal_claiming:
        claim_file = name + ".claim"
        native.genrule(
            name = name + "_static_claim",
            outs = [claim_file],
            srcs = [cc_kzip],
            tools = ["//kythe/cxx/tools:static_claim"],
            cmd = " ".join([
                "echo \"$<\" |",
                "$(location //kythe/cxx/tools:static_claim)",
                # Only claim protocol buffer headers and direct sources.
                # This won't work for things which aren't direct, immediate source files.
                "--include_files='.*/({}|[^/]+.(pb|proto).h)$$'".format("|".join(srcs)),
                "> \"$@\"",
            ]),
        )

    claim_opt = []
    claim_deps = []
    if minimal_claiming:
        claim_opt = [
            "--static_claim=$(execpath {})".format(claim_file),
            "--claim_unknown=false",
        ]
        claim_deps = [claim_file]
    
    aw_opt = []
    if experimental_set_aliases_as_writes:
        aw_opt = ["--experimental_set_aliases_as_writes"]

    cc_entries = _invoke(
        cc_index,
        name = name + "_cc_entries",
        srcs = [cc_kzip],
        deps = claim_deps,
        opts = [
            # Try to reduce the graph size to make life easier for the verifier.
            "--test_claim",
            "--noindex_template_instantiations",
            "--experimental_drop_instantiation_independent_data",
            "--noemit_anchors_on_builtins",
        ] + claim_opt + aw_opt,
        indexer = cc_indexer,
        test_indexer = cc_indexer,
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
