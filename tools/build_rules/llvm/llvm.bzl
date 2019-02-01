load("@io_kythe//tools/build_rules/llvm:configure_file.bzl", "configure_file")
load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//lib:collections.bzl", "collections")

def _repo_path(path):
    if native.repository_name() == "@":
        return path
    return paths.join("external", native.repository_name()[1:], path.lstrip("/"))

def _llvm_build_deps(ctx, name):
    # TODO(shahms): Do this transformation during generation.
    llvm_to_cmake = {
        "Scalar": "ScalarOpts",
        "IPO": "ipo",
        "ObjCARC": "ObjCARCOpts",
    }
    cmake_to_llvm = dict([(v, k) for k, v in llvm_to_cmake.items()])
    name = _replace_prefix(name, "LLVM", "")
    name = cmake_to_llvm.get(name, name)
    return [
        ":LLVM" + llvm_to_cmake.get(d, d)
        for d in ctx._config.llvm_build_deps.get(name, [])
    ]

def _root_path(ctx):
    return paths.join(*[s.path for s in ctx._state]).lstrip("/")

def _join_path(root, path):
    """Special handling for absolute paths."""
    if path.startswith("/"):
        return paths.normalize(path.lstrip("/"))  # CMake paths are all "rooted" at the workspace.
    return paths.normalize(paths.join(root.lstrip("/"), path))

def _llvm_headers(root):
    root = _replace_prefix(root, "lib/", "include/llvm/")
    return native.glob([_join_path(root, "**/*.*")])

def _replace_prefix(value, prefix, repl):
    if value.startswith(prefix):
        return repl + value[len(prefix):]
    return value

def _clang_headers(root):
    root = _replace_prefix(root, "tools/clang/lib/", "tools/clang/include/clang/")
    return native.glob([_join_path(root, "**/*.*")])

def _llvm_srcglob(root, additional_header_dirs = []):
    srcglob = [_join_path(root, "*.h"), _join_path(root, "*.inc")]
    for dir in additional_header_dirs:
        srcglob.extend([
            paths.join(_join_path(root, dir), "*.h"),
            paths.join(_join_path(root, dir), "*.inc"),
        ])
    return native.glob(srcglob)

def _clang_srcglob(root):
    return native.glob([_join_path(root, "**/*.h"), _join_path(root, "**/*.inc")])

def _current(ctx):
    return ctx._state[-1]

def _colonize(name):
    for prefix in [":", "/", "@"]:
        if name.startswith(prefix):
            return name
    return ":" + name

def _genfile_name(path):
    return path.lstrip("/").replace("/", "_").replace(".", "_").replace("-", "_")

def _group_sections(sections, args, leader = "srcs"):
    blocks = [(leader, [])]
    for arg in args:
        if arg in sections:
            blocks.append((arg.lower(), []))
            continue
        blocks[-1][-1].append(arg)
    return blocks

def _make_kwargs(ctx, name, args = [], sections = [], leader = "srcs"):
    kwargs = {}
    kwargs.update(ctx._config.target_defaults.get(name, {}))
    for key, values in _group_sections(sections, list(args), leader = leader):
        kwargs.setdefault(key, []).extend([v for v in values if v])
    return kwargs

def _configure_file(ctx, src, out, *unused):
    if not (src and out):
        return
    root = _root_path(ctx)
    configure_file(
        name = _genfile_name(out),
        src = _join_path(root, src),
        out = _join_path(root, out),
        defines = select({
            "//conditions:default": ctx._config.cmake_defines.default,
            "@io_kythe//:darwin": ctx._config.cmake_defines.darwin,
        }),
    )

