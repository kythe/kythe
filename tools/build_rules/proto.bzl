load("@//tools:build_rules/go.bzl", "go_library")

standard_proto_path = "third_party/proto/src/"

def go_package_name(go_prefix, label):
  return "%s%s/%s" % (go_prefix.go_prefix, label.package, label.name)

def _genproto_impl(ctx):
  proto_src_deps = [src.proto_src for src in ctx.attr.deps]
  inputs, outputs, arguments = [ctx.file.src] + proto_src_deps, [], ["--proto_path=."]
  for src in proto_src_deps:
    if src.path.startswith(standard_proto_path):
      arguments += ["--proto_path=" + standard_proto_path]
      break
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

  go_package = go_package_name(ctx.attr._go_package_prefix, ctx.label)
  if ctx.attr.gen_go:
    outputs += [ctx.outputs.go_src]
    go_cfg = ["import_path=" + go_package, _go_import_path(ctx.attr.deps)]
    if ctx.attr.has_services:
      go_cfg += ["plugins=grpc"]
    genfiles_path = ctx.configuration.genfiles_dir.path
    if ctx.attr.gofast:
      inputs += [ctx.executable._protoc_gen_gofast]
      arguments += [
          "--plugin=" + ctx.executable._protoc_gen_gofast.path,
          "--gofast_out=%s:%s" % (",".join(go_cfg), genfiles_path)
      ]
    else:
      inputs += [ctx.executable._protoc_gen_go]
      arguments += [
          "--plugin=" + ctx.executable._protoc_gen_go.path,
          "--golang_out=%s:%s" % (",".join(go_cfg), genfiles_path)
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
        command = "cp $1 $2")
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
    "gofast": attr.bool(),
    "_protoc": attr.label(
        default = Label("//third_party/proto:protoc"),
        executable = True,
    ),
    "_go_package_prefix": attr.label(
        default = Label("//external:go_package_prefix"),
        providers = ["go_prefix"],
        allow_files = False,
    ),
    "_protoc_gen_go": attr.label(
        default = Label("@go_protobuf//:protoc-gen-golang"),
        executable = True,
    ),
    "_protoc_gen_gofast": attr.label(
        default = Label("@go_gogo_protobuf//:protoc-gen-gofast"),
        executable = True,
    ),
    "_protoc_grpc_plugin_java": attr.label(
        default = Label("//third_party/grpc-java:plugin"),
        executable = True,
    ),
    "gen_cc": attr.bool(),
    "gen_java": attr.bool(),
    "gen_go": attr.bool(),
}

def _genproto_outputs(gen_cc, gen_go, gen_java):
  outputs = {}
  if gen_cc:
    outputs += {
        "cc_hdr": "%{src}.pb.h",
        "cc_src": "%{src}.pb.cc"
    }
  if gen_go:
    outputs += {
        "go_src": "%{src}.pb.go",
    }
  if gen_java:
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
                  gofast=True):
  if not src:
    if name.endswith("_proto"):
      src = name[:-6] + ".proto"
    else:
      src = name + ".proto"
  proto_pkg = genproto(name=name,
                       src=src,
                       deps=deps,
                       has_services=has_services,
                       gen_java=gen_java,
                       gen_go=gen_go,
                       gen_cc=gen_cc,
                       gofast=gofast)

  # TODO(shahms): These should probably not be separate libraries, but
  # allowing upstream *_library and *_binary targets to depend on the
  # proto_library() directly is a challenge.  We'd also need a different
  # workaround for the non-generated any.pb.{h,cc} from the upstream protocol
  # buffer library.
  if gen_java:
    java_deps = ["//third_party/proto:protobuf_java"]
    if has_services:
      java_deps += [
          "//external:guava",
          "//third_party/grpc-java",
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
    go_deps = ["@go_protobuf//:proto"]
    if has_services:
      go_deps += [
        "@go_x_net//:context",
        "@go_grpc//:grpc",
      ]
    for dep in deps:
      go_deps += [dep + "_go"]
    go_library(
        name  = name + "_go",
        srcs = [proto_pkg.label()],
        deps = go_deps,
        multi_package = 1,
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
