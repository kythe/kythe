#
# Copyright 2016 Google Inc. All rights reserved.
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

def _atomize_entries_impl(ctx):
  zcat = ctx.executable._zcat
  entrystream = ctx.executable._entrystream
  postprocessor = ctx.executable._postprocessor
  atomizer = ctx.executable._atomizer

  inputs = set(ctx.files.srcs)
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

def extract(ctx, kindex, extractor, vnames_config, srcs, opts, deps=[], mnemonic="ExtractKindex"):
  env = {
      "KYTHE_ROOT_DIRECTORY": ".",
      "KYTHE_OUTPUT_FILE": kindex.path,
      "KYTHE_VNAMES": vnames_config.path,
  }
  ctx.action(
      inputs = [extractor, vnames_config] + srcs + deps,
      outputs = [kindex],
      mnemonic = mnemonic,
      executable = extractor,
      arguments = (
          [ctx.expand_location(o) for o in opts] +
          [src.path for src in srcs]
      ),
      env = env
  )

def _extract_kindex_impl(ctx):
  extract(
      ctx = ctx,
      kindex = ctx.outputs.kindex,
      extractor = ctx.executable.extractor,
      vnames_config = ctx.file.vnames_config,
      srcs = ctx.files.srcs,
      opts = ctx.attr.opts,
      deps = ctx.files.data,
  )
  return struct(kythe_sources = ctx.files.srcs)

extract_kindex = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
        ),
        "data": attr.label_list(
            allow_files = True,
        ),
        "vnames_config": attr.label(
            default = Label("//kythe/data:vnames_config"),
            allow_single_file = True,
        ),
        "extractor": attr.label(
            mandatory = True,
            executable = True,
            cfg = "host",
        ),
        "opts": attr.string_list(),
    },
    outputs = {
        "kindex": "%{name}.kindex",
    },
    implementation = _extract_kindex_impl,
)

def _java_extract_kindex_impl(ctx):
  jars = []
  for dep in ctx.attr.deps:
    jars += [dep.kythe_jar]

  jar = ctx.new_file(ctx.outputs.kindex, ctx.outputs.kindex.basename + ".jar")
  srcsdir = ctx.new_file(jar, jar.basename + ".srcs")

  args = ctx.attr.opts + [
      "-encoding", "utf-8",
      "-cp", ":".join([j.path for j in jars]),
      "-d", srcsdir.path
  ]
  for src in ctx.files.srcs:
    args += [src.short_path]

  ctx.action(
      inputs = ctx.files.srcs + jars + [ctx.file._jar, ctx.file._javac] + ctx.files._jdk,
      outputs = [jar, srcsdir],
      mnemonic = "MockJavac",
      command = "\n".join([
          "set -e",
          "mkdir -p " + srcsdir.path,
          ctx.file._javac.path + '  "$@"',
          ctx.file._jar.path + " cf " + jar.path + " -C " + srcsdir.path + " .",
      ]),
      arguments = args,
  )
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
  return struct(
      kythe_sources = ctx.files.srcs,
      kythe_jar = jar,
  )

java_extract_kindex = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = True,
        ),
        "deps": attr.label_list(
            providers = ["kythe_jar"],
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
        "_javac": attr.label(
            default = Label("@bazel_tools//tools/jdk:javac"),
            single_file = True,
        ),
        "_jar": attr.label(
            default = Label("@bazel_tools//tools/jdk:jar"),
            single_file = True,
        ),
        "_jdk": attr.label(
            default = Label("@bazel_tools//tools/jdk:jdk"),
            allow_files = True,
        ),
    },
    outputs = {
        "kindex": "%{name}.kindex",
    },
    implementation = _java_extract_kindex_impl,
)

