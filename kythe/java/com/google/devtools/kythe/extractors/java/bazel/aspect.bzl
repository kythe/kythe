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

load("//tools/build_rules/verifier_test:verifier_test.bzl", "extract")

_mnemonic = "Javac"

def _extract_java(target, ctx):
    if JavaInfo not in target or not hasattr(ctx.rule.attr, "srcs"):
        return None

    javac_action = None
    for a in target.actions:
        if a.mnemonic == _mnemonic:
            javac_action = a
            break

    if not javac_action:
        return None

    kzip = ctx.actions.declare_file(ctx.label.name + ".xa.java.kzip")

    info = target[JavaInfo]
    compilation = info.compilation_info
    annotations = info.annotation_processing

    source_files = []
    for src in ctx.rule.files.srcs:
        source_files += [src.path]

    classpath = [j.path for j in compilation.compilation_classpath.to_list()]
    bootclasspath = [j.path for j in compilation.boot_classpath]

    processorpath = []
    processors = []
    if annotations and annotations.enabled:
        processorpath += [j.path for j in annotations.processor_classpath.to_list()]
        processors = annotations.processor_classnames

    output_jar = []
    for jar in info.outputs.jars:
        output_jar += [jar.class_jar.path]

    # TODO(schroederc): sourcegendir is currently extracted from raw arguments;
    # we need to embed it there or put it elsewhere
    xa = struct(**{
        "owner": str(target.label),
        "mnemonic": _mnemonic,
        "[blaze.JavaCompileInfo.java_compile_info]": struct(
            outputjar = output_jar,
            classpath = classpath,
            source_file = source_files,
            javac_opt = compilation.javac_options,
            processor = processors,
            processorpath = processorpath,
            bootclasspath = bootclasspath,
        ),
    })
    text_xa = ctx.actions.declare_file(ctx.label.name + ".xa.textproto")
    ctx.actions.write(
        output = text_xa,
        content = xa.to_proto(),
    )

    xa = ctx.actions.declare_file(ctx.label.name + ".xa")
    xa_args = ctx.actions.args()
    xa_args.add_all([text_xa, xa])
    ctx.actions.run(
        outputs = [xa],
        inputs = [text_xa],
        arguments = [xa_args],
        executable = ctx.executable._write_extra_action,
    )

    extract_args = ctx.actions.args()
    extract_args.add_all([xa, kzip, ctx.file._java_aspect_vnames_config])
    deps = [javac_action.inputs]
    ctx.actions.run(
        outputs = [kzip],
        inputs = depset([xa, ctx.file._java_aspect_vnames_config], transitive = deps),
        executable = ctx.executable._java_bazel_extractor,
        arguments = [extract_args],
    )

    return kzip

def _extract_java_aspect(target, ctx):
    kzip = _extract_java(target, ctx)
    if not kzip:
        return struct()
    return [OutputGroupInfo(kzip = [kzip])]

# Aspect to run the Bazel Javac extractor on all specified Java targets.
extract_java_aspect = aspect(
    _extract_java_aspect,
    attr_aspects = ["srcs"],
    attrs = {
        "_write_extra_action": attr.label(
            default = Label("@io_kythe//kythe/go/util/tools/write_extra_action"),
            executable = True,
            cfg = "host",
        ),
        "_java_bazel_extractor": attr.label(
            default = Label("@io_kythe//kythe/java/com/google/devtools/kythe/extractors/java/bazel:java_extractor"),
            executable = True,
            cfg = "host",
        ),
        "_java_aspect_vnames_config": attr.label(
            default = Label("//external:vnames_config"),
            allow_single_file = True,
        ),
    },
)

def _extract_java_impl(ctx):
    output = ctx.attr.compilation[OutputGroupInfo]
    return [
        OutputGroupInfo(kzip = output.kzip),
        DefaultInfo(files = output.kzip),
    ]

# Runs the Bazel Java extractor on the given Java compilation target
extract_java = rule(
    implementation = _extract_java_impl,
    attrs = {"compilation": attr.label(aspects = [extract_java_aspect])},
)
