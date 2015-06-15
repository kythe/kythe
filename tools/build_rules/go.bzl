compile_args = {
    "opt": [],
    "fastbuild": [],
    "dbg": ["-race"],
}

build_args = {
    "opt": [],
    "fastbuild": [],
    "dbg": ["-race"],
}

link_args = {
    "opt": [
        "-w",
        "-s",
    ],
    "fastbuild": [
        "-w",
        "-s",
    ],
    "dbg": ["-race"],
}

link_args_darwin = {
    # https://github.com/golang/go/issues/10254
    "opt": [
        "-w",
    ],
    "fastbuild": [
        "-w",
    ],
    "dbg": ["-race"],
}

def go_compile(ctx, pkg, srcs, archive, setupGOPATH=False, extra_archives=[]):
  gotool = ctx.file._go

  archives = []
  recursive_deps = set()
  include_paths = ""
  transitive_cc_libs = set()
  cgo_link_flags = []
  for dep in ctx.attr.deps:
    include_paths += "-I \"" + dep.go_archive.path + "_gopath\" "
    archives += [dep.go_archive]
    recursive_deps += dep.go_recursive_deps
    transitive_cc_libs += dep.transitive_cc_libs
    cgo_link_flags += dep.link_flags

  cc_inputs = set()
  cgo_compile_flags = []
  if hasattr(ctx.attr, "cc_deps"):
    for dep in ctx.attr.cc_deps:
      if not hasattr(dep.cc, "compile_flags"):
        fail('Newer Bazel version with CcSkylarkApiProvider.compile_flags support needed')
      cc_inputs += dep.cc.transitive_headers
      cc_inputs += dep.cc.libs
      cgo_link_flags += dep.cc.link_flags

      for flag in dep.cc.compile_flags:
        cgo_compile_flags += [flag
                              .replace('-isystem ', '-isystem $PWD/')
                              .replace('-iquote ', '-iquote $PWD/')
                              .replace('-I ', '-I $PWD/')]

      transitive_cc_libs += dep.cc.libs
      for lib in dep.cc.libs:
        cgo_link_flags += ["$PWD/" + lib.path]

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
    args = build_args[ctx.var['COMPILATION_MODE']]
    cmd = "\n".join([
        'export CC=' + ctx.var['CC'],
        'export CGO_CFLAGS="' + ' '.join(cgo_compile_flags) + '"',
        'export CGO_LDFLAGS="' + ' '.join(cgo_link_flags) + '"',
        'export GOPATH="$PWD/' + ctx.label.package + '"',
        gotool.path + ' build -a ' + ' '.join(args) + ' -o ' + archive.path + ' ' + ctx.attr.package,
    ])
    mnemonic = 'GoBuild'
  else:
    args = compile_args[ctx.var['COMPILATION_MODE']]
    cmd = (
        gotool.path + " tool 6g " + ' '.join(args) + " -p " + pkg + " -complete -pack -o " + archive.path + " " +
        include_paths + " " + cmd_helper.join_paths(" ", set(srcs)))
    mnemonic = 'GoCompile'

  outs = [archive]
  cmd = "set -e;" + cmd + "\n"
  if setupGOPATH:
    cmd += "ln -sf " + symlink_content + " " + symlink.path + ";"
    outs += [symlink]

  ctx.action(
      inputs = srcs + archives + [gotool] + list(cc_inputs),
      outputs = outs,
      mnemonic = mnemonic,
      command = cmd,
      use_default_shell_env = True)

  return recursive_deps, transitive_cc_libs, cgo_link_flags

def link_binary(ctx, binary, archive, recursive_deps, extldflags=[], transitive_cc_libs=[]):
  gotool = ctx.file._go

  include_paths = ""
  for a in recursive_deps + [archive]:
    include_paths += '-L "' + a.path + '_gopath" '

  for a in transitive_cc_libs:
    extldflags += [a.path]

  if str(ctx.configuration).find('darwin') >= 0:
    args = link_args_darwin[ctx.var['COMPILATION_MODE']]
  else:
    args = link_args[ctx.var['COMPILATION_MODE']]

  cmd = (
      "set -e;" +
      "export PATH;" +
      gotool.path + ' tool 6l -extldflags="' + ' '.join(extldflags) + '"'
      + ' ' + ' '.join(args) + ' ' + include_paths
      + ' -o ' + binary.path + ' ' + archive.path + ';')

  ctx.action(
      inputs = list(recursive_deps) + [archive, gotool] + list(transitive_cc_libs),
      outputs = [binary],
      mnemonic = 'GoLink',
      command = cmd,
      use_default_shell_env = True)

def go_library_impl(ctx):
  archive = ctx.outputs.archive
  pkg = ctx.attr.go_package_prefix + ctx.label.package
  if ctx.attr.package != "":
    pkg = ctx.attr.package

  recursive_deps, transitive_cc_libs, link_flags = go_compile(ctx, pkg, ctx.files.srcs, archive, setupGOPATH=True)
  return struct(
      go_sources = ctx.files.srcs,
      go_archive = archive,
      go_recursive_deps = recursive_deps,
      link_flags = link_flags,
      transitive_cc_libs = transitive_cc_libs,
  )

def go_binary_impl(ctx):
  gotool = ctx.file._go

  archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a")
  go_compile(ctx, 'main', ctx.files.srcs, archive)

  recursive_deps = set()
  link_flags = []
  transitive_cc_libs = set()
  for t in ctx.attr.deps:
    recursive_deps += t.go_recursive_deps + [t.go_archive]
    transitive_cc_libs += t.transitive_cc_libs
    link_flags += t.link_flags

  link_binary(ctx, ctx.outputs.executable, archive, recursive_deps,
              extldflags=link_flags,
              transitive_cc_libs=transitive_cc_libs)
  return struct()

def go_test_impl(ctx):
  testmain_generator = ctx.file._go_testmain_generator

  lib = ctx.attr.library
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
  go_compile(ctx, pkg, ctx.files.srcs + ctx.attr.library.go_sources, archive,
             extra_archives = ctx.attr.library.go_recursive_deps,
             setupGOPATH = True)

  test_archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + "main.a")
  go_compile(ctx, 'main', [testmain], test_archive, extra_archives=[archive])

  recursive_deps = lib.go_recursive_deps + [archive]
  transitive_cc_libs = lib.transitive_cc_libs
  link_flags = lib.link_flags
  for t in ctx.attr.deps:
    recursive_deps += t.go_recursive_deps + [t.go_archive]
    transitive_cc_libs += t.transitive_cc_libs
    link_flags += t.link_flags

  binary = ctx.outputs.executable
  link_binary(ctx, binary, test_archive, recursive_deps,
              extldflags = link_flags,
              transitive_cc_libs = transitive_cc_libs)

  runfiles = ctx.runfiles(files = [binary], collect_data = True)
  return struct(runfiles = runfiles)

base_attrs = {
    "srcs": attr.label_list(allow_files = FileType([".go"])),
    "deps": attr.label_list(
        allow_files = False,
        providers = ["go_archive"],
    ),
    "go_build": attr.bool(),
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
        "cc_deps": attr.label_list(
            allow_files = False,
            providers = ["cc"],
        ),
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
