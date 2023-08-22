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

    owner = str(target.label)
    kzip = ctx.actions.declare_file(ctx.label.name + ".xa.java.kzip")

    info = target[JavaInfo]
    compilation = info.compilation_info
    annotations = info.annotation_processing

    source_files = []
    for src in ctx.rule.files.srcs:
        source_files.append(src.path)

    classpath = [j.path for j in compilation.compilation_classpath.to_list()]
    bootclasspath = [j.path for j in compilation.boot_classpath]

    processorpath = []
    processors = []
    if annotations and annotations.enabled:
        processorpath += [j.path for j in annotations.processor_classpath.to_list()]
        processors = annotations.processor_classnames

    output_jar = [jar.class_jar.path for jar in info.outputs.jars]
    if len(output_jar) > 1:
        print("WARNING: multiple outputs for " + owner + ": " + str(output_jar))
        output_jar = [output_jar[0]]

    xa = struct(**{
        "owner": owner,
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
        toolchain = None,
    )

    extract_args = ctx.actions.args()
    extract_args.add_all([xa, kzip, ctx.file._java_aspect_vnames_config])
    deps = [javac_action.inputs, annotations.processor_classpath]
    ctx.actions.run(
        outputs = [kzip],
        inputs = depset([xa, ctx.file._java_aspect_vnames_config], transitive = deps),
        executable = ctx.executable._java_bazel_extractor,
        arguments = [extract_args],
        tools = ctx.attr._java_runtime[java_common.JavaRuntimeInfo].files,
        toolchain = None,
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
    attrs = {
        "_write_extra_action": attr.label(
            default = Label("@io_kythe//kythe/go/util/tools/write_extra_action"),
            executable = True,
            cfg = "exec",
        ),
        "_java_bazel_extractor": attr.label(
            default = Label("@io_kythe//kythe/java/com/google/devtools/kythe/extractors/java/bazel:java_extractor"),
            executable = True,
            cfg = "exec",
        ),
        "_java_aspect_vnames_config": attr.label(
            default = Label("//external:vnames_config"),
            allow_single_file = True,
        ),
        "_java_runtime": attr.label(
            default = Label("@rules_java//toolchains:current_java_runtime"),
            cfg = "exec",
            allow_files = True,
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
