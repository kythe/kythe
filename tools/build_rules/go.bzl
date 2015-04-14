def go_compile(ctx, pkg, archive, setupGOPATH):
  gotool = ctx.file._go
  srcs = ctx.files.srcs

  archives = []
  recursive_deps = set()
  include_paths = ""
  for dep in ctx.targets.deps:
    include_paths += "-I \"$(dirname " + dep.go_archive.path + ")/gopath\" "
    archives += [dep.go_archive]
    recursive_deps += dep.go_recursive_deps
  recursive_deps += archives

  if setupGOPATH:
    symlink = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + "/gopath/" + pkg + ".a")
    symlink_content = ctx.label.name + ".a"
    for component in pkg.split("/"):
      symlink_content = "../" + symlink_content

  if ctx.attr.go_build:
    # Cheat and build the package non-hermetically (usually because there is a cgo dependency)
    # TODO(schroederc): add -a flag to regain some hermeticity
    cmd = (
        "export PATH;" +
        "GOPATH=\"$PWD/" + ctx.label.package + "\" " +
        gotool.path + " build -o " + archive.path + " " + ctx.attr.package)
    mnemonic = 'GoBuild'
  else:
    cmd = (
        gotool.path + " tool 6g -p " + pkg + " -complete -pack -o " + archive.path + " " +
        include_paths + " " + cmd_helper.join_paths(" ", set(srcs)))
    mnemonic = 'GoCompile'

  outs = [archive]
  cmd = "set -e;" + cmd + "\n"
  if setupGOPATH:
    cmd += "ln -sf " + symlink_content + " " + symlink.path + ";"
    outs += [symlink]

  ctx.action(
      inputs = srcs + archives + [gotool],
      outputs = outs,
      mnemonic = mnemonic,
      command = cmd,
      use_default_shell_env = True)

  return recursive_deps

def go_library_impl(ctx):
  archive = ctx.outputs.archive
  pkg = ctx.attr.go_package_prefix + ctx.label.package
  if ctx.attr.package != "":
    pkg = ctx.attr.package

  recursive_deps = go_compile(ctx, pkg, archive, True)
  return struct(go_archive = archive,
                go_recursive_deps = recursive_deps)

def go_binary_impl(ctx):
  gotool = ctx.file._go

  archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a")
  go_compile(ctx, 'main', archive, False)

  binary = ctx.outputs.executable
  include_paths = ""
  recursive_deps = set()
  for t in ctx.targets.deps:
    deps = t.go_recursive_deps + [t.go_archive]
    recursive_deps += deps
    for a in deps:
      include_paths += "-L \"$(dirname " + a.path + ")/gopath\" "

  cmd = (
      "set -e;" +
      "export PATH;" +
      gotool.path + " tool 6l " + include_paths + " -o " + binary.path + " " +
      include_paths + " " + archive.path + ";")

  ctx.action(
      inputs = list(recursive_deps) + [archive, gotool],
      outputs = [binary],
      mnemonic = 'GoLink',
      command = cmd,
      use_default_shell_env = True)

  return struct()

base_attrs = {
    "srcs": attr.label_list(allow_files = FileType([".go"])),
    "deps": attr.label_list(
        allow_files = False,
        providers = [
            "go_archive",
        ],
    ),
    "go_build": attr.int(),
    # TODO(schroederc): put package prefix into common configuration file
    "go_package_prefix": attr.string(default = "kythe.io/"),
    "_go": attr.label(
        default = Label("//tools/go"),
        allow_files = True,
        single_file = True,
    ),
}

go_library = rule(
    go_library_impl,
    attrs = base_attrs + {
        "package": attr.string(),
    },
    outputs = {"archive": "%{name}/%{name}.a"},
)

go_binary = rule(
    go_binary_impl,
    attrs = base_attrs,
    executable = True,
)
