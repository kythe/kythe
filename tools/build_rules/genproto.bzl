load("go", "go_library")

def proto_package_impl(ctx):
  return struct(proto_src = ctx.file.src)

genproto_base_attrs = {
    "src": attr.label(
        allow_files = FileType([".proto"]),
        single_file = True,
    ),
    "deps": attr.label_list(
        allow_files = False,
        providers = ["proto_src"],
    ),
    "has_services": attr.bool(),
}

proto_package = rule(
    proto_package_impl,
    attrs = genproto_base_attrs,
)

def genproto_java_impl(ctx):
  src = ctx.file.src
  protoc = ctx.file._protoc

  srcjar = ctx.new_file(ctx.configuration.genfiles_dir, ctx.label.name + ".srcjar")
  java_srcs = srcjar.path + ".srcs"

  inputs = [src, protoc]
  java_grpc_cmd = ""
  if ctx.attr.has_services:
    java_grpc_plugin = ctx.file._protoc_grpc_plugin_java
    inputs += [java_grpc_plugin]
    java_grpc_cmd = (
        protoc.path + " --java_rpc_out=" + java_srcs +
        " --plugin=protoc-gen-java_rpc=" + java_grpc_plugin.path + " " + src.path)

  java_cmd = '\n'.join([
      "set -e",
      "rm -rf " + java_srcs,
      "mkdir " + java_srcs,
      protoc.path + " --java_out=" + java_srcs + " " + src.path,
      java_grpc_cmd,
      "jar cMf " + srcjar.path + " -C " + java_srcs + " .",
      "rm -rf " + java_srcs,
  ])
  ctx.action(
      inputs = inputs,
      outputs = [srcjar],
      mnemonic = 'ProtocJava',
      command = java_cmd,
      use_default_shell_env = True)

  return struct(files = set([srcjar]))

genproto_java = rule(
    genproto_java_impl,
    attrs = genproto_base_attrs + {
        "_protoc": attr.label(
            default = Label("//third_party/proto:protoc"),
            allow_files = True,
            single_file = True,
        ),
        "_protoc_grpc_plugin_java": attr.label(
            default = Label("//third_party/grpc-java:plugin"),
            executable = True,
            single_file = True,
        ),
    },
)

def genproto_go_impl(ctx):
  src = ctx.file.src
  protoc = ctx.file._protoc

  srcname = ctx.label.name
  if srcname.endswith("_go_src"):
    srcname = srcname[:-7]

  go_src = ctx.new_file(ctx.configuration.genfiles_dir, srcname + ".pb.go")
  outdir = go_src.path + ".dir"
  protoc_gen_go = ctx.file._protoc_gen_go
  go_pkg = ctx.attr.go_package_prefix + ctx.label.package + "/" + ctx.label.name
  proto_src_deps = []

  go_proto_import_path = ""
  for dep in ctx.attr.deps:
    proto_src_deps += [dep.proto_src]
    go_proto_import_path += ",M" + dep.proto_src.path + "=" + ctx.attr.go_package_prefix + dep.label.package + "/" + dep.label.name

  grpc = ""
  if ctx.attr.has_services:
    grpc = "plugins=grpc,"

  go_cmd = "\n".join([
      "set -e",
      "rm -rf " + outdir,
      "mkdir -p " + outdir,
      protoc.path + " --plugin=" + protoc_gen_go.path + " --go_out="+grpc+"import_path=" +
      go_pkg + go_proto_import_path + ":" + outdir + " " + src.path,
      "find " + outdir + " -type f -name '*.go' -exec mv -f {} " + go_src.path + " ';'",
      "rm -rf " + outdir,
  ])
  ctx.action(
      inputs = [src, protoc, protoc_gen_go] + proto_src_deps,
      outputs = [go_src],
      mnemonic = 'ProtocGo',
      command = go_cmd,
      use_default_shell_env = True)

  return struct(files = set([go_src]))

genproto_go = rule(
    genproto_go_impl,
    attrs = genproto_base_attrs + {
        "_protoc": attr.label(
            default = Label("//third_party/proto:protoc"),
            allow_files = True,
            single_file = True,
        ),
        "_protoc_gen_go": attr.label(
            default = Label("//third_party/go:protoc-gen-go"),
            single_file = True,
            allow_files = True,
        ),
        # TODO(schroederc): put package prefix into common configuration file
        "go_package_prefix": attr.string(default = "kythe.io/"),
    },
)

def proto_library(name, src=None, deps=[], visibility=None,
                  has_services=0,
                  gen_java=False, gen_go=False, gen_cc=False,
                  go_package=None):
  if not src:
    if name.endswith("_proto"):
      src = name[:-6]+".proto"
    else:
      src = name+".proto"
  proto_package(name=name, src=src, deps=deps)

  if gen_java:
    genproto_java(
        name = name+"_java_src",
        src = src,
        deps = deps,
        has_services = has_services,
        visibility = ["//visibility:private"],
    )
    java_deps = ["//third_party/proto:protobuf_java"]
    if has_services:
      java_deps += [
          "//third_party/grpc-java",
          "//third_party/guava",
          "//third_party/jsr305_annotations:jsr305",
      ]
    for dep in deps:
      java_deps += [dep+"_java"]
    native.java_library(
        name  = name+"_java",
        srcs = [name+"_java_src"],
        deps = java_deps,
        visibility = visibility,
    )

  if gen_go:
    genproto_go(
        name = name+"_go_src",
        src = src,
        deps = deps,
        has_services = has_services,
        visibility = ["//visibility:private"],
    )
    go_deps = ["//third_party/go:protobuf"]
    if has_services:
      go_deps += [
        "//third_party/go:context",
        "//third_party/go:grpc",
      ]
    for dep in deps:
      go_deps += [dep+"_go"]
    if not go_package:
      go_package = "kythe.io/" + PACKAGE_NAME + "/" + name
    go_library(
        name  = name+"_go",
        srcs = [name+"_go_src"],
        deps = go_deps,
        package = go_package,
        visibility = visibility,
    )

  if gen_cc:
    # We'll guess that the repository is set up such that a .proto in
    # //foo/bar has the package foo.bar. `location` is substituted with the
    # relative path to its label from the workspace root.
    proto_path = "$(location %s)" % src
    proto_hdr = src[:-6] + ".pb.h"
    proto_src = src[:-6] + ".pb.cc"
    proto_srcgen_rule = name + "_cc_src"
    proto_lib = name + "_cc"
    protoc = "//third_party/proto:protoc"
    proto_cmd = "$(location %s) --cpp_out=$(GENDIR)/ %s" % (protoc, proto_path)
    cc_deps = ["//third_party/proto:protobuf"]
    proto_deps = [src, protoc]
    for dep in deps:
      cc_deps += [dep + "_cc"]
      proto_deps += [dep]
    native.genrule(
        name = proto_srcgen_rule,
        visibility = visibility,
        outs = [proto_hdr, proto_src],
        srcs = proto_deps,
        cmd = proto_cmd,
    )
    native.cc_library(
        name = proto_lib,
        visibility = visibility,
        hdrs = [proto_hdr],
        srcs = [":" + proto_srcgen_rule],
        defines = ["GOOGLE_PROTOBUF_NO_RTTI"],
        deps = cc_deps,
    )
