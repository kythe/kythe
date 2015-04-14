package(default_visibility = ["//visibility:public"])

load("/tools/build_rules/genproto", "genproto_all")

genproto_all(
    name = "any_proto",
    src = "any.proto",
)

genproto_all(
    name = "storage_proto",
    src = "storage.proto",
)

genproto_all(
    name = "analysis_proto",
    src = "analysis.proto",
    deps = [
        ":any_proto",
        ":storage_proto",
    ],
)

genproto_all(
    name = "cxx_proto",
    src = "cxx.proto",
)

genproto_all(
    name = "claim_proto",
    src = "claim.proto",
    deps = [
        ":storage_proto",
    ],
)

genproto_all(
    name = "xref_proto",
    src = "xref.proto",
)

genproto_all(
    name = "serving_proto",
    src = "serving.proto",
)

genproto_all(
    name = "filetree_proto",
    src = "filetree.proto",
)
