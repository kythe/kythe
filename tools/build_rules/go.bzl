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

def _link_args(ctx):
  if ctx.var['TARGET_CPU'] == 'darwin':
    return {
      "opt": ["-w"],
      "fastbuild": [],
      "dbg": ["-race"],
    }
  else:
    return {
      "opt": ["-w", "-s"],
      "fastbuild": [],
      "dbg": ["-race"],
    }

def _get_cc_shell_path(ctx):
  cc = ctx.var["CC"]
  if cc[0] == "/":
    return cc
  else:
    return "$PWD/" + cc

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

def _construct_go_path(root, package_map):
  cmd = ['rm -rf ' + root, 'mkdir ' + root]
  for pkg, archive_path in package_map.items():
    pkg_dir = '/'.join([root, pkg[:pkg.rfind('/')]])
    pkg_depth = pkg_dir.count('/')
    pkg_name = pkg[pkg.rfind('/')+1:]
    symlink = pkg_dir + '/' + pkg_name + '.a'
    cmd += [
      'mkdir -p ' + pkg_dir,
      'ln -s ' + ('../' * pkg_depth) + archive_path + ' ' + symlink
    ]
  return cmd

def _construct_package_map(packages):
  archives = []
  package_map = {}
  for pkg in packages:
    archives += [pkg.archive]
    package_map[pkg.name] = pkg.archive.path

  return archives, package_map

# TODO(schroederc): remove this if https://github.com/bazelbuild/bazel/issues/539 is ever fixed
def _dedup_packages(packages):
  seen = set()
  filtered = []
  for pkg in packages:
    if pkg.name not in seen:
      seen += [pkg.name]
      filtered += [pkg]
  return filtered

def _go_compile(ctx, pkg, srcs, archive, extra_packages=[]):
  cgo_link_flags = set([], order="link")
  transitive_deps = []
  transitive_cc_libs = set()
  deps = []
  for dep in ctx.attr.deps:
    deps += [dep.go.package]
    transitive_deps += dep.go.transitive_deps
    cgo_link_flags += dep.go.cgo_link_flags
    transitive_cc_libs += dep.go.transitive_cc_libs

  transitive_deps += extra_packages
  deps += extra_packages
  transitive_deps = _dedup_packages(transitive_deps)

  archives, package_map = _construct_package_map(deps)

  gotool = ctx.file._go
  args = compile_args[ctx.var['COMPILATION_MODE']]
  go_path = archive.path + '.gopath/'
  goroot = ctx.file._go.owner.workspace_root
  cmd = "\n".join([
      'set -e',
      'export GOROOT="' + goroot + '"',
  ] + _construct_go_path(go_path, package_map) + [
      gotool.path + " tool compile " + ' '.join(args) + " -p " + pkg +
      ' -complete -pack -o ' + archive.path + " " + '-trimpath "$PWD" ' +
      '-I "' + go_path + '" ' + cmd_helper.join_paths(" ", set(srcs)),
  ])

  ctx.action(
      inputs = ctx.files._goroot + srcs + archives,
      outputs = [archive],
      mnemonic = 'GoCompile',
      command = cmd,
  )

  return transitive_deps, cgo_link_flags, transitive_cc_libs

