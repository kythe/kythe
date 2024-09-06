# Hermetic, ephemeral, reproducible Kythe

[![Test status](https://github.com/filmil/kythe/workflows/Test/badge.svg)](https://github.com/filmil/kythe/workflows/Test/badge.svg)

**This is aspirational. I'm almost there but not quite just yet.**

This file documents an effort to make the existing kythe fully [hermetic,
ephemeral and reproducible][her].

[her]: https://hdlfactory.com/note/2024/05/01/hermetic-ephemeral-reproducible-builds/

If you are intersted how this is even possible, see [this article][aa].

[aa]: https://hdlfactory.com/post/2024/04/20/nix-bazel-%EF%B8%8F/

## The problem

Building Kythe is hard. It should not be hard.  It should be as easy as issuing
a one-liner command. 

## How is building Kythe hard?

Building Kythe requires installing quite a few
non-hermetic build prerequisites.  You can [check for yourself][yy].

[yy]: https://kythe.io/contributing/

This is annoying because it adds programs you might not need onto your machine.
It is annoying if you want to move your dev environment between machines.

I also believe that you should be able to have your dev environment set up for you
automatically.

This is why I [bother with bazel][bb].

[bb]: https://hdlfactory.com/post/2024/04/27/why-do-i-bother-with-bazel/

## Why is Kythe important?

Kythe is an important tool for understanding large code bases. Its use has
remained niche because building and using it is involved. It is one of the rare
open source tools built for this purpose.

I have a notion that this might change if building and running it would be
made easier. This is what I am trying to do in this fork.

## How to build

### One time setup

* Install bazel via `bazelisk`. Install it under the name `bazel` into your `$PATH`.

* Clone this repository.

### Build and test

* From the root directory of the cloned repository, type:

  ```
  bazel test --config=nix //... \
              -- -//kythe/cxx/extractor/testdata/... -//kythe/cxx/indexer/cxx/testdata/... 
  ```

  We currently exclude the extractor and indexer testdata because of a silly
  bug: https://github.com/NixOS/nixpkgs/issues/150655

* Wait for the build to finish. It might take a long time.

* That should be it.

## Caveats

The binaries built in a nix shell refer to shared libraries and other
nix artifacts by their path in `/nix/store`. This makes such binaries
not directoy runnable outside a nix shell. They are runnable from within
a bazel context here, e.g. when using `bazel run`. 

If you want to build self-contained binaries, one can use a closure
computer such as https://github.com/tweag/clodl.
