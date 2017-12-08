load(
    "@io_bazel_rules_go//go:def.bzl",
    "go_library",
    g_go_binary = "go_binary",
    g_go_test = "go_test",
)

go_binary = g_go_binary  # Re-export the go_binary rule for simpler loads().

go_test = g_go_test  # Re-export the go_test rule for simpler loads().

def go_package_library(name, srcs, deps=[], visibility=None):
  """Macro for minimizing differences between go_library rules."""
  if name != PACKAGE_NAME.split("/")[-1]:
    fail("Package name '%s' must be the same as the Blaze package ('%s')" %
         (name, PACKAGE_NAME.split("/")[-1]))

  native.alias(name = name, actual = ":go_default_library")
  go_library(
      name = "go_default_library",
      srcs = srcs,
      deps = deps,
      visibility = visibility,
  )

# Rule to copy only the executable output from a go_binary, excluding the
# accompanying archive file.
def _go_exec_only_impl(ctx):
  info = ctx.attr.deps[0][DefaultInfo]
  binary = info.files_to_run.executable
  output = ctx.outputs.executable
  ctx.action(
      inputs = [binary],
      outputs = [output],
      mnemonic = "CopyGoTool",
      command = 'cp $1 $2',
      arguments = [binary.path, output.path],
  )
  runfiles = ctx.runfiles(files=[output], collect_data=True, collect_default=True)
  return [DefaultInfo(executable = output, runfiles = runfiles)]

_go_exec_only = rule(
    _go_exec_only_impl,
    attrs = {"deps": attr.label_list()},
    executable = True,
)

def go_release_binary(name, srcs, deps=None, visibility=None):
  """Macro for Go binaries destined to be released."""
  g_go_binary(
      name = name+"_binary",
      srcs = srcs,
      deps = deps,
      visibility = ["//visibility:private"],
      linkstamp = "kythe.io/kythe/go/util/build",
  )
  _go_exec_only(
      name = name,
      deps = [":" + name+"_binary"],
      visibility = visibility,
  )