def _go_build(ctx, archive):
  cgo_link_flags = set([], order="link")
  transitive_deps = []
  transitive_cc_libs = set()
  deps = []
  for dep in ctx.attr.deps:
    deps += [dep.go.package]
    transitive_deps += dep.go.transitive_deps
    cgo_link_flags += dep.go.cgo_link_flags
    transitive_cc_libs += dep.go.transitive_cc_libs

  transitive_deps = _dedup_packages(transitive_deps)

  cc_inputs = set()
  cgo_compile_flags = set([], order="compile")
  if hasattr(ctx.attr, "cc_deps"):
    for dep in ctx.attr.cc_deps:
      cc_inputs += dep.cc.transitive_headers
      cc_inputs += dep.cc.libs
      transitive_cc_libs += dep.cc.libs

      cgo_link_flags += dep.cc.link_flags
      # cgo directives in libraries will add "-lfoo" to ldflags when looking for a
      # libfoo.a; the compiler driver gets upset if it can't find libfoo.a using a -L
      # path even if it's also provided with an absolute path to libfoo.a. To fix this,
      # we provide both (since providing just the -L versions won't work as some
      # libraries like levigo have special rules regarding linking in other dependencies
      # like snappy).
      for lib in dep.cc.libs:
        cgo_link_flags += ["$PWD/" + lib.path, "-L\"$PWD/$(dirname '" + lib.path + "')\""]

      for flag in dep.cc.compile_flags:
        cgo_compile_flags += [replace_prefix(flag, include_prefix_replacements)]

  archives, package_map = _construct_package_map(deps)

  gotool = ctx.file._go
  args = build_args[ctx.var['COMPILATION_MODE']]
  package_root = '$PWD/' + ctx.label.workspace_root + '/' + ctx.label.package
  gopath_dest = '$GOPATH/src/' + ctx.attr.package

  # Cheat and build the package non-hermetically (usually because there is a cgo dependency)
  goroot = ctx.file._go.owner.workspace_root
  cmd = "\n".join([
      'set -e',
      'export GOROOT="$PWD/' + goroot + '"',
      'export CC=' + _get_cc_shell_path(ctx),
      'export CGO_CFLAGS="' + ' '.join(list(cgo_compile_flags)) + '"',
      'export CGO_LDFLAGS="' + ' '.join(list(cgo_link_flags)) + '"',

      # Setup temporary GOPATH for this package
      'export GOPATH="${TMPDIR:-/tmp}/gopath"',
      'mkdir -p "$(dirname "' + gopath_dest + '")"',
      'if [[ $(uname) == "Darwin" ]]; then',
      '  ln -fsh "' + package_root + '" "' + gopath_dest + '"',
      'else',
      '  ln -sTf "' + package_root + '" "' + gopath_dest + '"',
      'fi',

      gotool.path + ' build -a ' + ' '.join(args) + ' -o ' + archive.path + ' ' + ctx.attr.package,
  ])

  ctx.action(
      inputs = ctx.files._goroot + ctx.files.srcs + archives + list(cc_inputs),
      outputs = [archive],
      mnemonic = 'GoBuild',
      command = cmd,
  )

  return transitive_deps, cgo_link_flags, transitive_cc_libs

def _go_library_impl(ctx):
  archive = ctx.outputs.archive
  if ctx.attr.package == "":
    pkg = _package_name(ctx)
  else:
    pkg = ctx.attr.package
  # TODO(shahms): Figure out why protocol buffer .jar files are being included.
  srcs = FileType([".go"]).filter(ctx.files.srcs)

  if len(srcs) == 0:
    fail('ERROR: ' + str(ctx.label) + ' missing .go srcs')

  package = struct(
    name = pkg,
    archive = archive,
  )

  transitive_deps, cgo_link_flags, transitive_cc_libs = _go_compile(ctx, package.name, srcs, archive)
  return struct(
      go = struct(
          sources = ctx.files.srcs,
          package = package,
          transitive_deps = transitive_deps + [package],
          cgo_link_flags = cgo_link_flags,
          transitive_cc_libs = transitive_cc_libs,
      ),
  )

def _go_build_impl(ctx):
  if ctx.attr.package == "":
    fail('ERROR: missing package attribute')
  if len(FileType([".go"]).filter(ctx.files.srcs)) == 0:
    fail('ERROR: ' + str(ctx.label) + ' missing .go srcs')

  archive = ctx.outputs.archive
  package = struct(
    name = ctx.attr.package,
    archive = archive,
  )

  transitive_deps, cgo_link_flags, transitive_cc_libs = _go_build(ctx, archive)
  return struct(
      go = struct(
          sources = ctx.files.srcs,
          package = package,
          transitive_deps = transitive_deps + [package],
          cgo_link_flags = cgo_link_flags,
          transitive_cc_libs = transitive_cc_libs,
      ),
  )

_build_var_package = "kythe.io/kythe/go/util/build"

