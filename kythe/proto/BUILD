package(default_visibility = ["//visibility:public"])

load("/tools/build_rules/genproto", "genproto")

genproto(
    name = "any_proto",
    src = "any.proto",
)

genproto(
    name = "storage_proto",
    src = "storage.proto",
)

genproto(
    name = "analysis_proto",
    src = "analysis.proto",
    deps = [
        ":any_proto",
        ":storage_proto",
    ],
)

genproto(
    name = "cxx_proto",
    src = "cxx.proto",
)

genproto(
    name = "claim_proto",
    src = "claim.proto",
    deps = [
        ":storage_proto",
    ],
)

genproto(
    name = "xref_proto",
    src = "xref.proto",
)

genproto(
    name = "serving_proto",
    src = "serving.proto",
)

genproto(
    name = "filetree_proto",
    src = "filetree.proto",
)
