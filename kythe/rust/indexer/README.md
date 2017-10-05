# kythe-indexer

`kythe-indexer` is an **EXPERIMENTAL** plugin for rustc that utilizes Google's
[Kythe](https://kythe.io) pipeline for working with source code.

`kythe-indexer` currently supports:

-   cross-references
-   call graphs

**NOTE: AS A PLUGIN `kythe-indexer` REQUIRES NIGHTLY RUST**

## Usage

There are [many ways](https://doc.rust-lang.org/book/compiler-plugins.html) to
load the `kythe-indexer` plugin, but the least obtrusive is as a command line
flag like so:

```
$ rustc -Zextra-plugins=kythe_indexer
```

If you are using a build tool rather than the `rustc` command directly:

```
$ RUST_FLAGS="-Zextra-plugins=kythe_indexer" {my_build_cmd}
```

The plugin will output kythe entries in JSON form to stdout.
