# Copyright 2020 The Kythe Authors. All rights reserved.
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
load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain")
load("//kythe/go/indexer:testdata/go_indexer_test.bzl", "go_verifier_test")
load(
    "//tools/build_rules/verifier_test:verifier_test.bzl",
    "KytheEntries",
)

def _rust_extract_impl(ctx):
    # Get the path for the system's linker
    cc_toolchain = find_cpp_toolchain(ctx)
    cc_features = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
    )
    linker_path = cc_common.get_tool_for_action(
        feature_configuration = cc_features,
        action_name = "c++-link-executable",
    )

    # Rust toolchain
    rust_toolchain = ctx.toolchains["@rules_rust//rust:toolchain"]
    rustc_lib = rust_toolchain.rustc_lib.files.to_list()
    rust_lib = rust_toolchain.rust_lib.files.to_list()

    # Generate extra_action file to be used by the extractor
    extra_action_file = ctx.actions.declare_file(ctx.label.name + ".xa")
    xa_maker = ctx.executable._extra_action
    ctx.actions.run(
        executable = xa_maker,
        arguments = [
            "--src_files=%s" % ",".join([f.path for f in ctx.files.srcs]),
            "--output=%s" % extra_action_file.path,
            "--owner=%s" % ctx.label.name,
            "--crate_name=%s" % ctx.attr.crate_name,
            "--sysroot=%s" % paths.dirname(rust_lib[0].path),
            "--linker=%s" % linker_path,
        ],
        outputs = [extra_action_file],
    )

    # Generate the kzip
    output = ctx.outputs.kzip
    ctx.actions.run(
        mnemonic = "RustExtract",
        executable = ctx.executable._extractor,
        arguments = [
            "--extra_action=%s" % extra_action_file.path,
            "--output=%s" % output.path,
        ],
        inputs = [extra_action_file] + rustc_lib + rust_lib + ctx.files.srcs,
        outputs = [output],
        env = {
            "KYTHE_CORPUS": "test_corpus",
            "LD_LIBRARY_PATH": paths.dirname(rustc_lib[0].path),
        },
    )

    return struct(kzip = output)

# Generate a kzip with the compilations captured from a single Go library or
# binary rule.
rust_extract = rule(
    _rust_extract_impl,
    attrs = {
        # Additional data files to include in each compilation.
        "data": attr.label_list(
            allow_empty = True,
            allow_files = True,
        ),
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = [".rs"],
        ),
        "crate_name": attr.string(
            default = "test_crate",
        ),
        "_extractor": attr.label(
            default = Label("//kythe/rust/extractor"),
            executable = True,
            cfg = "host",
        ),
        "_extra_action": attr.label(
            default = Label("//tools/rust/extra_action"),
            executable = True,
            cfg = "host",
        ),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
            allow_files = True,
        ),
    },
    outputs = {"kzip": "%{name}.kzip"},
    fragments = ["cpp"],
    toolchains = [
        "@rules_rust//rust:toolchain",
        "@bazel_tools//tools/cpp:toolchain_type",
    ],
    incompatible_use_toolchain_transition = True,
)

def _rust_entries_impl(ctx):
    kzip = ctx.attr.kzip.kzip
    indexer = ctx.executable._indexer
    iargs = [indexer.path]
    output = ctx.outputs.entries

    # TODO(Arm1stice): Pass arguments to indexer based on rule attributes
    # # If the test wants marked source, enable support for it in the indexer.
    # if ctx.attr.has_marked_source:
    #     iargs.append("-code")

    # if ctx.attr.emit_anchor_scopes:
    #     iargs.append("-anchor_scopes")

    # # If the test wants linkage metadata, enable support for it in the indexer.
    # if ctx.attr.metadata_suffix:
    #     iargs += ["-meta", ctx.attr.metadata_suffix]

    iargs += [kzip.path, "| gzip >" + output.path]

    cmds = ["set -e", "set -o pipefail", " ".join(iargs), ""]
    ctx.actions.run_shell(
        mnemonic = "RustIndexer",
        command = "\n".join(cmds),
        outputs = [output],
        inputs = [kzip],
        tools = [indexer],
    )
    return [KytheEntries(compressed = depset([output]), files = depset())]

# Run the Kythe indexer on the output that results from a go_extract rule.
rust_entries = rule(
    _rust_entries_impl,
    attrs = {
        # Whether to enable explosion of MarkedSource facts.
        "has_marked_source": attr.bool(default = False),

        # Whether to enable anchor scope edges.
        "emit_anchor_scopes": attr.bool(default = False),

        # The kzip to pass to the Rust indexer
        "kzip": attr.label(
            providers = ["kzip"],
            mandatory = True,
        ),

        # The location of the Rust indexer binary.
        "_indexer": attr.label(
            default = Label("//kythe/rust/indexer:bazel_indexer"),
            executable = True,
            cfg = "host",
        ),
    },
    outputs = {"entries": "%{name}.entries.gz"},
)

def _rust_indexer(
        name,
        srcs,
        data = None,
        has_marked_source = False,
        emit_anchor_scopes = False,
        allow_duplicates = False,
        metadata_suffix = ""):
    kzip = name + "_units"
    rust_extract(
        name = kzip,
        srcs = srcs,
    )
    entries = name + "_entries"
    rust_entries(
        name = entries,
        has_marked_source = has_marked_source,
        emit_anchor_scopes = emit_anchor_scopes,
        kzip = ":" + kzip,
    )
    return entries

def rust_indexer_test(
        name,
        srcs,
        size = None,
        tags = None,
        log_entries = False,
        has_marked_source = False,
        emit_anchor_scopes = False,
        allow_duplicates = False):
    # Generate entries using the Rust indexer
    entries = _rust_indexer(
        name = name,
        srcs = srcs,
        has_marked_source = has_marked_source,
        emit_anchor_scopes = emit_anchor_scopes,
    )

    # Most of this code was copied from the Go verifier macros and modified for
    # Rust. This function does not need to be modified, so we are just calling
    # it directly here.
    go_verifier_test(
        name = name,
        size = size,
        allow_duplicates = allow_duplicates,
        entries = ":" + entries,
        has_marked_source = has_marked_source,
        log_entries = log_entries,
        tags = tags,
    )
