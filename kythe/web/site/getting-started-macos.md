---
layout: page
title: Getting Started on macOS
permalink: /getting-started-macos/
---

* toc
{:toc}

This document extends the
[Getting Started guide]({{site.baseuri}}/getting-started) to cover
installation of external dependencies on a macOS system. You will still need to
follow all the other Getting Started instructions to get a working Kythe build.


## Installing Developer Tools

**Xcode**: To build on macOS, you will need Xcode installed. You can do this
from the terminal by running:

{% highlight bash %}
softwareupdate --install Xcode
{% endhighlight %}

or by launching the App Store application and browsing to Develop → Xcode.
Verify your installation by running the command:

{% highlight bash %}
$ xcodebuild -version
{% endhighlight %}

You should get something like this (though the numbers will vary):

{% highlight bash %}
Xcode 10.1
Build version 10B61
{% endhighlight %}

If you get an error like this:

{% highlight bash %}
xcode-select: error: tool 'xcodebuild' requires Xcode, but active developer directory '/Library/Developer/CommandLineTools' is a command line tools instance
{% endhighlight %}

then the following command should fix it:

{% highlight bash %}
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
{% endhighlight %}


**Homebrew**: You will also need to install [Homebrew](https://brew.sh) (follow
the link for instructions).  There are other ways to install packages, but the
rest of these instructions assume you have it.


## Installing External Dependencies

To install most of the [external dependencies][ext], run

{% highlight bash %}
for pkg in asciidoc bison brotli cmake go graphviz leveldb node parallel source-highlight wget ; do
   brew install $pkg
done

# Bazel.
# See also:  https://docs.bazel.build/versions/master/install-os-x.html
#
# DO NOT run "brew install bazel"; the version from core does not work.
# If you did so by mistake, run "brew uninstall bazel" first.
brew tap bazelbuild/tap
brew tap-pin bazelbuild/tap
brew install bazelbuild/tap/bazel

# Bison. The stock version is too old, but Bison is keg-only and Bazel uses
# a restricted PATH, so we need to tell Bazel where to find bison (see #3514):
export BISON=/usr/local/opt/bison/bin/bison

# Docker: See the instructions below.
# DO NOT use brew install docker (or if you did: brew uninstall docker).

# Optional installs:
brew install leiningen
brew install ninja
{% endhighlight %}

Note that some of the external packages listed are not mentioned here, because
macOS already has suitable versions pre-installed.

**Docker**: Download and install Docker for Mac from the [docker store][dock].
Note that you will have to create a "Docker ID" and log in to get access to the
download, but you won't to log in to the store to use Docker once it has been
installed.


## Verifying the Installation

Once you have finished setting up your macOS installation (including
[the parts that aren't macOS specific]({{site.baseuri}}/getting-started)),
you can verify that everything works by running:

{% highlight bash %}
cd $KYTHE_DIR
bazel clean --expunge
bazel test //kythe/...
{% endhighlight %}

If everything is set up correctly, all the tests should build and pass.  You
will probably see warnings from the C++ compiler, but there should not be any
errors or failing tests.

To verify that Docker works, run

{% highlight bash %}
docker run --rm -it alpine:latest echo "Hello, world."
{% endhighlight %}


## Updating Dependencies

For all the packages managed by Homebrew, you can update by running:

{% highlight bash %}
brew upgrade  # upgrade all installed packages
{% endhighlight %}

If you need to update Bazel in particular, run:

{% highlight bash %}
brew upgrade bazelbuild/tap/bazel
{% endhighlight %}

The Docker application automatically checks for updates (you can disable this
in the Preferences and select Check for Updates manually if you prefer).

[ext]: {{site.baseuri}}/getting-started#external-dependencies
[dock]: https://store.docker.com/editions/community/docker-ce-desktop-mac