def _bazel_extract_kindex_impl(ctx):
  # TODO(shahms): This is a hack as we get both executable
  #   and .sh from files.scripts but only want the "executable" one.
  #   Unlike `attr.label`, `attr.label_list` lacks an `executable` argument.
  #   Excluding "is_source" files may be overly aggressive, but effective.
  scripts = [s for s in ctx.files.scripts if not s.is_source]
  ctx.action(
      inputs = [
          ctx.executable.extractor,
          ctx.file.vnames_config,
          ctx.file.data
      ] + scripts + ctx.files.srcs,
      outputs = [ctx.outputs.kindex],
      mnemonic = "BazelExtractKindex",
      executable = ctx.executable.extractor,
      arguments = [
          ctx.file.data.path,
          ctx.outputs.kindex.path,
          ctx.file.vnames_config.path
      ] + [script.path for script in scripts]
  )
  return struct(kythe_sources = ctx.files.srcs)

bazel_extract_kindex = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
        ),
        "extractor": attr.label(
            mandatory = True,
            executable = True,
            cfg = "host",
        ),
        "data": attr.label(
            mandatory = True,
            allow_single_file = [".xa"],
        ),
        "scripts": attr.label_list(
            cfg = "host",
            allow_files = True,
        ),
        "vnames_config": attr.label(
            default = Label("//kythe/data:vnames_config"),
            allow_single_file = True,
        ),
    },
    outputs = {
        "kindex": "%{name}.kindex",
    },
    implementation = _bazel_extract_kindex_impl,
)

