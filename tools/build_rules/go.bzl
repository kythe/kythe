def go_library_impl(ctx):
  gotool = ctx.file._go
  srcs = ctx.files.srcs

  archive = ctx.outputs.archive
  # TODO(schroederc): put package prefix into configuration file
  pkg = "kythe.io/" + ctx.label.package
  if ctx.attr.package != "":
      pkg = ctx.attr.package

  archives = []
  include_paths = ""
  for dep in ctx.targets.deps:
    include_paths += "-I \"$(dirname " + dep.go_archive.path + ")/gopath\" "
    archives += [dep.go_archive]

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

  cmd = "set -e;" + cmd + "\nln -sf " + symlink_content + " " + symlink.path + ";"
  ctx.action(
      inputs = srcs + archives + [gotool],
      outputs = [archive, symlink],
      mnemonic = mnemonic,
      command = cmd,
      use_default_shell_env = True)

  return struct(go_archive = archive)

go_library = rule(
    go_library_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = FileType([".go"])),
        "package": attr.string(),
        "go_build": attr.int(),
        "deps": attr.label_list(
            allow_files = False,
            providers = [
                "go_archive",
            ],
        ),
        "_go": attr.label(
            default = Label("//tools/go"),
            allow_files = True,
            single_file = True,
        ),
    },
    outputs = {"archive": "%{name}/%{name}.a"},
)
