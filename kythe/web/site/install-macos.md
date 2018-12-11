---
layout: page
title: Getting Started on macOS
permalink: /getting-started-macos/
---

* toc
{:toc}

This document extends the [Getting Started guide](getting-started.md) to cover
installation of external dependencies on a macOS system. You will still need to
follow all the other Getting Started instructions to get a working Kythe build.


## Installing Developer Tools

**Xcode**: To build on macOS, you will need Xcode installed. You can do this
from the terminal by running:

{% highligh bash %}
softwareupdate --install Xcode
{% endhighlight %}

or by launching the App Store application and browsing to Develop → Xcode.

**Homebrew**: You will also need to install [Homebrew](https://brew.sh) (follow
the link for instructions).  There are other ways to install packages, but the
rest of these instructions assume you have it.


## Installing External Dependencies

To install most of the [external dependencies][ext], run

{% highlight bash %}
for pkg in asciidoc cmake go graphviz node parallel source-highlight ; do
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

Once you have finished setting up your macOS installation, you can verify that
everything works by running:

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

[ext]: https://kythe.io/getting-started/#external-dependencies
[dock]: https://store.docker.com/editions/community/docker-ce-desktop-mac




