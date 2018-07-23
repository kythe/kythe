#
# Copyright 2016 The Kythe Authors. All rights reserved.
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
#

KytheVerifierSources = provider(
    doc = "Input files which the verifier should inspect for assertions.",
    fields = {
        "files": "Depset of files which should be considered.",
    },
)

KytheEntries = provider(
    doc = "Kythe indexer entry facts.",
    fields = {
        "files": "Depset of files which combine to make an index.",
        "compressed": "Depset of combined, compressed index entries.",
    },
)

KytheJavaJar = provider(
    doc = "Bundled Java compilation.",
    fields = {
        "jar": "The bundled jar file.",
    },
)

def _atomize_entries_impl(ctx):
  zcat = ctx.executable._zcat
  entrystream = ctx.executable._entrystream
  postprocessor = ctx.executable._postprocessor
  atomizer = ctx.executable._atomizer

  inputs = depset(ctx.files.srcs)
  for dep in ctx.attr.deps:
    inputs += dep.kythe_entries

  sorted_entries = ctx.new_file(ctx.outputs.entries, ctx.label.name + "_sorted_entries")
  ctx.action(
      outputs = [sorted_entries],
      inputs = [zcat, entrystream] + list(inputs),
      mnemonic = "SortEntries",
      command = '("$1" "${@:4}" | "$2" --sort) > "$3" || rm -f "$3"',
      arguments = (
          [zcat.path, entrystream.path, sorted_entries.path] +
          [s.path for s in inputs]
      ),
  )
  leveldb = ctx.new_file(ctx.outputs.entries, ctx.label.name + "_serving_tables")
  ctx.action(
      outputs = [leveldb],
      inputs = [sorted_entries, postprocessor],
      executable = postprocessor,
      mnemonic = "PostProcessEntries",
      arguments = ["--entries", sorted_entries.path, "--out", leveldb.path],
  )
  ctx.action(
      outputs = [ctx.outputs.entries],
      inputs = [atomizer, leveldb],
      mnemonic = "AtomizeEntries",
      command = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") | gzip -c > "${@:${#@}}"',
      arguments = ([atomizer.path, "--api", leveldb.path] +
                   ctx.attr.file_tickets +
                   [ctx.outputs.entries.path]),
      execution_requirements = {
          # TODO(shahms): Remove this when we can use a non-LevelDB store.
          "local": "true", # LevelDB is bad and should feel bad.
      },
  )
  return struct()

atomize_entries = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = [
                ".entries",
                ".entries.gz",
            ],
        ),
        "deps": attr.label_list(
            providers = ["kythe_entries"],
        ),
        "file_tickets": attr.string_list(
            mandatory = True,
            allow_empty = False,
        ),
        "_zcat": attr.label(
            default = Label("//tools:zcatext"),
            executable = True,
            cfg = "host",
        ),
        "_entrystream": attr.label(
            default = Label("//kythe/go/platform/tools/entrystream"),
            executable = True,
            cfg = "host",
        ),
        "_postprocessor": attr.label(
            default = Label("//kythe/go/serving/tools/write_tables"),
            executable = True,
            cfg = "host",
        ),
        "_atomizer": attr.label(
            default = Label("//kythe/go/test/tools:xrefs_atomizer"),
            executable = True,
            cfg = "host",
        ),
    },
    outputs = {
        "entries": "%{name}.entries.gz",
    },
    implementation = _atomize_entries_impl,
)

def extract(ctx, kindex, extractor, srcs, opts, deps=[], vnames_config=None, mnemonic="ExtractKindex"):
  env = {
      "KYTHE_ROOT_DIRECTORY": ".",
      "KYTHE_OUTPUT_FILE": kindex.path,
  }
  inputs = [extractor] + srcs + deps
  if vnames_config:
    env["KYTHE_VNAMES"] = vnames_config.path
    inputs += [vnames_config]
  ctx.actions.run(
      inputs = inputs,
      outputs = [kindex],
      mnemonic = mnemonic,
      executable = extractor,
      arguments = (
          [ctx.expand_location(o) for o in opts] +
          [src.path for src in srcs]
      ),
      env = env
  )
  return kindex

def _java_extract_kindex_impl(ctx):
  jars = []
  for dep in ctx.attr.deps:
    jars += [dep[KytheJavaJar].jar]

  # Actually compile the sources to be used as a dependency for other tests
  jar = ctx.new_file(ctx.outputs.kindex, ctx.outputs.kindex.basename + ".jar")
  info = java_common.compile(ctx,
    javac_opts = java_common.default_javac_opts(
        ctx, java_toolchain_attr = "_java_toolchain") + ctx.attr.opts,
    java_toolchain = ctx.attr._java_toolchain,
    host_javabase = ctx.attr._host_javabase,
    source_files = ctx.files.srcs,
    output = jar,
    deps = [JavaInfo(output_jar = curr_jar, compile_jar = curr_jar) for curr_jar in jars],
  )

  args = ctx.attr.opts + [
      "-encoding", "utf-8",
      "-cp", ":".join([j.path for j in jars]),
  ]
  for src in ctx.files.srcs:
    args += [src.short_path]
  extract(
      ctx = ctx,
      kindex = ctx.outputs.kindex,
      extractor = ctx.executable.extractor,
      vnames_config = ctx.file.vnames_config,
      srcs = ctx.files.srcs,
      opts = args,
      deps = jars + ctx.files.data,
      mnemonic = "JavaExtractKindex",
  )
  return [
      KytheJavaJar(jar=jar),
      KytheVerifierSources(files=ctx.files.srcs),
  ]

