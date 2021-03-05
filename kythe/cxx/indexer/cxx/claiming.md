Avoiding redundancy during analysis
===================================

It is convenient to be able to define an analysis over a C++ codebase as
the composition of separate analyses over each translation unit in that
codebase. This may lead to redundant work being performed, especially when
(as is commonly the case) there is a large overlap in the set of headers that
each translation unit includes. Such extra work may result in a large blowup
in computation time or temporary storage requirements. In this document we
describe a technique that simplifies the problem of reducing redundant work.

At the coarsest level, we define (for some stable `GraphObserver` `NodeId`)
the operation `claim : (TranslationUnit * (File | NodeId)) -> bool`. If `claim`
is `true`, then the partial analysis done on the `TranslationUnit` is
responsible for analyzing the data in the `File` or with the `NodeId`.
If it is `false`, then the partial analysis is not responsible for doing work
for that `File` or `NodeId` (though it may do the work anyway).

== Static claiming

The simplest implementation of `claim` is to simply return `true` in all
circumstances. This is the most conservative possible estimate of
responsibility, but wastes the most amount of effort. For example, every
header file will be processed for every translation unit that includes it;
every template instantiation will be processed anew whenever a translation unit
causes it to be instantiated.

This idea may be refined by statically assigning responsibility for `NodeId` s
to translation units. Ideally this would result in each file being analyzed
exactly once per whole-project run, where each translation unit would be
responsible for roughly the same amount of code. Unfortunately, some header
files differ in behavior depending on their context (because they are included
inside `extern {}` or `namespace {}` blocks, for example). In full generality,
these files must be visited once per unique context.

LLVM already includes a tool (`modularize`) that can determine which files in a
collection of headers and source code are badly behaved in this sense. This
works roughly by recording all preprocessor effects seen during compilation
(as well as some effects that can only be checked in the AST, like the
aforementioned extern or unnamed namespace blocks), then checking to see whether
there are any conflicts in the records made for a given header file. For
example, if a particular #ifdef in some header foo.h is observed to resolve in
two different ways (modulo heuristics to admit header guards), that header is
non-hermetic.

These headers can be handled in the following way. A preprocess run on each
compilation task under analysis keeps track of a transcript of preprocessor
effects for each file the job causes to be included. Inclusion sites for
some a.h are labelled with the transcript for a.h _that will result from
that particular inclusion site_. This means that two inclusions of a.h that
behave differently will have different transcript labels. Because compilation
is deterministic, we know that the next time the compile is run for the task
we can expect to see the same effects everywhere. If we also save the
transcript for the main source file, then it is possible to provide the later
analysis phase with an oracle that knows exactly which effects an include will
have. Such a construction is made by building a table that maps from
(header-file * input-transcript * include) to output-transcript. (We can
reconstruct the destination header file because compilation is deterministic.)
When the analysis step reaches some include inside some header-file while
running in the context of some input-transcript, it can use this table to pick
the correct output-transcript.

Now it is left to us to partition all (header-file * transcript) pairs among
the translation units being analyzed. Every translation unit independently
knows which of these pairs it uses. We merely need to transpose the
table--change all (TU, header-pair) to (header-pair, TU)--and choose for each
header-pair a single TU among those that require it. We currently implement
this choice in a fairly arbitrary way, choosing for some header-pair the
associated TU with the least number of other header-pairs assigned to it.

We can use this mechanism to restrict the emission of preprocessor-specific
analysis data depending on whether the TU being analyzed claims the header
being preprocessed. We can also use this mechanism to restrict emission of
data for AST entities with physical source locations--if some declaration D
is located in a file for which a TU under analysis has a claim, that TU
should emit data for that declaration.

This mechanism means that one must be careful in an analysis to avoid dropping
data completely. For instance, if you are trying to connect up a declaration
with all of its definitions, each definition should be tested independently
for a claim rather than the declaration itself. This is because the declaration
may be in a header that is not claimed by the TU that contains some definition.

Static claiming still results in deterministic and independent analyses.
Unfortunately, certain features of C++ prevent us from being able to use
it in all contexts; its whole-program style assignment phase prevents us
from performing analysis in chunks. In the next section we introduce a second
mechanism that can work in concert with static claiming but can handle these
additional problems.

== Dynamic claiming

Compiling a C++ project may result in the generation of _implicit code_.
This code does not have any set location where it is spelled out in a
source file, but it is still important for analysis. It may be unreasonable
to make static assignments for implicit code: there can be many more implicitly
declared special member functions than includes, for example. For corpora
that are sufficiently large, it may be impossible to build and query the static
assignment table quickly.

To handle implicit code--and, for large codebases, to handle _all_ claiming,
provided that one can tolerate nondeterminism--we suggest a complimentary
approach to the static one described in the previous section.  In this approach,
called dynamic claiming, every analyzer has access to a shared "blackboard".
When an analyzer is about to begin analysis on some artifact, it queries the
blackboard to see if any other analyzer has done this work. If not, then the
querying analyzer becomes responsible for that artifact. The blackboard need not
keep permanent record of all work that has been posted. Since a completely
conservative implementation of `claim` is always admissible, it is fine for the
blackboard to be implemented as a fixed-size cache.
