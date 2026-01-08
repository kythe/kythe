# Hermetic, ephemeral, reproducible Kythe

[![Test status](https://github.com/filmil/kythe/workflows/Test/badge.svg)](https://github.com/filmil/kythe/workflows/Test/badge.svg)

**This is aspirational. I'm almost there but not quite just yet.**

This file documents an effort to make the existing kythe fully [hermetic,
ephemeral and reproducible][her]. If you wonder how this is even possible,
see [this article][aa].

[aa]: https://hdlfactory.com/post/2024/04/20/nix-bazel-%EF%B8%8F/
[her]: https://hdlfactory.com/note/2024/05/01/hermetic-ephemeral-reproducible-builds-her/

## The problem

Building Kythe is hard. It should not be hard.  It should be as easy as issuing
a one-liner command. 

## How is building Kythe hard?

Building Kythe requires installing quite a few
non-hermetic build prerequisites.  You can [check for yourself][yy]. In this
day and age, this usually means throwing away quality time into figuring out
a tangled web of inter-dependencies that sometimes conspire to make each
other not work well.

[yy]: https://kythe.io/contributing/

System-wide installation is annoying, 
because it adds programs to your machine that you might not want installed.
It may pollute your dev environment with conflicting versions of programs 
and libraries. It is annoying if you want to move your dev environment between machines.  If you dismantle your dev environment, the installed programs
usually stay to linger around. Not ideal.

I believe that you should be able to have your dev environment set up for you
automatically. This is why I [bother with bazel][bb], and why I made
[a number of tools that make bazel work for you][tt].

[bb]: https://hdlfactory.com/post/2024/04/27/why-do-i-bother-with-bazel/
[tt]: https://hdlfactory.com/tags/bazel/

## Why is Kythe important?

Kythe is an important tool for understanding large code bases. Its use has
remained niche because building and using it is involved. It is one of the rare
open source tools built for this purpose.

I have a notion that this might change if building and running it would be
made easier. This is what I am trying to do in this fork.

## How to build

### One time setup

* [Install bazel via `bazelisk`][zz]. This is the easiest way to get
 [`bazel`][dd] if you don't already have it.

* Clone this repository.

[zz]: https://hdlfactory.com/note/2024/08/24/bazel-installation-via-the-bazelisk-method/
[dd]: https://bazel.build

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

If you want to build self-contained binaries, you can use a closure
computer such as https://github.com/tweag/clodl. It will find all the
libraries your binary uses and package them all together into a self-extracting
archive. While this is a quick way to get somewhat portable binaries, if your
needs are more elaborate, you may need to find a different solution, or make your own.
