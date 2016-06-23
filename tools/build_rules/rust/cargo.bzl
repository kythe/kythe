def _cargo_cmd(loc, out_dir, cmd, outs=[]):
     return "\n".join([
             "$(location //tools/build_rules/rust:run_cargo.sh) " + loc + " \"" + cmd + "\"",
        ] + [
            "mv " + loc + out_dir + out + " $(location " + out + ") " for out in outs 
        ])

_automatic_srcs = [
    "Cargo.toml",
    "src/**/*.rs",
]

def cargo_bin(name, loc, srcs=[], bin_name=None, release=False):
    bin_name = name if bin_name == None else bin_name
    output_path = "target/" + ("release/" if release else "debug/") 
    native.genrule(
        name = name,
        srcs = srcs + native.glob(_automatic_srcs),
        outs = [bin_name],
        cmd = _cargo_cmd(
          loc = loc,
          out_dir = output_path,
          cmd = "build --bin " + (" --release" if release else "") + bin_name,
          outs = [bin_name],
        ),
        local = 1,
        tools = ["//tools/build_rules/rust:run_cargo.sh"],
        tags = ["manual", "requires-network"],
    )

def _cargo_generic_lib(name, ext, loc, srcs, lib_name, release, visibility = "//visibility:public", tags = []):
    output_path = "target/" + ("release/" if release else "debug/")
    native.genrule(
            name = name,
            srcs = srcs + native.glob(_automatic_srcs),
            outs = [lib_name + ext],
            cmd = _cargo_cmd(
                    loc = loc,
                    out_dir = output_path,
                    cmd = "build --lib" + (" --release" if release else ""),
                    outs = [lib_name + ext],
                    ),
            local = 1,
            tools = ["//tools/build_rules/rust:run_cargo.sh"],
            tags = ["manual", "requires-network"] + tags,
            visibility = [visibility]
            )

def cargo_lib(name, loc, srcs=[], lib_name=None, lib_type="rlib", release=False):
    lib_name = (name if lib_name == None else lib_name) 
    if lib_type == "rlib":
        _cargo_generic_lib(name, ".rlib", loc, srcs, lib_name, release)
    elif lib_type == "dylib":
        # dylibs have inconsistent file names across platforms
        # and bazel doesn't allow for selecting over outputs.
        # To circumvent this we create platform specific rules.
        # The wrong rule will always fail on the wrong platform.
        _cargo_generic_lib("_" + name + "-k8", ".so", 
                           loc, srcs, lib_name, release, 
                           "//visibility:private", 
                           tags = ["arc-ignore"])
        _cargo_generic_lib("_" + name + "-darwin", ".dylib",
                           loc, srcs, lib_name, release,
                           "//visibility:private",
                           tags = ["arc-ignore"])

        # We then select the appropriate rule
        native.filegroup(
            name = name,
            srcs = select({
                "//tools/build_rules/rust:k8": ["_" + name + "-k8"],
                "//tools/build_rules/rust:darwin": ["_" + name + "-darwin"],
                }),
            tags = ["manual"],
            )

# BEWARE: cargo will report a test as passing if there is no matching
#         test name
def cargo_test(name, size, loc, test_name, srcs=[], cargo_opts=""):
    native.sh_test(
        name=name,
        size=size,
        srcs=["//tools/build_rules/rust:run_cargo.sh"],
        data=srcs + native.glob(_automatic_srcs),
        args=[loc, "\"test " + cargo_opts + " " + test_name + "\""],
        tags=["local", "manual"],
    )
