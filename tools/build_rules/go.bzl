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

def replace_prefix(s, prefixes):
  for p in prefixes:
    if s.startswith(p):
      return s.replace(p, prefixes[p], 1)
  return s

include_prefix_replacements = {
    "-isystem ": "-isystem $PWD/",
    "-iquote ": "-iquote $PWD/",
    "-I ": "-I $PWD/",
}

def _package_name(ctx):
  pkg = ctx.attr._go_package_prefix.go_prefix + ctx.label.package
  if ctx.attr.multi_package:
    pkg += "/" + ctx.label.name
  if pkg.endswith("_go"):
    pkg = pkg[:-3]
  return pkg

def go_compile(ctx, pkg, srcs, archive, setupGOPATH=False, extra_archives=[]):
  gotool = ctx.file._go
  goroot = ctx.files._goroot

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
      cc_inputs += dep.cc.transitive_headers
      cc_inputs += dep.cc.libs
      cgo_link_flags += dep.cc.link_flags

      for flag in dep.cc.compile_flags:
        cgo_compile_flags += [replace_prefix(flag, include_prefix_replacements)]

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
    cmd = "\n".join([
        'if ' + gotool.path + ' tool | grep -q 6g; then',
        '  TOOL=6g',
        'else',
        '  TOOL=compile',
        'fi',
        gotool.path + " tool $TOOL " + ' '.join(args) + " -p " + pkg + " -complete -pack -o " + archive.path + " " +
        include_paths + " " + cmd_helper.join_paths(" ", set(srcs)),
    ])
    mnemonic = 'GoCompile'

  outs = [archive]
  cmd = "\n".join([
      'set -e',
      'export GOROOT=$PWD/external/local-goroot',
      cmd + "\n",
  ])
  if setupGOPATH:
    cmd += "ln -sf " + symlink_content + " " + symlink.path + ";"
    outs += [symlink]

  ctx.action(
      inputs = srcs + archives + list(cc_inputs) + goroot,
      outputs = outs,
      mnemonic = mnemonic,
      command = cmd,
      use_default_shell_env = True)

  return recursive_deps, transitive_cc_libs, cgo_link_flags

def link_binary(ctx, binary, archive, recursive_deps, extldflags=[], transitive_cc_libs=[]):
  gotool = ctx.file._go
  goroot = ctx.files._goroot

  include_paths = ""
  for a in recursive_deps + [archive]:
    include_paths += '-L "' + a.path + '_gopath" '

  for a in transitive_cc_libs:
    extldflags += [a.path]

  if str(ctx.configuration).find('darwin') >= 0:
    args = link_args_darwin[ctx.var['COMPILATION_MODE']]
  else:
    args = link_args[ctx.var['COMPILATION_MODE']]

  cmd = "\n".join([
      'set -e',
      'export PATH',
      'if ' + gotool.path + ' tool | grep -q 6l; then',
      '  TOOL=6l',
      'else',
      '  TOOL=link',
      'fi',
      gotool.path + ' tool $TOOL -extldflags="' + ' '.join(extldflags) + '"'
      + ' ' + ' '.join(args) + ' ' + include_paths
      + ' -o ' + binary.path + ' ' + archive.path + ';',
  ])

  ctx.action(
      inputs = list(recursive_deps) + [archive] + list(transitive_cc_libs) + goroot,
      outputs = [binary],
      mnemonic = 'GoLink',
      command = cmd,
      use_default_shell_env = True)

def go_library_impl(ctx):
  archive = ctx.outputs.archive
  if ctx.attr.package == "":
    pkg = _package_name(ctx)
  else:
    pkg = ctx.attr.package
  # TODO(shahms): Figure out why protocol buffer .jar files are being included.
  srcs = FileType([".go"]).filter(ctx.files.srcs)

  recursive_deps, transitive_cc_libs, link_flags = go_compile(ctx, pkg, srcs, archive, setupGOPATH=True)
  return struct(
      go_sources = ctx.files.srcs,
      go_archive = archive,
      go_recursive_deps = recursive_deps,
      link_flags = link_flags,
      transitive_cc_libs = transitive_cc_libs,
  )