def _index_compilation_impl(ctx):
  sources = set()
  intermediates = []
  for dep in ctx.attr.deps:
    sources += getattr(dep, "kythe_sources", set())
    for input in dep.files:
      entries = ctx.new_file(ctx.outputs.entries, input.basename + ".entries")
      intermediates += [entries]
      ctx.action(
          outputs = [entries],
          inputs = [ctx.executable.indexer, input],
          arguments = ([ctx.executable.indexer.path] +
                       ctx.attr.opts +
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
  return struct(kythe_sources = sources)

index_compilation = rule(
    attrs = {
        "deps": attr.label_list(
            mandatory = True,
            allow_empty = False,
            allow_files = [".kindex"],
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
  entries = set()
  entries_gz = set()
  sources = set(ctx.files.srcs)
  for dep in ctx.attr.deps:
    sources += getattr(dep, "kythe_sources", set())
    for entry in dep.files:
      if entry.extension == "gz":
        entries_gz += [entry]
      else:
        entries += [entry]
  if not (entries or entries_gz):
    fail("Missing required entry stream input (check your deps!)")
  args = ctx.attr.opts + [src.short_path for src in sources]
  # If no dependency specifies `kythe_sources` and
  # we aren't provided explicit sources, assume `--use_file_nodes`.
  if not sources and "--use_file_nodes" not in args:
    args += ["--use_file_nodes"]
  ctx.file_action(
      output = ctx.outputs.executable,
      content = '\n'.join([
        "#!/bin/bash -e",
        "set -o pipefail",
        "RUNFILES=\"${RUNFILES_DIR:-${TEST_SRCDIR:-${0}.runfiles}}\"",
        "WORKSPACE=\"${TEST_WORKSPACE:-%s}\"" % ctx.workspace_name,
        "cd \"${RUNFILES}/${WORKSPACE}\"",
        "ENTRIES=({})".format(" ".join([e.short_path for e in entries])),
        "ENTRIES_GZ=({})".format(" ".join([e.short_path for e in entries_gz])),
        "(",
        "if [[ $ENTRIES ]]; then cat \"${ENTRIES[@]}\"; fi",
        "if [[ $ENTRIES_GZ ]]; then gunzip -c \"${ENTRIES_GZ}\"; fi",
        ") | {} {}\n".format(
            ctx.executable._verifier.short_path,
            " ".join(args))
      ]),
      executable = True,
  )
  runfiles = ctx.runfiles(files = list(sources + entries + entries_gz) + [
      ctx.outputs.executable,
      ctx.executable._verifier,
  ], collect_data = True)
  return struct(runfiles = runfiles, kythe_entries = entries + entries_gz)

verifier_test = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
        ),
        "deps": attr.label_list(
            mandatory = True,
            allow_files = [
                ".entries",
                ".entries.gz",
            ],
        ),
        "_verifier": attr.label(
            default = Label("//kythe/cxx/verifier"),
            executable = True,
            cfg = "target",
        ),
        "opts": attr.string_list(),
    },
    test = True,
    implementation = _verifier_test_impl,
)

def cc_verifier_test(name, srcs, deps=[], size="small",
                     indexer_opts=["--ignore_unimplemented=true"],
                     verifier_opts=["--ignore_dups"], tags=[]):
  args = ["-std=c++11", "-c"]
  kindexes = [extract_kindex(
        name = name + "_" + src + "_kindex",
        srcs = [src],
        tags = tags,
        extractor = "//kythe/cxx/extractor:cxx_extractor",
        testonly = True,
        opts = select({
            "//conditions:default": args,
            "//:darwin": args + [
                # TODO(zarko): This needs to be autodetected (or does doing so even make sense?)
                "-I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1",
                # TODO(salguarnieri): This could be made more generic with:
                # $(xcrun --show-sdk-path)/usr/include
                "-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include",
            ],
        }),
  ).label() for src in srcs]
  entries = [index_compilation(
      name = name + "_entries",
      deps = kindexes,
      opts = indexer_opts,
      indexer = "//kythe/cxx/indexer/cxx:indexer",
      tags = tags,
      testonly = True,
  ).label()]
  return verifier_test(
      name = name,
      size = size,
      deps = entries + deps,
      opts = verifier_opts,
      tags = tags,
  )

def java_verifier_test(name, srcs, meta=[], deps=[], size="small", tags=[],
                       indexer_opts=["--verbose"], verifier_opts=["--ignore_dups"],
                       vnames_config=None, visibility=None):
  kindex = java_extract_kindex(
      name = name + "_kindex",
      srcs = srcs,
      # This is a hack to depend on the .jar producer.
      deps = [d + "_kindex" for d in deps],
      data = meta,
      tags = tags,
      visibility = visibility,
      vnames_config = vnames_config,
      testonly = True,
  ).label()
  entries = index_compilation(
      name = name + "_entries",
      indexer = "//kythe/java/com/google/devtools/kythe/analyzers/java:wrapped_indexer",
      deps = [kindex],
      tags = tags,
      opts = indexer_opts,
      visibility = visibility,
      testonly = True,
  ).label()
  return verifier_test(
      name = name,
      size = size,
      tags = tags,
      deps = [entries],
      opts = verifier_opts,
      visibility = visibility,
  )

def objc_bazel_verifier_test(name, src, data, size="small", tags=[]):
  kindex = bazel_extract_kindex(
      name = name + "_kindex",
      srcs = [src],
      data = data,
      extractor = "//kythe/cxx/extractor:objc_extractor_bazel",
      scripts = [
          "//third_party/bazel:get_devdir",
          "//third_party/bazel:get_sdkroot",
      ],
      tags = tags,
      testonly = True,
  ).label()
  entries = index_compilation(
      name = name + "_entries",
      indexer = "//kythe/cxx/indexer/cxx:indexer",
      deps = [kindex],
      tags = tags,
      testonly = True,
  ).label()
  return verifier_test(
      name = name,
      size = size,
      tags = tags,
      deps = [entries],
  )

def cc_bazel_verifier_test(name, src, data, size="small", tags=[]):
  kindex = bazel_extract_kindex(
      name = name + "_kindex",
      srcs = [src],
      data = data,
      extractor = "//kythe/cxx/extractor:cxx_extractor_bazel",
      tags = tags,
      testonly = True,
  ).label()
  entries = index_compilation(
      name = name + "_entries",
      indexer = "//kythe/cxx/indexer/cxx:indexer",
      deps = [kindex],
      tags = tags,
      testonly = True,
  ).label()
  return verifier_test(
      name = name,
      size = size,
      tags = tags,
      deps = [entries],
  )

def kythe_integration_test(name, srcs, file_tickets, tags=[], size="small"):
  entries = atomize_entries(
      name = name + "_atomized_entries",
      file_tickets = file_tickets,
      srcs = [],
      deps = srcs,
      tags = tags,
      testonly = True,
  ).label()
  return verifier_test(
      name = name,
      size = size,
      tags = tags,
      deps = [entries],
      opts = ["--ignore_dups"],
  )