def _llvm_library(ctx, name, srcs, hdrs = [], deps = [], additional_header_dirs = [], **kwargs):
    # TODO(shahms): Do something with these
    kwargs.pop("link_libs", None)

    root = _root_path(ctx)
    depends = ([":llvm-c"] + deps +
               kwargs.pop("depends", []) +
               _llvm_build_deps(ctx, name))
    depends = collections.uniq([_colonize(d) for d in depends])
    defs = native.glob([_join_path(root, "*.def")])
    if defs:
        native.cc_library(
            name = name + "_defs",
            textual_hdrs = defs,
            visibility = ["//visibility:private"],
        )
        depends.append(":" + name + "_defs")
    sources = (
        [_join_path(root, s) for s in srcs] +
        _llvm_srcglob(root, additional_header_dirs) +
        _current(ctx).table_outs
    )
    includes = [root]
    if "/Target/" in root:
        parts = root.split("/")
        target = parts[parts.index("Target") + 1]
        target_root = "/".join(parts[:parts.index("Target") + 2])
        if target_root and target_root != root:
            sources += native.glob([_join_path(target_root, "**/*.h")])
            includes.append(target_root)
        depends.append(":" + target + "CommonTableGen")
        kind = _replace_prefix(name, "LLVM" + target, "")
        target_kind_deps = {
            "Utils": [":LLVMMC", ":LLVMCodeGen"],
            "Info": [":LLVMMC", ":LLVMTarget"],
            "AsmPrinter": [":LLVMTarget", ":LLVMCodeGen"],
        }
        depends += target_kind_deps.get(kind, [])

    native.cc_library(
        name = name,
        srcs = collections.uniq(sources),
        hdrs = _llvm_headers(root) + hdrs,
        deps = depends,
        copts = ["-I$(GENDIR)/{0} -I{0}".format(_repo_path(i)) for i in includes],
        **kwargs
    )

def _add_llvm_library(ctx, name, *args):
    sections = ["ADDITIONAL_HEADER_DIRS", "LINK_LIBS", "DEPENDS"]
    kwargs = _make_kwargs(ctx, name, list(args), sections)
    if name in ["LLVMHello", "LLVMTestingSupport"]:
        return
    _llvm_library(ctx, name = name, **kwargs)

def _map_llvm_lib(name):
    if name.islower():
        return name[0].title() + name[1:]
    return name

def _clang_library(
        ctx,
        name,
        srcs,
        deps = [],
        depends = [],
        link_libs = [],
        additional_headers = [],
        llvm_link_components = [],
        **kwargs):
    root = _root_path(ctx)
    deps = list(deps) + [":clang-c"]
    deps.extend([_colonize(d) for d in depends])
    deps.extend([_colonize(l) for l in link_libs])
    deps.extend([":LLVM" + _map_llvm_lib(l) for l in llvm_link_components])
    extra_dirs = collections.uniq([
        _repo_path(_join_path(root, paths.dirname(s)))
        for s in srcs
        if paths.dirname(s)
    ] + [_repo_path(root)])

    native.cc_library(
        name = name,
        srcs = [_join_path(root, s) for s in srcs] + _clang_srcglob(root),
        hdrs = _clang_headers(root) + kwargs.pop("hdrs", []),
        deps = collections.uniq(deps),
        copts = ["-I%s -I$(GENDIR)/%s" % (d, d) for d in extra_dirs],
        **kwargs
    )

def _add_clang_library(ctx, name, *args):
    sections = ["ADDITIONAL_HEADERS", "LINK_LIBS", "DEPENDS"]
    kwargs = _make_kwargs(ctx, name, list(args), sections)
    kwargs["llvm_link_components"] = _current(ctx).vars.get("LLVM_LINK_COMPONENTS", [])
    _clang_library(ctx, name, **kwargs)

def _add_tablegen(ctx, name, tag, *srcs):
    root = _root_path(ctx)
    kwargs = _make_kwargs(ctx, name, [_join_path(root, s) for s in srcs])
    kwargs["srcs"].extend(_llvm_srcglob(root))
    deps = [
        ":LLVMSupport",
        ":LLVMTableGen",
        ":LLVMMC",
    ] + kwargs.pop("deps", [])
    native.cc_binary(name = name, deps = deps, **kwargs)

def _set_cmake_var(ctx, key, *args):
    if key in ("LLVM_TARGET_DEFINITIONS", "LLVM_LINK_COMPONENTS", "sources"):
        _current(ctx).vars[key] = args

