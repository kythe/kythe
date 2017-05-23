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

# Bazel rules to extract Go compilations from library targets for testing the
# Go cross-reference indexer.

# We depend on the Go toolchain to identify the effective OS and architecture
# settings and to resolve standard library packages.
load(
    "@io_bazel_rules_go//go:def.bzl",
    "go_environment_vars",
    "go_library",
    "go_library_attrs",
)
load(
    "//tools:build_rules/kythe.bzl",
    "kythe_integration_test",
    "verifier_test",
)

# Emit a shell script that sets up the environment needed by the extractor to
# capture dependencies and runs the extractor.
def _emit_extractor_script(ctx, script, output, srcs, deps, ipath, data):
  env     = go_environment_vars(ctx) # for GOOS and GOARCH
  tmpdir  = output.dirname + '/tmp'
  srcdir  = tmpdir + '/src/' + ipath
  pkgdir  = tmpdir + '/pkg/%s_%s' % (env['GOOS'], env['GOARCH'])
  outpack = output.path + '_pack'
  extras  = []
  cmds    = ['set -e',
             'mkdir -p ' + pkgdir, 'mkdir -p ' + srcdir]

  # Link the source files and dependencies into a common temporary directory.
  # Source files need to be made relative to the temp directory.
  ups = srcdir.count('/') + 1
  cmds += ['ln -s "%s%s" "%s"' % ('../'*ups, src.path, srcdir)
           for src in srcs]
  for path, dpath in deps.items():
    fullpath = '/'.join([pkgdir, dpath])
    tups = fullpath.count('/')
    cmds += [
        'mkdir -p ' + fullpath.rsplit('/', 1)[0],
        "ln -s '%s%s' '%s.a'" % ('../'*tups, path, fullpath),
    ]

  # Gather any extra data dependencies.
  for target in data:
    for f in target.files:
      cmds.append('ln -s "%s%s" "%s"' % ('../'*ups, f.path, srcdir))
      extras.append(srcdir + '/' + f.path.rsplit('/', 1)[-1])

  # Invoke the extractor on the temp directory.
  goroot = '/'.join(ctx.files._goroot[0].path.split('/')[:-2])
  cmds.append(' '.join([
      ctx.files._extractor[-1].path,
      '-output_dir', outpack,
      '-goroot', goroot,
      '-gopath', tmpdir,
      '-extra_files', "'%s'" % ','.join(extras),
      '-bydir',
      srcdir,
  ]))

  # Pack the results into a ZIP archive, so we have a single output.
  cmds += [
      'cd ' + output.dirname,
      "zip -qr '%s' '%s'" % (output.basename, output.basename+'_pack'),
      '',
  ]

  f = ctx.new_file(ctx.configuration.bin_dir, script)
  ctx.file_action(output=f, content='\n'.join(cmds), executable=True)
  return f

def _go_indexpack(ctx):
  depfiles = [dep.go_library_object for dep in ctx.attr.library.direct_deps]
  deps   = {dep.path: ctx.attr.library.transitive_go_importmap[dep.path]
            for dep in depfiles}
  srcs   = list(ctx.attr.library.go_sources)
  tools  = ctx.files._goroot + ctx.files._extractor
  output = ctx.outputs.archive
  data   = ctx.attr.data
  ipath  = ctx.attr.import_path
  if not ipath:
    ipath = 'test/' + srcs[0].path.rsplit('/', 1)[-1].rsplit('.', 1)[0]
  extras = []
  for target in data:
    extras += target.files.to_list()

  script = _emit_extractor_script(ctx, ctx.label.name+'-extract.sh',
                                  output, srcs, deps, ipath, data)
  ctx.action(
      mnemonic   = 'GoIndexPack',
      executable = script,
      outputs    = [output],
      inputs     = srcs + extras + depfiles + tools,
  )
  return struct(zipfile = output)

_library_providers = [
    "go_sources",
    "go_library_object",
    "direct_deps",
    "transitive_go_importmap",
]

# Generate an index pack with the compilations captured from a single Go
# library or binary rule. The output is written as a single ZIP file that
# contains the index pack directory.
go_indexpack = rule(
    _go_indexpack,
    attrs = {
        "library": attr.label(
            providers = _library_providers,
            mandatory = True,
        ),

        # The import path to attribute to the compilation.
        # If omitted, use the base name of the source directory.
        "import_path": attr.string(),

        # Additional data files to include in each compilation.
        "data": attr.label_list(
            allow_empty = True,
            allow_files = True,
        ),

        # The location of the Go extractor binary.
        "_extractor": attr.label(
            default = Label("//kythe/go/extractors/cmd/gotool"),
            executable = True,
            cfg = "target",
        ),

        # The location of the Go toolchain, needed to resolve standard
        # library packages.
        "_goroot": attr.label(
            default = Label("@io_bazel_rules_go_toolchain//:toolchain"),
        ),
    },
    fragments = ["cpp"],  # required to isolate GOOS and GOARCH
    outputs = {"archive": "%{name}.zip"},
)

