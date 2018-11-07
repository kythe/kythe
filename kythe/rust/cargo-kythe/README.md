# cargo-kythe

`cargo-kythe` is a `cargo` subcommand for managing [Kythe](https://kythe.io)
indices of cargo projects.

## Installation

To install `cargo-kythe` simply enter the following command:

```
cargo install cargo-kythe
```

`cargo-kythe` **requires nightly rust**

## Usage

`cargo-kythe` reads an environment variable `KYTHE_DIR` to determine the path to
the local installation of kythe. You will probably need to download it from
[kythe/kythe](https://github.com/kythe/kythe/releases)

To index the top level package:

```
cargo kythe index
```

To index the package and all its dependencies:

```
cargo kythe full-index
```

To launch the sample web UI for viewing cross-references:

```
cargo kythe web
```

Indices are constructed in the target/kythe directory.
