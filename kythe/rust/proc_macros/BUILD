load(":proc_macros.bzl", "proc_macro_bundle")

# Allows loading a set of proc-macro dylibs, for use when parsing source code.
# The required data is provided by a proc_macro_bundle().
rust_library(
    name = "proc_macros",
    srcs = ["proc_macros.rs"],
    deps = [
        "//rust_analyzer",
        "//rust_analyzer:proc_macro_srv",
        "//crates_io/log/v1:log",
        "//absl/status:status_rs",
    ],
)

# A set of widely-used and well-behaved macros that source tools should support.
# This is a Fileset compatible with proc_macros::load() (see `:proc_macros`).
proc_macro_bundle(
    name = "standard_proc_macros",
    deps = [
        # ... 
        # Supported proc macro libraries go here
        # ...
    ],
)
