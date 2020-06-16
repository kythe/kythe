# Rust Hello World

This is an example Rust project that is built using Bazel's [Rust rules](https://github.com/bazelbuild/rules_rust).  

Dependencies are managed using [cargo-raze](https://github.com/google/cargo-raze) in [Remote Dependency Mode](https://github.com/google/cargo-raze#remote-dependency-mode).  

## Building using Bazel
The binary can be built using the `:hello-world` target in this package.
