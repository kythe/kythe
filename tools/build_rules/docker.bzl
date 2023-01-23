def docker_build(
        name,
        image_name,
        src = "Dockerfile",
        deps = [],
        data = [],
        stage_only = False,
        tags = None,
        use_cache = False):
    done_marker = name + ".done"

    data_locs = []
    for d in data:
        data_locs.append("$(locations " + d + ")")

    args = []
    if not use_cache:
        args += ["--force-rm", "--no-cache"]

    # Use cp -p instead of cp --preserve=all because --preserve is a non-standard
    # flag.
    build_step = []
    if not stage_only:
        build_step = ["docker build -t %s %s ." % (image_name, " ".join(args))]
    cmd = [
        "set +u",
        "CTX=$@.ctx",
        'rm -rf "$$CTX"',
        'mkdir "$$CTX"',
        "srcs=(%s)" % (" ".join(data_locs)),
        "for src in $${srcs[@]}; do",
        "  dir=$$(dirname $$src)",
        "  if [[ $$dir == bazel-out/* ]]; then",
        "    dir=$${dir#*/*/*/}",
        "  fi",
        '  mkdir -p "$$CTX/$$dir"',
        '  cp -Lp $$src "$$CTX/$$dir"',
        "done",
        "cp $(location %s) \"$$CTX\"" % (src),
        'cd "$$CTX"',
    ] + build_step + [
        "cd - >/dev/null",
        "touch $@",
    ]

    print("docker dir is ", done_marker)

    native.genrule(
        name = name,
        cmd = "\n".join(cmd),
        srcs = [src] + data + deps,
        outs = [done_marker],
        tags = tags + ["docker"],
        local = 1,
    )
