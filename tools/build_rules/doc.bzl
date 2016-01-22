def asciidoc(name, src, attrs={}, confs=[], data=[], tools=[], tags=None):
  args = ['--backend', 'html', '--no-header-footer']
  for k, v in attrs:
    a = '--attribute=' + k
    if v:
      args += [a + '=' + v]
    else:
      args += [a + '!']
  for f in confs:
    args += ['--conf-file=$(location ' + f + ')']
  out = name + '.html'

  native.genrule(
    name = name,
    srcs = [src] + data + confs,
    outs = [out],
    local = 1,
    tags = tags,
    tools = tools,
    output_to_bindir = 1,
    cmd = '\n'.join([
        'export BINDIR="$$PWD/bazel-out/host/bin"',
        'export OUTDIR="$$PWD/$(@D)"',
        'export LOGFILE="$$(mktemp -t \"XXXXXXasciidoc\")"',
        'export PATH',
        'trap "rm \"$${LOGFILE}\"" EXIT ERR INT',
        "asciidoc %s -o $(@) $(location %s) 2> \"$${LOGFILE}\"" % (' '.join(args), src),
        'cat $${LOGFILE}',
        '! grep -q -e "filter non-zero exit code" -e "no output from filter" "$${LOGFILE}"']))
