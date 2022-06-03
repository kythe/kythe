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

load(
    ":verifier_test.bzl",
    "KytheVerifierSources",
    "extract",
    "index_compilation",
    "verifier_test",
)

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)

def _jvm_extract_kzip_impl(ctx):
    jars = []
    for dep in ctx.attr.deps:
        jars.append(dep[JavaInfo].full_compile_jars)
    jars = depset(transitive = jars)

    extract(
        srcs = jars,
        ctx = ctx,
        extractor = ctx.executable.extractor,
        kzip = ctx.outputs.kzip,
        mnemonic = "JvmExtractKZip",
        opts = ctx.attr.opts,
        vnames_config = ctx.file.vnames_config,
    )
    return [KytheVerifierSources(files = depset())]

jvm_extract_kzip = rule(
    attrs = {
        "extractor": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/extractors/jvm:jar_extractor"),
            executable = True,
            cfg = "exec",
        ),
        "opts": attr.string_list(),
        "vnames_config": attr.label(
            default = Label("//external:vnames_config"),
            allow_single_file = True,
        ),
        "deps": attr.label_list(
            providers = [JavaInfo],
        ),
    },
    outputs = {"kzip": "%{name}.kzip"},
    implementation = _jvm_extract_kzip_impl,
)

def jvm_verifier_test(
        name,
        srcs,
        deps = [],
        size = "small",
        tags = [],
        indexer_opts = [],
        verifier_opts = ["--ignore_dups"],
        visibility = None):
    """Extract, analyze, and verify a JVM compilation.

    Args:
      srcs: Source files containing verifier goals for the JVM compilation
      deps: List of java/jvm verifier_test targets to be used as compilation dependencies
      indexer_opts: List of options passed to the indexer tool
      verifier_opts: List of options passed to the verifier tool
    """
    kzip = _invoke(
        jvm_extract_kzip,
        name = name + "_kzip",
        testonly = True,
        tags = tags,
        visibility = visibility,
        # This is a hack to depend on the .jar producer.
        deps = [d + "_kzip" for d in deps],
    )
    indexer = "//kythe/java/com/google/devtools/kythe/analyzers/jvm:class_file_indexer"
    entries = _invoke(
        index_compilation,
        name = name + "_entries",
        testonly = True,
        indexer = indexer,
        opts = indexer_opts,
        tags = tags,
        visibility = visibility,
        deps = [kzip],
    )
    return _invoke(
        verifier_test,
        name = name,
        size = size,
        srcs = [entries] + srcs,
        opts = verifier_opts,
        tags = tags,
        visibility = visibility,
        deps = [entries],
    )
