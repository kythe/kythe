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

"""
Implements a rule for testing the Rust extractor

Generates a test source file and sets the proper environment variables to run
the extractor
"""

load("@bazel_skylib//lib:paths.bzl", "paths")

def _rust_extractor_test_impl(ctx):
    test_binary = ctx.executable.src

    extractor = ctx.executable._extractor

    # Rust toolchain
    rust_toolchain = ctx.toolchains["@rules_rust//rust:toolchain"]
    rustc_lib = rust_toolchain.rustc_lib.files.to_list()
    rust_lib = rust_toolchain.rust_lib.files.to_list()

    source_file = ctx.actions.declare_file("main.rs")

    # sha256 digest = 7cb3b3c74ecdf86f434548ba15c1651c92bf03b6690fd0dfc053ab09d094cf03
    source_content = """
    fn main() {
        println!("Hello, world!");
    }
    """
    ctx.actions.write(
        output = source_file,
        content = source_content,
    )

    rustc_lib_path = paths.dirname(rustc_lib[0].short_path)
    rust_lib_path = paths.dirname(rust_lib[0].short_path)
    script = "\n".join(
        ["export DYLD_FALLBACK_LIBRARY_PATH=${DYLD_FALLBACK_LIBRARY_PATH:+$DYLD_FALLBACK_LIBRARY_PATH:}%s:%s" % (rustc_lib_path, rust_lib_path)] +
        ["export LD_LIBRARY_PATH=${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}%s:%s" % (rustc_lib_path, rust_lib_path)] +
        ["export SYSROOT=%s" % rust_lib_path] +
        ["export TEST_FILE=%s" % source_file.short_path] +
        ["export EXTRACTOR_PATH=%s" % extractor.short_path] +
        ["export KYTHE_CORPUS=test_corpus"] +
        ["./%s" % test_binary.short_path],
    )
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = script,
    )

    runfiles = ctx.runfiles(
        files = [test_binary, source_file, extractor, ctx.outputs.executable] + rustc_lib + rust_lib,
    )

    return [DefaultInfo(runfiles = runfiles)]

rust_extractor_test = rule(
    implementation = _rust_extractor_test_impl,
    attrs = {
        "src": attr.label(
            mandatory = True,
            executable = True,
            cfg = "target",
            doc = "The Rust binary to be executed",
        ),
        "_extractor": attr.label(
            default = Label("//kythe/rust/extractor:extractor"),
            executable = True,
            cfg = "target",
        ),
    },
    test = True,
    toolchains = ["@rules_rust//rust:toolchain"],
)
