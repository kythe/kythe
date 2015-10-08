load("go", "go_library")

standard_proto_path = "third_party/proto/src/"

def _genproto_impl(ctx):
  proto_src_deps = [src.proto_src for src in ctx.attr.deps]
  inputs, outputs, arguments = [ctx.file.src] + proto_src_deps, [], [
      "--proto_path=.",
      # Until we can do this correctly, we will assume any proto file may
      # depend on a "builtin" protocol buffer under third_party/proto/src.
      # TODO(shahms): Do this correctly.
      "--proto_path=" + standard_proto_path,
  ]
  if ctx.attr.gen_cc:
    outputs += [ctx.outputs.cc_hdr, ctx.outputs.cc_src]
    arguments += ["--cpp_out=" + ctx.configuration.genfiles_dir.path]
  if ctx.attr.gen_java:
    if ctx.outputs.java_src.path.endswith(".srcjar"):
      srcjar = ctx.new_file(ctx.outputs.java_src.basename[:-6] + "jar")
    else:
      srcjar = ctx.outputs.java_src
    outputs += [srcjar]
    arguments += ["--java_out=" + srcjar.path]
    if ctx.attr.has_services:
      java_grpc_plugin = ctx.executable._protoc_grpc_plugin_java
      inputs += [java_grpc_plugin]
      arguments += [
          "--plugin=protoc-gen-java_rpc=" + java_grpc_plugin.path,
          "--java_rpc_out=" + srcjar.path
      ]
  go_package = ctx.attr.go_package
  if not go_package:
    go_package = "%s/%s" % (ctx.label.package, ctx.label.name)
  if ctx.attr.gen_go:
    inputs += [ctx.executable._protoc_gen_go]
    outputs += [ctx.outputs.go_src]
    go_cfg = ["import_path=" + go_package, _go_import_path(ctx.attr.deps)]
    if ctx.attr.has_services:
      go_cfg += ["plugins=grpc"]
    genfiles_path = ctx.configuration.genfiles_dir.path
    arguments += [
        "--plugin=" + ctx.executable._protoc_gen_go.path,
        "--go_out=%s:%s" % (",".join(go_cfg), genfiles_path)
    ]

  ctx.action(
      mnemonic = "GenProto",
      inputs = inputs,
      outputs = outputs,
      arguments = arguments + [ctx.file.src.path],
      executable = ctx.executable._protoc)
  # This is required because protoc only understands .jar extensions, but Bazel
  # requires source JAR files end in .srcjar.
  if ctx.attr.gen_java and srcjar != ctx.outputs.java_src:
    ctx.action(
        mnemonic = "FixProtoSrcJar",
        inputs = [srcjar],
        outputs = [ctx.outputs.java_src],
        arguments = [srcjar.path, ctx.outputs.java_src.path],
        command = "mv $1 $2")
    # Fixup the resulting outputs to keep the source-only .jar out of the result.
    outputs += [ctx.outputs.java_src]
    outputs = [e for e in outputs if e != srcjar]

  return struct(files=set(outputs),
                go_package=go_package,
                proto_src=ctx.file.src)

_genproto_attrs = {
    "src": attr.label(
        allow_files = FileType([".proto"]),
        single_file = True,
    ),
    "deps": attr.label_list(
        allow_files = False,
        providers = ["proto_src"],
    ),
    "has_services": attr.bool(),
    "_protoc": attr.label(
        default = Label("//third_party/proto:protoc"),
        executable = True,
    ),
    "_protoc_gen_go": attr.label(
        default = Label("//third_party/go:protoc-gen-go"),
        executable = True,
    ),
    "_protoc_grpc_plugin_java": attr.label(
        default = Label("//third_party/grpc-java:plugin"),
        executable = True,
    ),
    "gen_cc": attr.bool(),
    "gen_java": attr.bool(),
    "gen_go": attr.bool(),
    "go_package": attr.string(),
}

def _genproto_outputs(attrs):
  outputs = {}
  if attrs.gen_cc:
    outputs += {
        "cc_hdr": "%{src}.pb.h",
        "cc_src": "%{src}.pb.cc"
    }
  if attrs.gen_go:
    outputs += {
        "go_src": "%{src}.pb.go",
    }
  if attrs.gen_java:
    outputs += {
        "java_src": "%{src}.srcjar",
    }
  return outputs

genproto = rule(
    _genproto_impl,
    attrs = _genproto_attrs,
    output_to_genfiles = True,
    outputs = _genproto_outputs,
)

def proto_library(name, src=None, deps=[], visibility=None,
                  has_services=False,
                  gen_java=False, gen_go=False, gen_cc=False,
                  go_package=None):
  if not src:
    if name.endswith("_proto"):
      src = name[:-6] + ".proto"
    else:
      src = name + ".proto"
  if not go_package:
    go_package = "kythe.io/" + PACKAGE_NAME + "/" + name
  proto_pkg = genproto(name=name,
                       src=src,
                       deps=deps,
                       has_services=has_services,
                       go_package=go_package,
                       gen_java=gen_java,
                       gen_go=gen_go,
                       gen_cc=gen_cc)

  # TODO(shahms): These should probably not be separate libraries, but
  # allowing upstream *_library and *_binary targets to depend on the
  # proto_library() directly is a challenge.  We'd also need a different
  # workaround for the non-generated any.pb.{h,cc} from the upstream protocol
  # buffer library.
  if gen_java:
    java_deps = ["//third_party/proto:protobuf_java"]
    if has_services:
      java_deps += [
          "//third_party/grpc-java",
          "//third_party/guava",
          "//third_party/jsr305_annotations:jsr305",
      ]
    for dep in deps:
      java_deps += [dep + "_java"]
    native.java_library(
        name  = name + "_java",
        srcs = [proto_pkg.label()],
        deps = java_deps,
        visibility = visibility,
    )

  if gen_go:
    go_deps = ["//third_party/go:protobuf"]
    if has_services:
      go_deps += [
        "//third_party/go:context",
        "//third_party/go:grpc",
      ]
    for dep in deps:
      go_deps += [dep + "_go"]
    go_library(
        name  = name + "_go",
        srcs = [proto_pkg.label()],
        deps = go_deps,
        package = go_package,
        visibility = visibility,
    )

  if gen_cc:
    cc_deps = ["//third_party/proto:protobuf"]
    for dep in deps:
      cc_deps += [dep + "_cc"]
    native.cc_library(
        name = name + "_cc",
        visibility = visibility,
        hdrs = [proto_pkg.label()],
        srcs = [proto_pkg.label()],
        defines = ["GOOGLE_PROTOBUF_NO_RTTI"],
        deps = cc_deps,
    )

def _go_import_path(deps):
  import_map = {}
  for dep in deps:
    if dep.proto_src.path.startswith(standard_proto_path):
      import_map += {dep.proto_src.path[len(standard_proto_path):]: dep.go_package}
    else:
      import_map += {dep.proto_src.path: dep.go_package}
  return ",".join(["M%s=%s" % i for i in import_map.items()])
