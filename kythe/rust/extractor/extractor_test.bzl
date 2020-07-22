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

def _rust_extractor_test_impl(ctx):
    test_binary = ctx.executable.src

    lib = ctx.files.lib
    sysroot = ctx.files.sysroot

    source_file = ctx.actions.declare_file("main.rs")
    source_content = """
    fn main() {
        println!("Hello, world!");
    }
    """
    ctx.actions.write(
        output = source_file,
        content = source_content
    )

    script = "\n".join(
        ["export LD_LIBRARY_PATH=%s" % paths.dirname(lib[0].path)] +
        ["export SYSROOT=%s" % paths.dirname(sysroot[0].path)] +
        ["export TEST_FILE=%s" % source_file.short_path] +
        ["./%s" % test_binary.short_path]
    )
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = script,
    )

    runfiles = ctx.runfiles(
        files = [test_binary, source_file, ctx.outputs.executable] + lib + sysroot
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
        "lib": attr.label(
            mandatory = True,
            allow_files = True,
            doc = "A label for the filegroup containing the rustc library files"
        ),
        "sysroot": attr.label(
            mandatory = True,
            allow_files = True,
            doc = "A label for the filegroup containing the rustc sysroot files"
        )
    },
    test = True,
)
