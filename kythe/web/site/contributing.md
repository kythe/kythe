---
layout: page
title: Contributing
permalink: /contributing/
---

* toc
{:toc}

## Getting started with the Kythe codebase

[Instructions to build Kythe from source]({{site.baseuri}}/getting-started)

## Code review

All changes to Kythe must go through code review before being submitted, and
each individual or corporate contributor must sign an appropriate [Contributor
License Agreement](https://cla.developers.google.com/about).  Once your CLA is
submitted (or if you already submitted one for another Google project), make a
commit adding yourself to the
[AUTHORS]({{site.data.development.source_browser}}/AUTHORS) and
[CONTRIBUTORS]({{site.data.development.source_browser}}/CONTRIBUTORS)
files. This commit can be part of your first Pull Request.

The Kythe team has chosen to use GitHub [Pull Requests](https://guides.github.com/activities/forking/)
for code review.

## Linting and Testing

Run `./tools/git/setup_hooks.sh` to initialize the standard Git hooks used by
the Kythe developers.  This ensures that each commit passes all unit/lint tests
and that commit descriptions follow the [Conventional
Commits](https://www.conventionalcommits.org/en/v1.0.0-beta.2/) specification
*before* a requesting code review.  If necessary, use `git commit --no-verify`
to bypass these checks.  In addition to the tools necessary to build Kythe, the
following are necessary to run the installed hooks:

- pre-commit
- buildifier
- clang-format (>= 11)
- gofmt
- golint
- google-java-format
- jq
- shellcheck

Each Pull Request will also require a gambit of tests run Google Cloud Build.
All non-maintainers will require a Kythe team member to add a `/gcbrun` comment
in each Pull Request to verify the PR should be checked by presubmits.

### Style formatting

Kythe C++ code follows the Google style guide. You can run `clang-format` to do
this automatically:

{% highlight bash %}
clang-format -i --style=file <filename>
{% endhighlight %}

If you forgot to do this for a commit, you can amend it easily:

{% highlight bash %}
clang-format -i --style=file $(git show --pretty="" --name-only <SHA1>)
git commit --amend
{% endhighlight %}

Likewise, all Java code should be formatted by the latest version of
`google-java-format` and Go code should pass through `gofmt`.

## Contribution Ideas

Along with working off of our [tasks
list]({{site.data.development.issue_tracker}}) (and, in particular, the
[Wishlist]({{site.data.development.github}}/labels/wishlist)),
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