def _go_entries(ctx):
  pack     = ctx.attr.indexpack.zipfile
  indexer  = ctx.files._indexer[-1]
  iargs    = [indexer.path, '-zip']
  output   = ctx.outputs.entries

  # If the test wants marked source, enable support for it in the indexer.
  if ctx.attr.has_marked_source:
    iargs.append('-code')

  # If the test wants linkage metadata, enable support for it in the indexer.
  if ctx.attr.metadata_suffix:
    iargs += ['-meta', ctx.attr.metadata_suffix]

  iargs += [pack.path, '| gzip >'+output.path]

  cmds = ['set -e', 'set -o pipefail', ' '.join(iargs), '']
  ctx.action(
      mnemonic = 'GoIndexPack',
      command  = '\n'.join(cmds),
      outputs  = [output],
      inputs   = [pack] + ctx.files._indexer,
  )
  return struct(kythe_entries = [output])

# Run the Kythe indexer on the output that results from a go_indexpack rule.
go_entries = rule(
    _go_entries,
    attrs = {
        # The go_indexpack output to pass to the indexer.
        "indexpack": attr.label(
            providers = ["zipfile"],
            mandatory = True,
        ),

        # Whether to enable explosion of MarkedSource facts.
        "has_marked_source": attr.bool(default = False),

        # The suffix used to recognize linkage metadata files, if non-empty.
        "metadata_suffix": attr.string(default = ""),

        # The location of the Go indexer binary.
        "_indexer": attr.label(
            default = Label("//kythe/go/indexer/cmd/go_indexer"),
            executable = True,
            cfg = "data",
        ),
    },
    outputs = {"entries": "%{name}.entries.gz"},
)

def _go_verifier_test(ctx):
  entries  = ctx.attr.entries.kythe_entries
  verifier = ctx.file._verifier
  vargs    = [verifier.short_path,
              '--use_file_nodes', '--show_goals', '--check_for_singletons']

  if ctx.attr.log_entries:
    vargs.append('--show_protos')
  if ctx.attr.allow_duplicates:
    vargs.append('--ignore_dups')

  # If the test wants marked source, enable support for it in the verifier.
  if ctx.attr.has_marked_source:
    vargs.append('--convert_marked_source')

  cmds = ['set -e', 'set -o pipefail', ' '.join(
      ['zcat', entries.short_path, '|'] + vargs),
  '']
  ctx.file_action(output=ctx.outputs.executable,
                  content='\n'.join(cmds), executable=True)
  return struct(
      runfiles = ctx.runfiles([verifier, entries]),
  )

def go_verifier_test(name, entries, size="small", tags=[],
                     log_entries=False, has_marked_source=False,
                     allow_duplicates=False):
  opts = ['--use_file_nodes', '--show_goals', '--check_for_singletons']
  if log_entries:
    opts.append('--show_protos')
  if allow_duplicates:
    opts.append('--ignore_dups')

  # If the test wants marked source, enable support for it in the verifier.
  if has_marked_source:
    opts.append('--convert_marked_source')
  return verifier_test(
      name = name,
      size = size,
      tags = tags,
      deps = [entries],
      opts=opts
  )

# Shared extract/index logic for the go_indexer_test/go_integration_test rules.
def _go_indexer(name, srcs, deps=[], import_path='',
                    data=None,
                    has_marked_source=False,
                    allow_duplicates=False,
                    metadata_suffix=''):
  testlib = name+'_lib'
  go_library(
      name = testlib,
      srcs = srcs,
      deps = deps,
  )
  testpack = name+'_pack'
  go_indexpack(
      name = testpack,
      library = ':'+testlib,
      import_path = import_path,
      data = data if data else [],
  )
  entries = name+'_entries'
  go_entries(
      name = entries,
      indexpack = ':'+testpack,
      has_marked_source = has_marked_source,
      metadata_suffix = metadata_suffix,
  )
  return entries

# A convenience macro to generate a test library, pass it to the Go indexer,
# and feed the output of indexing to the Kythe schema verifier.
def go_indexer_test(name, srcs, deps=[], import_path='', size = 'small',
                    log_entries=False, data=None,
                    has_marked_source=False,
                    allow_duplicates=False,
                    metadata_suffix=''):
  entries = _go_indexer(
      name = name,
      srcs = srcs,
      deps = deps,
      data = data,
      import_path = import_path,
      has_marked_source = has_marked_source,
      metadata_suffix = metadata_suffix,
  )
  go_verifier_test(
      name = name,
      size = size,
      entries = ':'+entries,
      log_entries = log_entries,
      has_marked_source = has_marked_source,
      allow_duplicates = allow_duplicates,
  )

# A convenience macro to generate a test library, pass it to the Go indexer,
# and feed the output of indexing to the Kythe integration test pipeline.
def go_integration_test(name, srcs, deps=[], data=None,
                        file_tickets=[],
                        import_path='', size='small',
                        has_marked_source=False,
                        metadata_suffix=''):
  entries = _go_indexer(
      name = name,
      srcs = srcs,
      deps = deps,
      data = data,
      import_path = import_path,
      has_marked_source = has_marked_source,
      metadata_suffix = metadata_suffix,
  )
  kythe_integration_test(
      name = name,
      size = size,
      srcs = [':'+entries],
      file_tickets = file_tickets,
  )
