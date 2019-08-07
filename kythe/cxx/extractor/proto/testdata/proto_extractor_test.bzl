"""Rules for testing the proto extractor"""

# Copyright 2018 The Kythe Authors. All rights reserved.
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

load("@bazel_skylib//lib:dicts.bzl", "dicts")

def _extract_kzip_impl(ctx):
    cmd = [ctx.executable.extractor.path] + [p.path for p in ctx.files.srcs] + ctx.attr.opts
    ctx.actions.run_shell(
        mnemonic = "Extract",
        command = " ".join(cmd),
        env = dicts.add(ctx.attr.extra_env, {"KYTHE_OUTPUT_FILE": ctx.outputs.kzip.path}),
        outputs = [ctx.outputs.kzip],
        tools = [ctx.executable.extractor],
        inputs = ctx.files.srcs + ctx.files.deps,
    )
    return [DefaultInfo(runfiles = ctx.runfiles(files = [ctx.outputs.kzip]))]

extract_kzip = rule(
    implementation = _extract_kzip_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = True, mandatory = True),
        "deps": attr.label_list(allow_files = True),
        "extractor": attr.label(
            cfg = "host",
            executable = True,
            default = Label("//kythe/cxx/extractor/proto:proto_extractor"),
        ),
        "opts": attr.string_list(),
        "extra_env": attr.string_dict(),
    },
    outputs = {"kzip": "%{name}.kzip"},
)

def _kzip_diff_test_impl(ctx):
    # Write a script that `bazel test` will execute.
    script = " ".join([
        ctx.executable.diff_bin.short_path,
        ctx.executable.kzip_tool.short_path,
        ctx.executable.jq.short_path,
        ctx.files.kzip[0].short_path,
        ctx.files.golden_file[0].short_path,
    ])
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = script,
    )

    runfiles = ctx.runfiles(files = [
        ctx.executable.diff_bin,
        ctx.executable.kzip_tool,
        ctx.executable.jq,
        ctx.file.kzip,
        ctx.file.golden_file,
    ])
    return [DefaultInfo(runfiles = runfiles)]

kzip_diff_test = rule(
    implementation = _kzip_diff_test_impl,
    attrs = {
        "golden_file": attr.label(mandatory = True, allow_single_file = True),
        "kzip": attr.label(mandatory = True, allow_single_file = True),
        "diff_bin": attr.label(
            cfg = "host",
            executable = True,
            default = Label("//kythe/cxx/extractor/proto/testdata:kzip_diff_test"),
        ),
        "kzip_tool": attr.label(
            cfg = "host",
            executable = True,
            default = Label("//kythe/go/platform/tools/kzip"),
        ),
        "jq": attr.label(
            cfg = "host",
            executable = True,
            default = Label("@com_github_stedolan_jq//:jq"),
        ),
    },
    test = True,
)

def extractor_golden_test(
        name,
        srcs,
        deps = [],
        opts = [],
        extra_env = {},
        extractor = "//kythe/cxx/extractor/proto:proto_extractor"):
    """Runs the extractor and compares the result to a golden file.

    Args:
      name: test name (note: _test will be appended to the end)
      srcs: files to extract
      deps: any other required deps
      opts: arguments to pass to the extractor
      extra_env: environment variables to configure extractor behavior
      extractor: the extractor binary to use
    """
    kzip = name + "_kzip"
    extract_kzip(
        name = kzip,
        opts = opts,
        deps = deps,
        srcs = srcs,
        extra_env = extra_env,
        extractor = extractor,
        testonly = True,
    )

    kzip_diff_test(
        name = name + "_test",
        kzip = kzip,
        golden_file = name + ".UNIT",
    )
