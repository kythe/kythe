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
load("@io_bazel_rules_go//go:def.bzl", "go_environment_vars")

# Emit a shell script that sets up the environment needed by
# the extractor to capture dependencies.
def _emit_setup_script(ctx, script, output, srcs, deps):
  env     = go_environment_vars(ctx) # for GOOS and GOARCH
  tmpdir  = output.dirname + '/tmp'
  pkgdir  = tmpdir + '/pkg/%s_%s' % (env['GOOS'], env['GOARCH'])
  outpack = output.path + '_pack'
  cmds    = ['set -e', 'mkdir -p ' + pkgdir]

  # Link the source files and dependencies into a common temporary directory.
  # Source files need to be made relative to the temp directory.
  ups = tmpdir.count('/') + 1
  cmds += ['ln -s "%s%s" "%s"' % ('../'*ups, src.path, tmpdir)
           for src in srcs]
  for path, ipath in deps.items():
    fullpath = '/'.join([pkgdir, ipath])
    tups = fullpath.count('/')
    cmds += [
        'mkdir -p ' + fullpath.rsplit('/', 1)[0],
        "ln -s '%s%s' '%s.a'" % ('../'*tups, path, fullpath),
    ]

  # Invoke the extractor on the temp directory.
  goroot = '/'.join(ctx.files._goroot[0].path.split('/')[:-2])
  cmds.append(' '.join([
      ctx.files._extractor[-1].path,
      '-v',
      '-output_dir', outpack,
      '-goroot', goroot,
      '-gopath', tmpdir,
      '-bydir', tmpdir,
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
  depfiles= [dep.go_library_object for dep in ctx.attr.library.direct_deps]
  deps   = {dep.path: ctx.attr.library.transitive_go_importmap[dep.path]
            for dep in depfiles}
  srcs   = list(ctx.attr.library.go_sources)
  output = ctx.outputs.archive
  script = _emit_setup_script(ctx, 'setup.sh', output, srcs, deps)
  ctx.action(
      mnemonic   = 'GoIndexPack',
      executable = script,
      outputs    = [output],
      inputs     = srcs + depfiles + ctx.files._goroot,
  )
  return struct(zipfile = output)

# Generate an index pack with the compilations captured from a single Go
# library or binary rule. The output is written as a single ZIP file that
# contains the index pack directory.
go_indexpack = rule(
    _go_indexpack,
    attrs = {
        'library': attr.label(
            providers = [
                'go_sources',
                'go_library_object',
                'direct_deps',
                'transitive_go_importmap',
            ],
            mandatory = True,
        ),

        # The location of the Go extractor binary.
        '_extractor': attr.label(
            default = Label('//kythe/go/extractors/cmd/gotool'),
            executable = True,
            cfg        = 'host',
        ),

        # The location of the Go toolchain, needed to resolve standard
        # library packages.
        '_goroot': attr.label(
            default = Label('@io_bazel_rules_go_toolchain//:toolchain'),
        ),
    },
    outputs = {'archive': '%{name}.zip'},
    fragments = ['cpp'], # required to isolate GOOS and GOARCH
)
