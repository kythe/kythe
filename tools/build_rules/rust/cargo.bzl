def _cargo_cmd(loc, cmd, outs=[]):
     return "\n".join([
             "$(location //tools/build_rules/rust:run_cargo.sh) " + loc + " \"" + cmd + "\"",
        ] + [
            "mv " + loc + out + " $(location " + out + ") " for out in outs 
        ])

_automatic_srcs = [
    "Cargo.toml",
    "src/**/*.rs",
]

def cargo_bin(name, loc, srcs=[], bin_name=None, release=False):
    bin_name = name if bin_name == None else bin_name
    output_path = "target/" + ("release/" if release else "debug/") + bin_name
    native.genrule(
        name = name,
        srcs = srcs + native.glob(_automatic_srcs),
        outs = [output_path],
        cmd = _cargo_cmd(
          loc = loc,
          cmd = "build --bin " + (" --release" if release else "") + bin_name,
          outs = [output_path],
        ),
        local = 1,
        tools = ["//tools/build_rules/rust:run_cargo.sh"],
        tags = ["manual", "requires-network"],
    )

def cargo_lib(name, loc, srcs=[], libname=None, release=False):
    libname = (name if libname == None else libname) + ".rlib"
    output_path = "target/" + ("release/" if release else "debug/") + libname
    native.genrule(
        name = name,
        srcs = srcs + native.glob(_automatic_srcs),

        outs = [output_path],
        cmd = _cargo_cmd(
          loc = loc,
          cmd = "build --lib" + (" --release" if release else ""),
          outs = [output_path],
        ),
        local = 1,
        tools = ["//tools/build_rules/rust:run_cargo.sh"],
        tags = ["manual", "requires-network"],
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
        tags=["local"],
    )