def _llvm_tablegen(ctx, kind, out, *opts):
    cur = _current(ctx)
    root = _root_path(ctx)
    out = _join_path(root, out)
    src = _join_path(root, cur.vars["LLVM_TARGET_DEFINITIONS"][0])
    cur.table_outs.append(out)
    includes = [root, "include"]
    opts = " ".join(["-I " + _repo_path(i) for i in includes] + list(opts))
    native.genrule(
        name = _genfile_name(out),
        outs = [out],
        srcs = native.glob([
            _join_path(root, "*.td"),  # local_tds
            "include/llvm/**/*.td",  # global_tds
        ]),
        tools = [":llvm-tblgen"],
        cmd = "$(location :llvm-tblgen) %s $(location %s) -o $@" % (opts, src),
    )

def _clang_diag_gen(ctx, comp):
    _clang_tablegen(
        ctx,
        "Diagnostic%sKinds.inc" % comp,
        "-gen-clang-diags-defs",
        "-clang-component=" + comp,
        "SOURCE",
        "Diagnostic.td",
        "TARGET",
        "ClangDiagnostic" + comp,
    )

def _clang_tablegen(ctx, out, *args):
    root = _root_path(ctx)
    out = _join_path(root, out)
    name = _genfile_name(out)
    kwargs = _make_kwargs(ctx, name, args, ["SOURCE", "TARGET", "-I"], leader = "opts")
    src = _join_path(root, kwargs["source"][0])
    includes = ["include/", root] + [
        _join_path(root, p)
        for p in kwargs.get("-i", [])
    ]
    opts = " ".join(["-I " + _repo_path(i) for i in includes] + kwargs["opts"])
    native.genrule(
        name = name,
        outs = [out],
        srcs = native.glob([
            _join_path(root, "*.td"),  # local_tds
            _join_path(paths.dirname(src), "*.td"),  # local_tds
            "include/llvm/**/*.td",  # global_tds
        ]),
        tools = [":clang-tblgen"],
        cmd = "$(location :clang-tblgen) %s $(location %s) -o $@" % (opts, src),
    )
    _current(ctx).gen_hdrs.append(out)
    target = kwargs.get("target")
    if target:
        native.cc_library(name = target[0], textual_hdrs = [out])

def _add_public_tablegen_target(ctx, name):
    table_outs = _current(ctx).table_outs
    includes = []
    for out in table_outs:
        include = paths.dirname(out)
        if include not in includes and "include" in include:
            includes.append(include)
    native.cc_library(
        name = name,
        textual_hdrs = _current(ctx).table_outs + ctx._config.target_defaults.get(name, {}).get("textual_hdrs", []),
        includes = includes,
    )

def _add_llvm_target(ctx, name, *args):
    sources = list(_current(ctx).vars.get("sources", []))
    sources.extend(args)
    _add_llvm_library(ctx, "LLVM" + name, *sources)

def _enter_directory(ctx, path):
    ctx._state.append(struct(
        path = path,
        vars = {},
        table_outs = [],
        gen_hdrs = [],
    ))
    return ctx

def _exit_directory(ctx, path):
    gen_hdrs = _current(ctx).gen_hdrs
    if gen_hdrs:
        native.filegroup(
            name = _genfile_name(_root_path(ctx)) + "_genhdrs",
            srcs = gen_hdrs,
        )
    ctx._state.pop()
    return ctx

def make_context(**kwargs):
    return struct(
        _state = [],
        _config = struct(**kwargs),
        enter_directory = _enter_directory,
        exit_directory = _exit_directory,
        set = _set_cmake_var,
        configure_file = _configure_file,
        add_llvm_library = _add_llvm_library,
        add_llvm_target = _add_llvm_target,
        add_clang_library = _add_clang_library,
        add_tablegen = _add_tablegen,
        tablegen = _llvm_tablegen,
        clang_tablegen = _clang_tablegen,
        clang_diag_gen = _clang_diag_gen,
        add_public_tablegen_target = _add_public_tablegen_target,
    )
