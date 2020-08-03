# Fuchsia Rust compilation extractor

This directory contains a Rust compilation extractor for the [Fuchsia OS][fx].
Fuchsia build system is a bit specific, and is custom built on top of [GN][gn].
At the moment the approach is not generalized to all gn builds though
potentially it could be with some additional GN configuration.

[fx]: https://www.fuchsia.dev
[gn]: https://gn.googlesource.com/gn/

Technically, this extractor produces `kzip` files as any other extractor does,
and the `kzip` file specification is well known.  However, in practice, the
kzip specification is open for interpretation (for example, what if you need
to inject a specific extra file to a compilation unit?).  That in turn can
lead to mismatches in the interpretations that would break the ingestion
pipeline.

This document is an attempt to lay out any such assumptions for
`fuchsia_extractor` going forward.

# Specifics of Fuchsia compilation

Fuchsia is compiled with [GN][gn].  However, due to the size of the code base,
the GN generator scripts are heavily customized and templatized in a specific
way, so it could be said that Fuchsia uses a custom purpose-built build system.

Fuchsia source code is spread across many repositories, some of which can be
seen at https://fuchsia.googlesource.com.  When the source code is checked out,
a custom utility `fx jiri` is used (which is to say, a custom wrapper over the
`jiri` program) to check out all the repositories into its appropriate place
underneath a common source root directory, which we will call `$FUCHSIA_DIR`
after the environment variable that is usually used to refer to it in scripts.

Fuchsia code is compiled in a separate "output" directory.  However, this
directory is created underneath `$FUCHSIA_DIR`, and each build configuration,
for a "product-dot-board" combination such as `terminal.x64`, gets a separate
build output directory.  The build configuration `terminal.x64` gets an output
directory in `$FUCHSIA_DIR/out/terminal.x64`, though this last path component
can be given a different name by the developer.  Let us call this directory
`$FUCHSIA_OUT` for short.

Multiple code corpora exist in `$FUCHSIA_DIR` and `$FUCHSIA_OUT`.  There is C
and C++ code, Rust code, go code, test fixtures, dart and flutter code, and
dedicated IDL generated code for languages called FIDL and Banjo.

When filling out compilation unit data, we need to ensure that when we refer to
files, their names are expressed relative to some of those directories, and
then to correct ones.  Furthermore, sometimes relative paths are needed with
respect to the corpus root, and sometimes with respect to the build directory.
For example, generated code lives in `$FUCHSIA_OUT/gen`.  Object code, such as
`.o` files, live in `$FUCHSIA_OUT/obj`, and there are about a dozen of such
different corpus roots.

Even further, though, even as the relationships between these directories are fixed,
infrastructure builds may take liberties with the arrangement of these directories.
This means that we need to keep options open to retarget these directories
as the developer sees fit for their use.  Hence the many flags.

# Flags

;`--basedir BASE_DIR`: The directory in which compilation is ran, i.e.
`$FUCHSIA_OUT`.  If unspecified, the default is "current directory".

;`--inputdir INPUT_DIR`: The directory that contains the `save-analysis` files.
The compilation indexing is fully driven by these files, so the program will
need to know where to find them.  In Fuchsia, they are all bunched up in a
single flat directory, which is normally at `$FUCHSIA_OUT/save-analysis-temp`
(with limited options to rename the directory).  Required.

;`--inputfiles FILES`: A comma-separated list of specific `save-analysis` files
to read, in case the `--inputdir` flag is too broad.  If this is specified together
with `--inputdir`, the

;`--outputdir OUTPUT_DIR`: The directory that will contain the resulting `kzip`
files.  Note that `fuchsia_extractor` will refuse to overwrite any existing
`kzip` files.  Required.

;`--corpus CORPUS`: The corpus name to use.  If defined, the `CORPUS` value is
used verbatim.  Otherwise, if specified, the value from the environment variable
`KYTHE_CORPUS` is used.  Otherwise, the fallback is `fuchsia`.

;`--language LANGUAGE`: Used verbatim, if not specified.  Otherwise `rust` is
the default value.

;`--revisions REVISIONS`: A comma-separated set of revision markers, for example
commit IDs, for which this index is current.  Since the extractor has no way
of knowing this information, the program expects to be supplied these values.

# Example invocation

The script below shows a sample invocation of the fuchsia extractor, ran from
the Kythe source code tree, using `bazel`.  In production use, one would
instead normally invoke a regular released binary.

```bash
#! /bin/bash
set -x

if [[ $FUCHSIA_DIR == "" ]]; then
  echo "No FUCHSIA_DIR defined"
  exit 1
fi

FUCHSIA_BUILD_DIR="${FUCHSIA_DIR}/out/terminal.x64"

readonly _output_dir="${HOME}/tmp/fuchsia-extractor-experiment"
readonly _build_dir="${FUCHSIA_BUILD_DIR:-$(fx get-build-dir)}"

mkdir -p "${_output_dir}"

bazel run //kythe/rust/fuchsia_extractor:fuchsia_extractor -- \
  --basedir="${FUCHSIA_BUILD_DIR}" \
  --inputdir="${FUCHSIA_BUILD_DIR}/save-analysis-temp" \
  --output="${_output_dir}" \
  --corpus="fuchsia" \
  --language="rust" \
  "${@}"
```

The processing is sequential, and therefore fairly slow.  Expect on the order
of 10 seconds to generate a single `kzip` for a single compilation unit. This
sequential processing is not a fundamental limitation, but will be addressed
only if needed. The infrastructure may.

The script will sparingly output the progress of the conversion.

# Extraction approach

The extraction is a simple linear process: we read and parse each
`save-analysis` JSON file using the compiler-specific data model.  This
information is then converted into compilation unit taking care of the following:

* Every relevant source file mentioned in the analysis is stored in the `kzip`.
* Every relevant `rlib` object file is stored in the `kzip`.
* Care is taken that the source paths and paths relative to the compilation
  and corpus source roots are filled out correctly.
* The resulting output is saved in a single output directory.

# Testing approach

Every module in `fuchsia_extractor` is unit tested.  Run the respective
tests from command line with `bazel` like so:

```bash
bazel test //kythe/rust/fuchsia_extractor/...
```

# The content of the produced `kzip` files

The kzip files produced by the fuchsia extractor contain a single compilation
unit per zip file.  The `save-analysis` JSON file is injected into the list of
required inputs into each compilation unit.

Other than that, the packaged files are:

- `*.rs` files, which are in the same directory as the main crate and its
  subdirectories.
- `*.rlib` files which are the required part of the compilation, even though
  the current indexer does not use them.