def binary_struct(ctx, extra_runfiles=[]):
  runfiles = ctx.runfiles(
      files = [ctx.outputs.executable] + extra_runfiles,
      collect_data = True,
  )
  return struct(
      args = ctx.attr.args,
      runfiles = runfiles,
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

  return binary_struct(ctx)

def go_test_impl(ctx):
  testmain_generator = ctx.file._go_testmain_generator
  testmain_srcs = ctx.files._go_testmain_srcs

  lib = ctx.attr.library
  pkg = _package_name(ctx)

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
  go_compile(ctx, 'main', [testmain] + testmain_srcs, test_archive, extra_archives=[archive])

  recursive_deps = lib.go_recursive_deps + [archive]
  transitive_cc_libs = lib.transitive_cc_libs
  link_flags = lib.link_flags
  for t in ctx.attr.deps:
    recursive_deps += t.go_recursive_deps + [t.go_archive]
    transitive_cc_libs += t.transitive_cc_libs
    link_flags += t.link_flags

  link_binary(ctx, ctx.outputs.bin, test_archive, recursive_deps,
              extldflags = link_flags,
              transitive_cc_libs = transitive_cc_libs)

  test_parser = ctx.file._go_test_parser
  ctx.file_action(
      output = ctx.outputs.executable,
      content = "\n".join([
        "#!/bin/bash -e",
        'set -o pipefail',
        'if [[ -n "$XML_OUTPUT_FILE" ]]; then',
        '  %s -test.v "$@" | \\' % (ctx.outputs.bin.short_path),
        '    %s --format xml --out "$XML_OUTPUT_FILE"' % (test_parser.short_path),
        'else',
        '  exec %s "$@"' % (ctx.outputs.bin.short_path),
        'fi'
      ]),
      executable = True,
  )

  return binary_struct(ctx, extra_runfiles=[ctx.outputs.bin, test_parser])

base_attrs = {
    "srcs": attr.label_list(allow_files = FileType([".go"])),
    "deps": attr.label_list(
        allow_files = False,
        providers = ["go_archive"],
    ),
    "go_build": attr.bool(),
    "multi_package": attr.bool(),
    "_go_package_prefix": attr.label(
        default = Label("//external:go_package_prefix"),
        providers = ["go_prefix"],
        allow_files = False,
    ),
    "_go": attr.label(
        default = Label("//tools/go"),
        allow_files = True,
        single_file = True,
    ),
    "_goroot": attr.label(
        default = Label("//tools/go:goroot"),
        allow_files = True,
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

binary_attrs = base_attrs + {
    "data": attr.label_list(
        allow_files = True,
        cfg = DATA_CFG,
    ),
}

go_binary = rule(
    go_binary_impl,
    attrs = binary_attrs,
    executable = True,
)

go_test = rule(
    go_test_impl,
    attrs = binary_attrs + {
        "library": attr.label(providers = [
            "go_sources",
            "go_recursive_deps",
        ]),
        "_go_testmain_generator": attr.label(
            default = Label("//tools/go:testmain_generator"),
            single_file = True,
        ),
        "_go_test_parser": attr.label(
            default = Label("//tools/go:parse_test_output"),
            single_file = True,
        ),
        "_go_testmain_srcs": attr.label(
            default = Label("//tools/go:testmain_srcs"),
            allow_files = FileType([".go"]),
        ),
    },
    executable = True,
    outputs = {"bin": "%{name}.bin"},
    test = True,
)

def go_package(name=None, package=None,
               srcs="", deps=[], test_deps=[], test_args=[], test_data=[], cc_deps=[],
               tests=True, exclude_srcs=[], go_build=False,
               visibility=None):
  if not name:
    name = PACKAGE_NAME.split("/")[-1]

  if srcs and not srcs.endswith("/"):
    srcs += "/"

  exclude = []
  for src in exclude_srcs:
    exclude += [srcs+src]

  lib_srcs, test_srcs = [], []
  for src in native.glob([srcs+"*.go"], exclude=exclude, exclude_directories=1):
    if src.endswith("_test.go"):
      test_srcs += [src]
    else:
      lib_srcs += [src]

  go_library(
    name = name,
    srcs = lib_srcs,
    deps = deps,
    go_build = go_build,
    cc_deps = cc_deps,
    package = package,
    visibility = visibility,
  )

  if tests and test_srcs:
    go_test(
      name = name + "_test",
      srcs = test_srcs,
      library = ":" + name,
      deps = test_deps,
      args = test_args,
      data = test_data,
      visibility = ["//visibility:private"],
    )

# Configuration rule for go packages
def _go_prefix_impl(ctx):
    return struct(go_prefix = ctx.attr.prefix)

go_prefix = rule(
    _go_prefix_impl,
    attrs = {"prefix": attr.string()},
)

def go_package_prefix(prefix):
  if not prefix.endswith("/"):
    prefix = prefix + "/"
  go_prefix(
    name = "go_package_prefix",
    prefix = prefix,
    visibility = ["//visibility:public"],
  )