def _link_binary(ctx, binary, archive, transitive_deps,
                 stamp=False, extldflags=[], cc_libs=[]):
  gotool = ctx.file._go

  for a in cc_libs:
    extldflags += [a.path]

  dep_archives, package_map = _construct_package_map(transitive_deps)
  go_path = binary.path + '.gopath/'

  mode = ctx.var['COMPILATION_MODE']
  if mode != 'opt':
    # See https://github.com/bazelbuild/bazel/issues/1054
    stamp=False # enable stamping only on optimized release builds

  args = _link_args(ctx)[mode]
  inputs = ctx.files._goroot + [archive] + dep_archives + list(cc_libs)
  goroot = ctx.file._go.owner.workspace_root
  cmd = ['set -e'] + _construct_go_path(go_path, package_map) + [
      'export GOROOT="' + goroot + '"',
      'export GOROOT_FINAL=/usr/local/go',
      'export PATH',
  ]

  tool_cmd = gotool.path + ' tool link '
  if stamp:
    # TODO(schroederc): only stamp when given --stamp flag
    cmd += ['BUILD_VARS=$('
            + ctx.file._format_build_vars.path + ' ' + _build_var_package + ')']
    tool_cmd += '${BUILD_VARS} '
    inputs += [ctx.file._format_build_vars, ctx.info_file, ctx.version_file]
  tool_cmd += ('-extldflags="' + ' '.join(list(extldflags)) + '"'
               + ' ' + ' '.join(args) + ' -L "' + go_path + '"'
               + ' -o ' + binary.path + ' ' + archive.path)
  cmd += [tool_cmd]

  ctx.action(
      inputs = inputs,
      outputs = [binary],
      mnemonic = 'GoLink',
      command = "\n".join(cmd),
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

def _go_binary_impl(ctx):
  gotool = ctx.file._go

  if len(ctx.files.srcs) == 0:
    fail('ERROR: ' + str(ctx.label) + ' missing srcs')

  archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a")
  transitive_deps, cgo_link_flags, transitive_cc_libs = _go_compile(ctx, 'main', ctx.files.srcs, archive)

  _link_binary(ctx, ctx.outputs.executable, archive, transitive_deps,
               stamp=True, extldflags=cgo_link_flags, cc_libs=transitive_cc_libs)

  return binary_struct(ctx)

def _go_test_impl(ctx):
  lib = ctx.attr.library
  pkg = _package_name(ctx)

  # Construct the Go source that executes the tests when run.
  test_srcs = ctx.files.srcs
  testmain = ctx.new_file(ctx.configuration.genfiles_dir, ctx.label.name + "main.go")
  testmain_generator = ctx.file._go_testmain_generator
  cmd = (
      'set -e;' +
      testmain_generator.path + ' ' + pkg + ' ' + testmain.path + ' ' +
      cmd_helper.join_paths(' ', set(test_srcs)) + ';')
  ctx.action(
      inputs = test_srcs + [testmain_generator],
      outputs = [testmain],
      mnemonic = 'GoTestMain',
      command = cmd,
  )

  # Compile the library along with all of its test sources (creating the test package).
  archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + '.a')
  transitive_deps, cgo_link_flags, transitive_cc_libs = _go_compile(
      ctx, pkg, test_srcs + lib.go.sources, archive,
      extra_packages = lib.go.transitive_deps)
  test_pkg = struct(
    name = pkg,
    archive = archive,
  )
  transitive_cc_libs += lib.go.transitive_cc_libs

  # Compile the generated test main.go source
  testmain_archive = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + "main.a")
  _go_compile(ctx, 'main', [testmain] + ctx.files._go_testmain_srcs, testmain_archive,
              extra_packages = [test_pkg])

  # Link the generated test runner
  _link_binary(ctx, ctx.outputs.bin, testmain_archive, transitive_deps + [test_pkg],
               extldflags=cgo_link_flags,
               cc_libs = transitive_cc_libs)

  # Construct a script that runs ctx.outputs.bin and parses the test log.
  test_parser = ctx.file._go_test_parser
  test_script = [
      "#!/bin/bash -e",
      'set -o pipefail',
      'if [[ -n "$XML_OUTPUT_FILE" ]]; then',
      '  %s -test.v "$@" | \\' % (ctx.outputs.bin.short_path),
      '    %s --format xml --out "$XML_OUTPUT_FILE"' % (test_parser.short_path),
      'else',
      '  exec %s "$@"' % (ctx.outputs.bin.short_path),
      'fi'
  ]

  ctx.file_action(
      output = ctx.outputs.executable,
      content = "\n".join(test_script),
      executable = True,
  )

  return binary_struct(ctx, extra_runfiles=[ctx.outputs.bin, test_parser])

base_attrs = {
    "srcs": attr.label_list(
        mandatory = True,
        allow_files = FileType([".go"]),
    ),
    "deps": attr.label_list(
        allow_files = False,
        providers = ["go"],
    ),
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

go_build = rule(
    _go_build_impl,
    attrs = base_attrs + {
        "cc_deps": attr.label_list(
            allow_files = False,
            providers = ["cc"],
        ),
        "package": attr.string(mandatory = True),
    },
    outputs = {"archive": "%{name}.a"},
)

go_library = rule(
    _go_library_impl,
    attrs = base_attrs + {
        "package": attr.string(),
    },
    outputs = {"archive": "%{name}.a"},
)

binary_attrs = base_attrs + {
    "_format_build_vars": attr.label(
        default = Label("//tools/go:format_build_vars.sh"),
        allow_files = True,
        single_file = True,
    ),
    "data": attr.label_list(
        allow_files = True,
        cfg = DATA_CFG,
    ),
}

go_binary = rule(
    _go_binary_impl,
    attrs = binary_attrs,
    executable = True,
)

go_test = rule(
    _go_test_impl,
    attrs = binary_attrs + {
        "library": attr.label(
            mandatory = True,
            providers = ["go"],
        ),
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
               srcs="", deps=[], test_deps=[], test_args=[], test_data=[],
               tests=True, exclude_srcs=[],
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