java_extract_kindex = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
        ),
        "deps": attr.label_list(
            providers = [KytheJavaJar],
        ),
        "data": attr.label_list(
            allow_files = True,
        ),
        "vnames_config": attr.label(
            default = Label("//kythe/data:vnames_config"),
            allow_single_file = True,
        ),
        "extractor": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor"),
            executable = True,
            cfg = "host",
        ),
        "opts": attr.string_list(),
        "_java_toolchain": attr.label(
            default = Label("@bazel_tools//tools/jdk:toolchain"),
        ),
        "_host_javabase": attr.label(
            cfg = "host",
            default = Label("@bazel_tools//tools/jdk:current_java_runtime"),
        ),
    },
    fragments = ["java"],
    host_fragments = ["java"],
    outputs = {
        "kindex": "%{name}.kindex",
    },
    implementation = _java_extract_kindex_impl,
)

def _jvm_extract_kindex_impl(ctx):
  jars = []
  for dep in ctx.attr.deps:
    jars += [dep[KytheJavaJar].jar]

  extract(
      ctx = ctx,
      srcs = jars,
      kindex = ctx.outputs.kindex,
      extractor = ctx.executable.extractor,
      vnames_config = ctx.file.vnames_config,
      opts = ctx.attr.opts,
      mnemonic = "JvmExtractKindex",
  )
  return [KytheVerifierSources(files=[])]

jvm_extract_kindex = rule(
    attrs = {
        "deps": attr.label_list(
            providers = [KytheJavaJar],
        ),
        "extractor": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/extractors/jvm:jar_extractor"),
            executable = True,
            cfg = "host",
        ),
        "vnames_config": attr.label(
            default = Label("//kythe/data:vnames_config"),
            allow_single_file = True,
        ),
        "opts": attr.string_list(),
    },
    outputs = {"kindex": "%{name}.kindex"},
    implementation = _jvm_extract_kindex_impl,
)

def _index_compilation_impl(ctx):
  sources = depset()
  intermediates = []
  for dep in ctx.attr.deps:
    if KytheVerifierSources in dep:
      sources += dep[KytheVerifierSources].files
    for input in dep.files:
      entries = ctx.new_file(ctx.outputs.entries,
                             ctx.label.name + input.basename + ".entries")
      intermediates += [entries]
      ctx.action(
          outputs = [entries],
          inputs = [ctx.executable.indexer, input] + ctx.files.tools,
          arguments = ([ctx.executable.indexer.path] +
                       [ctx.expand_location(o) for o in ctx.attr.opts] +
                       [input.path, entries.path]),
          command = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") > "${@:${#@}}"',
          mnemonic = "IndexCompilation",
      )
  ctx.action(
      outputs = [ctx.outputs.entries],
      inputs = intermediates,
      command = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") | gzip -c > "${@:${#@}}"',
      mnemonic = "CompressEntries",
      arguments = ["cat"] + [i.path for i in intermediates] + [ctx.outputs.entries.path],
  )
  return [
      KytheVerifierSources(files=sources),
      KytheEntries(files=intermediates, compressed=depset([ctx.outputs.entries])),
  ]

index_compilation = rule(
    attrs = {
        "deps": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = [".kindex"],
        ),
        "tools": attr.label_list(
            cfg = "host",
            allow_files = True,
        ),
        "indexer": attr.label(
            mandatory = True,
            executable = True,
            cfg = "host",
        ),
        "opts": attr.string_list(),
    },
    outputs = {
        "entries": "%{name}.entries.gz",
    },
    implementation = _index_compilation_impl,
)

def _verifier_test_impl(ctx):
  entries = depset()
  entries_gz = depset()
  sources = depset()
  for src in ctx.attr.srcs:
    if KytheVerifierSources in src:
      sources += src[KytheVerifierSources].files
      if KytheEntries in src:
        if src[KytheEntries].files:
          entries += src[KytheEntries].files
        else:
          entries_gz += src[KytheEntries].compressed
    else:
      sources += src.files


  for dep in ctx.attr.deps:
    # TODO(shahms): Allow specifying .entries files directly.
    if dep[KytheEntries].files:
      entries += dep[KytheEntries].files
    else:
      entries_gz += dep[KytheEntries].compressed

  if not (entries or entries_gz):
    fail("Missing required entry stream input (check your deps!)")
  args = ctx.attr.opts + [src.short_path for src in sources]
  # If no dependency specifies KytheVerifierSources and
  # we aren't provided explicit sources, assume `--use_file_nodes`.
  if not sources and "--use_file_nodes" not in args:
    args += ["--use_file_nodes"]
  ctx.actions.expand_template(
      template = ctx.file._template,
      output = ctx.outputs.executable,
      is_executable = True,
      substitutions = {
          "@WORKSPACE_NAME@": ctx.workspace_name,
          "@ENTRIES@": " ".join([e.short_path for e in entries]),
          "@ENTRIES_GZ@": " ".join([e.short_path for e in entries_gz]),
          # If failure is expected, invert the sense of the verifier return.
          "@INVERT@": "!" if not ctx.attr.expect_success else "",
          "@VERIFIER@": ctx.executable._verifier.short_path,
          "@ARGS@": " ".join(args),
      },
  )
  runfiles = ctx.runfiles(files = list(sources + entries + entries_gz) + [
      ctx.outputs.executable,
      ctx.executable._verifier,
  ], collect_data = True)
  return [
      DefaultInfo(runfiles=runfiles),
  ]

verifier_test = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            providers = [
                [KytheVerifierSources],
                [
                    KytheVerifierSources,
                    KytheEntries,
                ],
            ],
        ),
        "deps": attr.label_list(
            # TODO(shahms): Allow directly specifying sources/deps.
            #allow_files = [
            #    ".entries",
            #    ".entries.gz",
            #],
            providers = [KytheEntries],
        ),
        # Arguably, "expect_failure" is more natural, but that
        # attribute is used by Skylark.
        "expect_success": attr.bool(default = True),
        "_verifier": attr.label(
            default = Label("//kythe/cxx/verifier"),
            executable = True,
            cfg = "target",
        ),
        "_template": attr.label(
            default = Label("//tools/build_rules/verifier_test:verifier_test.sh.in"),
            allow_single_file = True,
        ),
        "opts": attr.string_list(),
    },
    test = True,
    implementation = _verifier_test_impl,
)

