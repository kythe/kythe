load("@//tools/cdexec:cdexec.bzl", "rootpath")

def extract(ctx, kindex, args, inputs=[], mnemonic=None):
  tool_inputs, _, input_manifests = ctx.resolve_command(tools=[ctx.attr._extractor])
  env = {
      "KYTHE_ROOT_DIRECTORY": ".",
      "KYTHE_OUTPUT_FILE": kindex.path,
      "KYTHE_VNAMES": ctx.file.vnames_config.path,
  }
  ctx.action(
      inputs = ctx.files.srcs + inputs + tool_inputs + [ctx.file.vnames_config],
      outputs = [kindex],
      mnemonic = mnemonic,
      executable = ctx.executable._extractor,
      arguments = args,
      input_manifests = input_manifests,
      env = env)

def index(ctx, kindex, entries, mnemonic=None):
  tools = [ctx.attr._indexer]
  indexer = [ctx.executable._indexer.path]
  paths = [kindex.path, entries.path]
  # If '_cdexec' is requested, munge the arguments appropriately.
  if hasattr(ctx.executable, "_cdexec"):
    tools += [ctx.attr._cdexec]
    indexer = [ctx.executable._cdexec.path,
               "-t", ctx.label.name + ".XXXXXX",
               rootpath(ctx.executable._indexer.path)]
    paths = [rootpath(kindex.path), entries.path]
  inputs, _, input_manifests = ctx.resolve_command(tools=tools)
  # Quote and execute all arguments, except the last,
  # which is used as a redirection for gzip.
  # _cdexec is required for the Java indexer, which refuses to run
  # in the source directory. See https://kythe.io/phabricator/T70
  cmd = '("${@:1:${#@}-1}" || rm -f "${@:${#@}}") | gzip > "${@:${#@}}"'
  ctx.action(
      inputs = [kindex] + inputs,
      outputs = [entries],
      mnemonic = mnemonic,
      command = cmd,
      arguments = indexer + ctx.attr.indexer_opts + paths,
      input_manifests = input_manifests,
  )

def verify(ctx, entries):
  all_srcs = set(ctx.files.srcs)
  all_entries = set(entries)
  for dep in ctx.attr.deps:
    all_srcs += dep.sources
    all_entries += [dep.entries]

  ctx.file_action(
      output = ctx.outputs.executable,
      content = '\n'.join([
        "#!/bin/bash -e",
        "set -o pipefail",
        "gunzip -c " + " ".join(cmd_helper.template(all_entries, "%{short_path}")) + " | " +
        ctx.executable._verifier.short_path + " " + " ".join(ctx.attr.verifier_opts) +
        " " + cmd_helper.join_paths(" ", all_srcs),
      ]),
      executable = True,
  )
  return ctx.runfiles(files = list(all_srcs) + list(all_entries) + [
      ctx.outputs.executable,
      ctx.executable._verifier,
  ], collect_data = True)

def java_verifier_test_impl(ctx):
  inputs = []
  classpath = []
  for dep in ctx.attr.deps:
    inputs += [dep.jar]
    classpath += [dep.jar.path]

  jar = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".jar")
  srcs_out = jar.path + '.srcs'

  args = ['-encoding', 'utf-8', '-cp', ':'.join(classpath), '-d', srcs_out]
  for src in ctx.files.srcs:
    args += [src.short_path]

  ctx.action(
      inputs = ctx.files.srcs + inputs + [ctx.file._jar, ctx.file._javac] + ctx.files._jdk,
      outputs = [jar],
      mnemonic = 'MockJavac',
      command = '\n'.join([
          'set -e',
          'mkdir -p ' + srcs_out,
          ctx.file._javac.path + '  "$@"',
          ctx.file._jar.path + ' cf ' + jar.path + ' -C ' + srcs_out + ' .',
      ]),
      arguments = args,
  )

  kindex = ctx.new_file(ctx.configuration.genfiles_dir, ctx.label.name + "/compilation.kindex")
  extract(ctx, kindex, args, inputs=inputs+[jar], mnemonic='JavacExtractor')

  entries = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".entries.gz")
  index(ctx, kindex, entries, mnemonic='JavaIndexer')

  runfiles = verify(ctx, [entries])
  return struct(
      runfiles = runfiles,
      jar = jar,
      entries = entries,
      sources = ctx.files.srcs,
      files = set([kindex, entries]),
  )

def cc_verifier_test_impl(ctx):
  entries = []

  for src in ctx.files.srcs:
    args = ['-std=c++11']
    if ctx.var['TARGET_CPU'] == 'darwin':
      # TODO(zarko): This needs to be autodetected (or does doing so even make
      # sense?)
      args += ['-I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1']
      args += ['-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include']
    args += ['-c', src.short_path]
    kindex = ctx.new_file(ctx.label.name + ".compilation/" + src.short_path + ".kindex")
    extract(ctx, kindex, args, inputs=[src], mnemonic='CcExtractor')
    entry = ctx.new_file(ctx.label.name + ".compilation/" + src.short_path + ".entries.gz")
    entries += [entry]
    index(ctx, kindex, entry, mnemonic='CcIndexer')

  runfiles = verify(ctx, entries)
  return struct(
      runfiles = runfiles,
  )

base_attrs = {
    "vnames_config": attr.label(
        default = Label("//kythe/data:vnames_config"),
        allow_files = True,
        single_file = True,
    ),
    "_verifier": attr.label(
        default = Label("//kythe/cxx/verifier"),
        executable = True,
    ),
    "indexer_opts": attr.string_list([]),
    "verifier_opts": attr.string_list(["--ignore_dups"]),
}

java_verifier_test = rule(
    java_verifier_test_impl,
    attrs = base_attrs + {
        "srcs": attr.label_list(allow_files = FileType([".java"])),
        "deps": attr.label_list(
            allow_files = False,
            providers = [
                "entries",
                "sources",
                "jar",
            ],
        ),
        "_extractor": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor"),
            executable = True,
        ),
        "_indexer": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/analyzers/java:indexer"),
            executable = True,
        ),
        "indexer_opts": attr.string_list(["--verbose"]),
        "_cdexec": attr.label(
            default = Label("//tools/cdexec:cdexec"),
            executable = True,
        ),
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
    executable = True,
    test = True,
)

cc_verifier_test = rule(
    cc_verifier_test_impl,
    attrs = base_attrs + {
        "srcs": attr.label_list(allow_files = FileType([
            ".cc",
            ".h",
        ])),
        "deps": attr.label_list(
            allow_files = False,
        ),
        "_extractor": attr.label(
            default = Label("//kythe/cxx/extractor:cxx_extractor"),
            executable = True,
        ),
        "_indexer": attr.label(
            default = Label("//kythe/cxx/indexer/cxx:indexer"),
            executable = True,
        ),
        "indexer_opts": attr.string_list(["--ignore_unimplemented=true"]),
    },
    executable = True,
    output_to_genfiles = True,
    test = True,
)
