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

load("//tools/build_rules/verifier_test:verifier_test.bzl", "extract")
load("@rules_java//java:defs.bzl", "JavaInfo")
load("@rules_java//java:java_utils.bzl", _java_utils = "utils")

def _extract_java_aspect(target, ctx):
    if JavaInfo not in target or not hasattr(ctx.rule.attr, "srcs"):
        return struct()

    kzip = ctx.actions.declare_file(ctx.label.name + ".kzip")

    info = target[JavaInfo]
    compilation = info.compilation_info
    annotations = info.annotation_processing

    # compilation.javac_options may be a depset
    javac_options = _java_utils.tokenize_javacopts(ctx, compilation.javac_options)

    classpath = [j.path for j in compilation.compilation_classpath]
    bootclasspath = [j.path for j in compilation.boot_classpath]

    processorpath = []
    processors = []
    if annotations and annotations.enabled:
        processorpath += [j.path for j in annotations.processor_classpath]
        processors = annotations.processor_classnames

    # Skip --release options; -source/-target/-bootclasspath are already set
    args = _remove_flags(javac_options, {"--release": 1}) + [
        "-cp",
        ":".join(classpath),
        "-bootclasspath",
        ":".join(bootclasspath),
        "-processorpath",
        ":".join(processorpath),
    ]

    if processors:
        args += ["-processor", ",".join(processors)]
    else:
        args.append("-proc:none")

    deps = depset(
        transitive = annotations.processor_classpath + [
            a.inputs
            for a in target.actions
            if a.mnemonic == "Javac"
        ],
    )

    extract(
        ctx = ctx,
        kzip = kzip,
        extractor = ctx.executable._java_aspect_extractor,
        vnames_config = ctx.file._java_aspect_vnames_config,
        srcs = ctx.rule.files.srcs,
        opts = args,
        deps = deps.to_list(),
        mnemonic = "JavaExtractKZip",
    )

    return struct(kzip = kzip, output_groups = {"kzip": [kzip]})

def _remove_flags(lst, to_remove):
    res = []
    skip = 0
    for flag in lst:
        if skip > 0:
            skip -= 1
        elif flag in to_remove:
            skip += to_remove[flag]
        else:
            res.append(flag)
    return res

# Aspect to run the javac_extractor on all specified Java targets.
#
# Example usage:
#   bazel build -k --output_groups=kzip \
#       --aspects @io_kythe//tools/build_rules/verifier_test:verifier_test.bzl%extract_java_aspect \
#       //...
extract_java_aspect = aspect(
    _extract_java_aspect,
    attr_aspects = ["srcs"],
    attrs = {
        "_java_aspect_extractor": attr.label(
            default = Label("@io_kythe//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor"),
            executable = True,
            cfg = "exec",
        ),
        "_java_aspect_vnames_config": attr.label(
            default = Label("//external:vnames_config"),
            allow_single_file = True,
        ),
    },
)