def _invoke(rulefn, name, **kwargs):
  """Invoke rulefn with name and kwargs, returning the label of the rule."""
  rulefn(name=name, **kwargs)
  return "//{}:{}".format(native.package_name(), name)

def java_verifier_test(name, srcs, meta=[], deps=[], size="small", tags=[],
                       indexer_opts=["--verbose"], verifier_opts=["--ignore_dups"],
                       load_plugin=None, extra_goals=[],
                       vnames_config=None, visibility=None):
  kindex = _invoke(java_extract_kindex,
      name = name + "_kindex",
      srcs = srcs,
      # This is a hack to depend on the .jar producer.
      deps = [d + "_kindex" for d in deps],
      data = meta,
      tags = tags,
      visibility = visibility,
      vnames_config = vnames_config,
      testonly = True,
  )
  indexer = "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer"
  tools = []
  if load_plugin:
    # If loaded plugins have deps, those must be included in the loaded jar
    native.java_binary(
        name = name + "_load_plugin",
        main_class = "not.Used",
        runtime_deps = [load_plugin],
    )
    load_plugin_deploy_jar = ":{}_load_plugin_deploy.jar".format(name)
    indexer_opts = indexer_opts + [
      "--load_plugin",
      "$(location {})".format(load_plugin_deploy_jar),
    ]
    tools += [load_plugin_deploy_jar]

  entries = _invoke(index_compilation,
      name = name + "_entries",
      indexer = indexer,
      deps = [kindex],
      tags = tags,
      opts = indexer_opts,
      tools = tools,
      visibility = visibility,
      testonly = True,
  )
  return _invoke(verifier_test,
      name = name,
      size = size,
      tags = tags,
      srcs = [entries] + extra_goals,
      deps = [entries],
      opts = verifier_opts,
      visibility = visibility,
  )

def jvm_verifier_test(name, srcs, deps=[], size="small", tags=[],
                      indexer_opts=[], verifier_opts=["--ignore_dups"],
                      visibility=None):
  kindex = _invoke(jvm_extract_kindex,
      name = name + "_kindex",
      # This is a hack to depend on the .jar producer.
      deps = [d + "_kindex" for d in deps],
      tags = tags,
      visibility = visibility,
      testonly = True,
  )
  indexer = "//kythe/java/com/google/devtools/kythe/analyzers/jvm:class_file_indexer"
  entries = _invoke(index_compilation,
      name = name + "_entries",
      indexer = indexer,
      deps = [kindex],
      tags = tags,
      opts = indexer_opts,
      visibility = visibility,
      testonly = True,
  )
  return _invoke(verifier_test,
      name = name,
      size = size,
      tags = tags,
      srcs = [entries] + srcs,
      deps = [entries],
      opts = verifier_opts,
      visibility = visibility,
  )

def kythe_integration_test(name, srcs, file_tickets, tags=[], size="small"):
  entries = _invoke(atomize_entries,
      name = name + "_atomized_entries",
      file_tickets = file_tickets,
      srcs = [],
      deps = srcs,
      tags = tags,
      testonly = True,
  )
  return _invoke(verifier_test,
      name = name,
      size = size,
      tags = tags,
      deps = [entries],
      opts = ["--ignore_dups"],
  )
