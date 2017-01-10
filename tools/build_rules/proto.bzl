load(
    "@io_bazel_rules_go//go:def.bzl",
    "go_library_impl",
    "go_library_attrs",
    "go_library_outputs",
)

standard_proto_path = "third_party/proto/src/"

def _go_proto_library_impl(ctx):
  """Pseudo-go_library rule used to work around the proto import-path mismatch.

  The upstream Go rules require `go_library` rules name to match the package,
  but this isn't true for `proto_library` generated Go rules.  We use this
  rule to create a Go library with a munged import path.
  """
  providers = go_library_impl(ctx)
  package = providers.transitive_go_importmap[ctx.outputs.lib.path]
  if package.endswith("_go"):
    package = package[:-3]
  importmap = providers.transitive_go_importmap + {
      ctx.outputs.lib.path: package,
  }
  return struct(
      label = providers.label,
      runfiles = providers.runfiles,
      files = providers.files,
      direct_deps = providers.direct_deps,
      go_sources = providers.go_sources,
      asm_sources = providers.asm_sources,
      go_library_object = providers.go_library_object,
      transitive_go_library_object = providers.transitive_go_library_object,
      cgo_object = providers.cgo_object,
      transitive_cgo_deps = providers.transitive_cgo_deps,
      transitive_go_importmap = importmap,
  )

_go_proto_library_hack = rule(
    _go_proto_library_impl,
    attrs = go_library_attrs,
    fragments = ["cpp"],
    outputs = go_library_outputs,
)

def go_package_name(go_prefix, label):
  return "%s%s/%s" % (go_prefix.go_prefix, label.package, label.name)

def _genproto_impl(ctx):
  proto_src_deps = [src.proto_src for src in ctx.attr.deps]
  inputs, outputs, arguments = [ctx.file.src] + proto_src_deps, [], ["--proto_path=."]
  for src in proto_src_deps:
    if src.path.startswith(standard_proto_path):
      arguments += ["--proto_path=" + standard_proto_path]
      break
  if ctx.attr.emit_metadata:
    outputs += [ctx.outputs.descriptor]
    arguments += ["-o" + ctx.outputs.descriptor.path, "--include_imports", "--include_source_info"]
  if ctx.attr.gen_cc:
    outputs += [ctx.outputs.cc_hdr, ctx.outputs.cc_src]
    metadata_args = ""
    if ctx.attr.emit_metadata:
      metadata_args = "annotate_headers=1,annotation_pragma_name=kythe_metadata,annotation_guard_name=KYTHE_IS_RUNNING:"
      outputs += [ctx.outputs.cc_meta]
    arguments += ["--cpp_out=" + metadata_args + ctx.configuration.genfiles_dir.path]
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
    genroot = ctx.configuration.genfiles_dir.path
    if ctx.attr.go_package:
      # Handle a "go_package" annotation by moving the output file to the expected location
      gosrc = ctx.new_file(ctx.attr.go_package + "/" + ctx.outputs.go_src.basename)
      genroot = gosrc.root.path + "/" + ctx.label.package
      outputs += [gosrc]
      ctx.action(
          inputs = [gosrc],
          outputs = [ctx.outputs.go_src],
          command = "mv " + gosrc.path + " " + ctx.outputs.go_src.path,
      )
    else:
      outputs += [ctx.outputs.go_src]
    go_cfg = ["import_path=" + go_package, _go_import_path(ctx.attr.deps)]
    if ctx.attr.has_services:
      go_cfg += ["plugins=grpc"]
    if ctx.attr.gofast:
      inputs += [ctx.executable._protoc_gen_gofast]
      arguments += [
          "--plugin=" + ctx.executable._protoc_gen_gofast.path,
          "--gofast_out=%s:%s" % (",".join(go_cfg), genroot)
      ]
    else:
      inputs += [ctx.executable._protoc_gen_go]
      arguments += [
          "--plugin=" + ctx.executable._protoc_gen_go.path,
          "--golang_out=%s:%s" % (",".join(go_cfg), genroot)
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
    "emit_metadata": attr.bool(),
    "gofast": attr.bool(),
    "go_package": attr.string(),
    "_protoc": attr.label(
        default = Label("//third_party/proto:protoc"),
        executable = True,
        cfg = "host",
    ),
    "_go_package_prefix": attr.label(
        default = Label("//:go_prefix"),
        providers = ["go_prefix"],
        allow_files = False,
    ),
    "_protoc_gen_go": attr.label(
        default = Label("@go_protobuf//:protoc-gen-golang"),
        executable = True,
        cfg = "host",
    ),
    "_protoc_gen_gofast": attr.label(
        default = Label("@go_gogo_protobuf//:protoc-gen-gofast"),
        executable = True,
        cfg = "host",
    ),
    "_protoc_grpc_plugin_java": attr.label(
        default = Label("//third_party/grpc-java:plugin"),
        executable = True,
        cfg = "host",
    ),
    "gen_cc": attr.bool(),
    "gen_java": attr.bool(),
    "gen_go": attr.bool(),
}

def _genproto_outputs(gen_cc, gen_go, gen_java, emit_metadata):
  outputs = {}
  if emit_metadata:
    outputs += {
        "descriptor": "%{src}.descriptor",
    }
  if gen_cc:
    if emit_metadata:
      outputs += {
          "cc_meta": "%{src}.pb.h.meta"
      }
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

def proto_library(name, srcs, deps=[], visibility=None,
                  has_services=False,
                  java_api_version=0, go_api_version=0, cc_api_version=0,
                  emit_metadata=False, gofast=True, go_package=None):
  if java_api_version not in (None, 0, 2):
    fail("java_api_version must be 2 if present")
  if go_api_version not in (None, 0, 2):
    fail("go_api_version must be 2 if present")
  if cc_api_version not in (None, 0, 2):
    fail("cc_api_version must be 2 if present")

  proto_pkg = genproto(name=name,
                       src=srcs[0],
                       deps=deps,
                       has_services=has_services,
                       gen_java=bool(java_api_version),
                       gen_go=bool(go_api_version),
                       gen_cc=bool(cc_api_version),
                       emit_metadata=emit_metadata,
                       gofast=gofast, go_package=go_package)

  # TODO(shahms): These should probably not be separate libraries, but
  # allowing upstream *_library and *_binary targets to depend on the
  # proto_library() directly is a challenge.  We'd also need a different
  # workaround for the non-generated any.pb.{h,cc} from the upstream protocol
  # buffer library.
  if java_api_version:
    java_deps = ["//third_party/proto:protobuf_java"]
    if has_services:
      java_deps += [
          "@com_google_guava_guava//jar",
          "@com_google_code_findbugs_jsr305//jar",
          "//third_party/grpc-java",
      ]
    for dep in deps:
      java_deps += [dep + "_java"]
    native.java_library(
        name  = name + "_java",
        srcs = [proto_pkg.label()],
        deps = java_deps,
        visibility = visibility,
    )

  if go_api_version:
    go_deps = ["@go_protobuf//:proto"]
    if has_services:
      go_deps += [
        "@go_x_net//:context",
        "@go_grpc//:grpc",
      ]
    for dep in deps:
      go_deps += [dep + "_go"]
    _go_proto_library_hack(
        name  = name + "_go",
        srcs = [proto_pkg.label()],
        deps = go_deps,
        visibility = visibility,
    )

  if cc_api_version:
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
