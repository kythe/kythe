def asciidoc(name, src, attrs={}, confs=[], partial=False, data=[], tools=[], tags=None):
  args = ['--backend', 'html']
  for k, v in attrs:
    a = '--attribute=' + k
    if v:
      args += [a + '=' + v]
    else:
      args += [a + '!']
  for f in confs:
    args += ['--conf-file=$(location ' + f + ')']
  if partial:
    args += '--no-header-footer'
  out = name + '.html'

  native.genrule(
    name = name,
    srcs = [src] + data + confs,
    outs = [out],
    tags = tags,
    tools = tools,
    output_to_bindir = 1,
    cmd = '\n'.join([
        'export BINDIR="$$PWD/bazel-out/host/bin"',
        'export OUTDIR="$$PWD/$(@D)"',
        "asciidoc %s -o $(@) $(location %s)" % (' '.join(args), src)]))
