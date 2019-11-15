workspace(name = "kythe_release")

load("//:external.bzl", "kythe_release_dependencies")

kythe_release_dependencies()

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

rules_proto_dependencies()

rules_proto_toolchains()

protobuf_deps()
