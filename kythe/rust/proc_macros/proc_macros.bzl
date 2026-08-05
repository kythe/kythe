"""Provides proc_macro_bundle(), which collects dylibs needed to parse code that uses macros."""

def proc_macro_bundle(name, deps):
    """Creates a Fileset containing proc macro dylibs and a manifest listing them.

    The `:proc_macros` library can load the dylibs at runtime and connect them
    to the rust-analyzer frontend, allowing tools to understand code that uses
    proc macros.

    This bundle specifies a static set of macros chosen at build time. Code may
    of course use other proc macros, and they will not be understood.

    The keys in the manifest are the full labels of the rust_proc_macro targets.

    Args:
      deps: List[label]. The `rust_proc_macro` targets that should be included.
            These have names like "foo_proc_macro_internal".
            (The "foo" target is a _rust_proc_macro_wrapper, which is the
            proc-macro built in exec config).
    """

    # We don't treat the deps as opaque labels, but chop up the strings to form
    # the fileset and corresponding manifest. Let's check them first...
    for dep in deps:
        if not dep.startswith("//"):
            fail("need an absolute label:", dep)
        if dep.count(":") != 1:
            fail("need a label in canonical //package:target form:", dep)
        if not dep.endswith("_proc_macro_internal"):
            fail("need the underlying rust_proc_macro, not a wrapper:", dep)

    # The manifest lists each dylib as: //build:target=filename
    native.genrule(
        name = name + "_manifest",
        outs = [name + "_manifest/manifest"],
        cmd = "\n".join(["echo '%s=%s' >> $@" % (
            target,
            target.split(":")[1],
        ) for target in deps]),
    )

    # A directory containing all the macro dylibs, and a manifest.
    native.Fileset(
        name = name,
        entries = [
            native.FilesetEntry(files = [name + "_manifest/manifest"], strip_prefix = name + "_manifest"),
        ] + [
            native.FilesetEntry(
                srcdir = target.split(":")[0] + ":BUILD",
                files = [target.split(":")[1]],
            )
            for target in deps
        ],
    )
