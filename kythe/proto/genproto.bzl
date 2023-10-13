load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_files")

def update_generated_protos(name):
    write_source_files(
        name = name,
        additional_update_targets = [
            key
            for key, value in native.existing_rules().items()
            if value["kind"] == "_write_source_file"
        ],
    )
