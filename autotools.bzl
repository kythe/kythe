# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Originally from google/sandboxed-api; added 'bootstrap'.

load("@bazel_tools//tools/build_defs/repo:utils.bzl", "patch")

"""Repository rule that runs autotools' configure after extraction."""

def _configure(ctx):
    bash_exe = ctx.os.environ["BAZEL_SH"] if "BAZEL_SH" in ctx.os.environ else "bash"
    ctx.report_progress("Running configure script...")

    # Run bootstrap script if it exists.
    ctx.execute(
        [bash_exe, "-c", "./bootstrap"],
        quiet = ctx.attr.quiet,
    )

    # Run configure script and move host-specific directory to a well-known
    # location.
    ctx.execute(
        [bash_exe, "-c", """./configure --disable-dependency-tracking {args};
                            mv $(. config.guess) configure-bazel-gen || true
                         """.format(args = " ".join(ctx.attr.configure_args))],
        quiet = ctx.attr.quiet,
    )

def _buildfile(ctx):
    bash_exe = ctx.os.environ["BAZEL_SH"] if "BAZEL_SH" in ctx.os.environ else "bash"
    if ctx.attr.build_file:
        ctx.execute([bash_exe, "-c", "rm -f BUILD BUILD.bazel"])
        ctx.symlink(ctx.attr.build_file, "BUILD.bazel")
    elif ctx.attr.build_file_content:
        ctx.execute([bash_exe, "-c", "rm -f BUILD.bazel"])
        ctx.file("BUILD.bazel", ctx.attr.build_file_content)
    patch(ctx)

def _autotools_repository_impl(ctx):
    if ctx.attr.build_file and ctx.attr.build_file_content:
        ctx.fail("Only one of build_file and build_file_content can be provided.")
    ctx.download_and_extract(
        ctx.attr.urls,
        "",  # output
        ctx.attr.sha256,
        "",  # type
        ctx.attr.strip_prefix,
    )
    _configure(ctx)
    _buildfile(ctx)

autotools_repository = repository_rule(
    attrs = {
        "urls": attr.string_list(
            mandatory = True,
            allow_empty = False,
        ),
        "sha256": attr.string(),
        "strip_prefix": attr.string(),
        "build_file": attr.label(
            mandatory = True,
            allow_single_file = [".BUILD"],
        ),
        "build_file_content": attr.string(),
        "configure_args": attr.string_list(
            allow_empty = True,
        ),
        "quiet": attr.bool(
            default = True,
        ),
        "patches": attr.label_list(
            default = [],
        ),
        "patch_args": attr.string_list(
            default = ["-p0"],
        ),
    },
    implementation = _autotools_repository_impl,
)
