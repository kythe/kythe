---
layout: page
title: Contributing
permalink: /contributing/
---

* toc
{:toc}

## Building Source Code

Kythe uses [Bazel](http://bazel.io) to build its source code.  After
[installing Bazel](http://bazel.io/docs/install.html), building Kythe should be
as simple as:

{% highlight bash %}
./setup_bazel.sh           # Run initial Kythe+Bazel setup
./tools/modules/update.sh  # Ensure third_party is updated

bazel build //... # Build all Kythe sources
bazel test  //... # Run all Kythe tests
{% endhighlight %}

### Dependencies

Kythe relies on the following external dependencies:

* go >=1.3
* clang-3.5
* jdk >=7 (Bazel requires >=8)
* parallel
* asciidoc
* source-highlight
* graphviz
* libncurses-dev
* libcurl4-openssl-dev
* uuid-dev
* libssl-dev
* bison-3.0.2 (2.3 is also acceptable)
* flex-2.5
* [docker](https://www.docker.com/) (for release images `//kythe/release/...` and `//buildtools/docker`)
* [leiningen](http://leiningen.org/) (used to build `kythe/web/ui`)

NOTE: All other Kythe dependencies are hosted within the repository under
`//third_party/...`

#### Installing Debian Jessie Packages

{% highlight bash %}
echo deb http://http.debian.net/debian jessie-backports main >> /etc/apt/sources.list
apt-get update

apt-get install \
    asciidoc source-highlight graphviz \
    libssl-dev uuid-dev libncurses-dev libcurl4-openssl-dev flex clang-3.5 bison \
    openjdk-8-jdk \
    golang-go gcc \
    parallel

# https://docs.docker.com/installation/debian/#debian-jessie-80-64-bit for Docker installation
{% endhighlight %}

### Installing and keeping LLVM up to date

When building Kythe, we assume that you have an LLVM checkout in
third_party/llvm/llvm.  If you don't have an LLVM checkout in that directory, or
if you fall out of date, the `./tools/modules/update.sh` script will update you
to the exact revisions that we test against.  `./setup_bazel.sh` should be run
before updating the modules.

Note that you don't need to have a checkout of LLVM per Kythe checkout.  It's
enough to have a symlink of the third_party/llvm/llvm directory.

### Using the Go tool

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

## Code review

All changes to Kythe must go through code review before being submitted, and
each individual or corporate contributor must sign an appropriate [Contributor
License Agreement](https://cla.developers.google.com/about).  Once your CLA is
submitted (or if you already submitted one for another Google project), make a
commit adding yourself to the
[AUTHORS]({{site.data.development.source_browser}}/AUTHORS) and
[CONTRIBUTORS]({{site.data.development.source_browser}}/CONTRIBUTORS)
files. This commit can be part of your first Differential code review.

The Kythe team has chosen to use a [Phabricator](http://phabricator.org/)
instance located at
[{{site.url}}/phabricator]({{site.data.development.phabricator}})
for code reviews.  This requires some local setup for each developer:

### Installing arcanist

{% highlight bash %}
ARC_PATH=~/apps/arc # path to install arcanist/libphutil

sudo apt-get install php5 php5-curl
mkdir -p "$ARC_PATH"
pushd "$ARC_PATH"
git clone https://github.com/phacility/libphutil.git
git clone https://github.com/phacility/arcanist.git
popd

# add arc to your PATH
echo "export PATH=\"${ARC_PATH}/arcanist/bin:\$PATH\"" >> ~/.bashrc
source ~/.bashrc

arc install-certificate # in Kythe repository root
{% endhighlight %}

### Using arcanist

{% highlight bash %}
git checkout master
arc feature feature-name # OR git checkout -b feature-name
# do some changes
git add ...                    # add the changes
git commit -m "Commit message" # commit the changes
arc diff --browse              # send the commit for review
# go through code review in Phabricator UI...
# get change accepted

arc land                       # merge the change into master
{% endhighlight %}

For core contributors with write access to the Kythe repository, `arc land` will
merge the change into master and push it to Github.  Others should request that
someone else land their change for them once the change has been reviewed and
accepted.

{% highlight bash %}
# Land a reviewed change
arc patch D1234
arc land
{% endhighlight %}

## Contribution Ideas

Along with working off of our [tasks
list]({{site.data.development.phabricator}}/maniphest) (and, in particular, the
[Wishlist]({{site.data.development.phabricator}}/maniphest/query/uFWarCNL9v7z/)),
there are many ways to contribute to Kythe.

### New Extractors and Indexers

Kythe is built on the idea of having a common set of tools across programming
languages so Kythe is always happy to
[add another language to its family]({{site.baseurl}}/docs/kythe-compatible-compilers.html).

### Build System Integration

In order to use Kythe's compilation extractors, they must be given precise
information about how a compilation is processed.  Currently, Kythe has
[built-in support]({{site.data.development.source_browser}}/kythe/extractors/bazel/extract.sh)
for Bazel and rudimentary support for
[CMake]({{site.data.development.source_browser}}/kythe/extractors/cmake/).
Contributing support for more build systems like [Gradle](https://gradle.org)
will greatly help the ease of use for Kythe and increase the breadth of what it
can index.

### User Interfaces

Kythe emits a lot of data and there are *many* ways to interpret/display it all.
Kythe has provided a
[sample UI]({{site.baseuri}}/examples#visualizing-cross-references), but it
currently only scratches the surface of Kythe's data.  Other ideas for
visualizers include an interactive graph, a documentation browser and a source file
hierarchical overview.
