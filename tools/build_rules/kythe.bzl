def extract(ctx, kindex, args, mnemonic=None):
  tools = ctx.command_helper([ctx.attr._extractor], {}).resolved_tools
  cmd = '\n'.join([
      "set -e",
      'export KYTHE_ROOT_DIRECTORY="$PWD"',
      'export KYTHE_OUTPUT_DIRECTORY="$(dirname ' + kindex.path + ')"',
      'rm -rf "$KYTHE_OUTPUT_DIRECTORY"',
      'mkdir -p "$KYTHE_OUTPUT_DIRECTORY"',
      ctx.executable._extractor.path + " " + ' '.join(args),
      'mv "$KYTHE_OUTPUT_DIRECTORY"/*.kindex ' + kindex.path])
  ctx.action(
      inputs = ctx.files.srcs + tools,
      outputs = [kindex],
      mnemonic = mnemonic,
      command = cmd,
      use_default_shell_env = True)

def index(ctx, kindex, entries, mnemonic=None):
  tools = ctx.command_helper([ctx.attr._indexer], {}).resolved_tools
  cmd = "set -e;" + ctx.executable._indexer.path + " " + kindex.path + " >" + entries.path
  ctx.action(
      inputs = [kindex] + tools,
      outputs = [entries],
      mnemonic = mnemonic,
      command = cmd,
      use_default_shell_env = True)

def verify(ctx, entries):
  srcs = cmd_helper.join_paths(" ", set(ctx.files.srcs))
  ctx.file_action(
      output = ctx.outputs.executable,
      content = '\n'.join([
        "#!/bin/bash -e",
        "set -o pipefail",
        ctx.executable._verifier.short_path + " --ignore_dups " + srcs + " <" + entries.short_path,
      ]),
      executable = True,
  )
  runfiles = ctx.runfiles(files = ctx.files.srcs + [
      entries,
      ctx.outputs.executable,
      ctx.executable._verifier,
  ], collect_data = True)
  return struct(runfiles = runfiles)

def java_verifier_test_impl(ctx):
  args = []
  for src in ctx.files.srcs:
    args += [src.short_path]
  # TODO(schroederc): add -cp based on ctx.targets.deps

  kindex = ctx.new_file(ctx.configuration.genfiles_dir, ctx.label.name + "/compilation.kindex")
  extract(ctx, kindex, args, mnemonic='JavacExtractor')

  entries = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".entries")
  index(ctx, kindex, entries, mnemonic='JavaIndexer')

  return verify(ctx, entries)

base_attrs = {
    "_verifier": attr.label(
        default = Label("//kythe/cxx/verifier"),
        executable = True,
    ),
}

java_verifier_test = rule(
    java_verifier_test_impl,
    attrs = base_attrs + {
        "srcs": attr.label_list(allow_files = FileType([".java"])),
        "_extractor": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor"),
            executable = True,
        ),
        "_indexer": attr.label(
            default = Label("//kythe/java/com/google/devtools/kythe/analyzers/java:indexer"),
            executable = True,
        ),
    },
    executable = True,
    test = True,
)
