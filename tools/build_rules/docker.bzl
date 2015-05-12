def docker_build(name, image_name, src, data=[], deps=[], use_cache=False):
  args = []
  if not use_cache:
    args += ['--force-rm', '--no-cache']
  native.genrule(
    name = name,
    srcs = [src] + deps + data,
    outs = [name + '.done'],
    tags = ["manual"],
    cmd = '\n'.join([
        "rm -rf _docker_ctx",
        "mkdir _docker_ctx",
        "srcs=($(SRCS))",
        "for src in $${srcs[@]:%s}; do" % (1+len(deps)),
        "  dir=$$(dirname $$src)",
        "  dir=$${dir#$(BINDIR)}",
        "  mkdir -p _docker_ctx/$$dir",
        "  cp -L --preserve=all $$src _docker_ctx/$$dir",
        "done",
        "cp $(location %s) _docker_ctx" % (src),
        "cd _docker_ctx",
        "docker build -t %s %s ." % (image_name, ' '.join(args)),
        "touch ../$@",
    ]),
  )
