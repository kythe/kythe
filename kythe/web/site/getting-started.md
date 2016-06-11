---
layout: page
title: Getting Started
permalink: /getting-started/
---

* toc
{:toc}

## Get the Kythe source code

{% highlight bash %}
git clone https://github.com/google/kythe.git
{% endhighlight %}

### External Dependencies

Kythe relies on the following external dependencies:

* go >=1.6
* clang-3.5
* cmake
* node.js
* asciidoc
* jdk >=8
* parallel
* source-highlight
* graphviz
* libncurses-dev
* libcurl4-openssl-dev
* uuid-dev
* libssl-dev
* bison-3.0.2 (2.3 is also acceptable)
* flex-2.5
* libmemcached-dev
* [docker](https://www.docker.com/) (for release images `//kythe/release/...` and `//buildtools/docker`)
* [leiningen](http://leiningen.org/) (used to build `kythe/web/ui`)

You will need to ensure they exist using your favorite method (apt-get, brew,
etc.).

#### Installing Debian Jessie Packages

{% highlight bash %}
echo deb http://http.debian.net/debian jessie-backports main >> /etc/apt/sources.list
apt-get update

apt-get install \
    asciidoc source-highlight graphviz \
    gcc libssl-dev uuid-dev libncurses-dev libcurl4-openssl-dev flex clang-3.5 bison \
    openjdk-8-jdk \
    parallel libmemcached-dev

# https://golang.org/dl/ for Golang installation
# https://docs.docker.com/installation/debian/#debian-jessie-80-64-bit for Docker installation
{% endhighlight %}

### Internal Dependencies

All other Kythe dependencies are hosted within the repository under
`//third_party/...`. Run the `./tools/modules/update.sh` script to update these
dependencies to the exact revision that we test against.

This step may take a little time the first time it is run and should be quick
on subsequent runs.

#### Installing and keeping LLVM up to date

When building Kythe, we assume that you have an LLVM checkout in
`third_party/llvm/llvm`.  If you don't have an LLVM checkout in that directory, or
if you fall out of date, the `./tools/modules/update.sh` script will update you
to the exact revisions that we test against.

Note that you don't need to have a checkout of LLVM per Kythe checkout.  It's
enough to have a symlink of the `third_party/llvm/llvm` directory.

## Building Kythe

### Building using Bazel

Kythe uses [Bazel](http://bazel.io) to build its source code.  After
[installing Bazel](http://bazel.io/docs/install.html) and all external
dependencies, building Kythe should be as simple as:

{% highlight bash %}
./tools/modules/update.sh  # Ensure third_party is updated

bazel build //... # Build all Kythe sources
bazel test  //... # Run all Kythe tests
{% endhighlight %}

### Build a release of Kythe using Bazel and unpack it in /opt/kythe

Many examples on the site assume you have installed kythe in /opt/kythe.

{% highlight bash %}
# Your current Kythe version
export KYTHE_RELEASE="0.0.21"
# Build a Kythe release
bazel build //kythe/release
# Extract our new Kythe release to /opt/ including its version number
tar -zxf bazel-genfiles/kythe/release/kythe-v${KYTHE_RELEASE}.tar.gz /opt/
# Remove the old pointer to Kythe if we had one
rm -f /opt/kythe 
# Point Kythe to our new version
ln -s /opt/kythe-v${KYTHE_RELEASE} /opt/kythe
{% endhighlight %}

### Using the Go tool to build Go sources directly

Kythe's Go sources can be directly built with the `go` tool as well as with
Bazel.

{% highlight bash %}
# Install LevelDB/snappy libraries for https://github.com/jmhodges/levigo
sudo apt-get install libleveldb-dev libsnappy-dev

# With an appropriate GOPATH setup
go get kythe.io/kythe/...

# Using the vendored versions of the needed third_party Go libraries
git clone https://github.com/google/kythe.git
GOPATH=$GOPATH:$PWD/kythe/third_party/go go get kythe.io/kythe/...
{% endhighlight %}

The additional benefits of using Bazel are the built-in support for generating
the Go protobuf code in `kythe/proto/` and the automatic usage of the checked-in
`third_party/go` libraries (instead of adding to your `GOPATH`).  However, for
quick access to Kythe's Go sources (which implement most of Kythe's platform and
language-agnostic services), using the Go tool is very convenient.

## Updating and building the website

* Make change in ./kythe/web/site
* Spell check
* Build a local version to verify fixes

Prerequisites:
{% highlight bash %}
gem install bundler
{% endhighlight %}

Build and serve:
{% highlight bash %}
cd ./kythe/web/site
./build.sh
# Serve website locally on port 4000
bundle exec jekyll serve
{% endhighlight %}
