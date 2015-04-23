def go_compile(ctx, pkg, srcs, archive, setupGOPATH=False, extra_archives=[]):
  gotool = ctx.file._go

  archives = []
  recursive_deps = set()
  include_paths = ""
  for dep in ctx.targets.deps:
    include_paths += "-I \"" + dep.go_archive.path + "_gopath\" "
    archives += [dep.go_archive]
    recursive_deps += dep.go_recursive_deps

  for a in extra_archives:
    include_paths += "-I \"" + a.path + "_gopath\" "
  archives += list(extra_archives)
  recursive_deps += archives

  if setupGOPATH:
    symlink = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a_gopath/" + pkg + ".a")
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

def link_binary(ctx, binary, archive, recursive_deps):
  gotool = ctx.file._go

  include_paths = ""
  for a in recursive_deps + [archive]:
    include_paths += "-L \"" + a.path + "_gopath\" "

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

def go_library_impl(ctx):
  archive = ctx.outputs.archive
  pkg = ctx.attr.go_package_prefix + ctx.label.package
  if ctx.attr.package != "":
    pkg = ctx.attr.package

  recursive_deps = go_compile(ctx, pkg, ctx.files.srcs, archive, setupGOPATH=True)
  return struct(go_sources = ctx.files.srcs,
                go_archive = archive,
                go_recursive_deps = recursive_deps)

def go_binary_impl(ctx):
  gotool = ctx.file._go

  archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a")
  go_compile(ctx, 'main', ctx.files.srcs, archive)

  recursive_deps = set()
  for t in ctx.targets.deps:
    recursive_deps += t.go_recursive_deps + [t.go_archive]

  link_binary(ctx, ctx.outputs.executable, archive, recursive_deps)
  return struct()

def go_test_impl(ctx):
  testmain_generator = ctx.file._go_testmain_generator

  lib = ctx.target.library
  pkg = ctx.attr.go_package_prefix + lib.label.package

  test_srcs = ctx.files.srcs
  testmain = ctx.new_file(ctx.configuration.genfiles_dir, ctx.label.name + "main.go")
  cmd = (
      "set -e;" +
      testmain_generator.path + " " + pkg + " " + testmain.path + " " +
      cmd_helper.join_paths(" ", set(test_srcs)) + ";")
  ctx.action(
      inputs = test_srcs + [testmain_generator],
      outputs = [testmain],
      mnemonic = 'GoTestMain',
      command = cmd,
      use_default_shell_env = True)

  archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a")
  go_compile(ctx, pkg, ctx.files.srcs + ctx.target.library.go_sources, archive,
             extra_archives = ctx.target.library.go_recursive_deps,
             setupGOPATH = True)

  test_archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + "main.a")
  go_compile(ctx, 'main', [testmain], test_archive, extra_archives=[archive])

  recursive_deps = lib.go_recursive_deps + [archive]
  for t in ctx.targets.deps:
    recursive_deps += t.go_recursive_deps + [t.go_archive]

  binary = ctx.outputs.executable
  link_binary(ctx, binary, test_archive, recursive_deps)

  runfiles = ctx.runfiles(files = [binary], collect_data = True)
  return struct(runfiles = runfiles)

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
    outputs = {"archive": "%{name}.a"},
)

go_binary = rule(
    go_binary_impl,
    attrs = base_attrs,
    executable = True,
)

go_test = rule(
    go_test_impl,
    attrs = base_attrs + {
        "library": attr.label(providers = [
            "go_sources",
            "go_recursive_deps",
        ]),
        "_go_testmain_generator": attr.label(
            default = Label("//tools/go:testmain_generator"),
            single_file = True,
        ),
    },
    executable = True,
    test = True,
)

def go_package(deps=[], test_deps=[], visibility=None):
  name = PACKAGE_NAME.split("/")[-1]
  go_library(
    name = name,
    srcs = native.glob(
        includes = ["*.go"],
        excludes = ["*_test.go"],
    ),
    deps = deps,
    visibility = visibility,
  )
  test_srcs = native.glob(includes = ["*_test.go"])
  if test_srcs:
    go_test(
      name = name + "_test",
      srcs = test_srcs,
      library = ":" + name,
      deps = test_deps,
      visibility = ["//visibility:private"],
    )
