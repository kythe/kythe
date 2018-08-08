# Kythe Extractor Wrapping

This package contains a binary and supporting libraries for doing extraction
on repos using Kythe.  It is intended as the last step where extraction is
actually performed by invoking the builds, along with any immediate
preprocessing required.

## Build System Extractors

We support Kythe extraction on a few different build systems.

### Gradle

In order to run Kythe extraction on a gradle repo, we must first modify the
`gradle.build` file to hook into a separate javac wrapper binary.
`gradle_build_modifier.go` takes an input `gradle.build` file and appends the
bits necessary for replacing javac calls with Kythe's `javac-wrapper.sh`.

```
allprojects {
  gradle.projectsEvaluated {
    tasks.withType(JavaCompile) {
      options.fork = true
      options.forkOptions.executable = '/opt/kythe/extractors/javac-wrapper.sh'
    }
  }
}
```

If the input file already contains reference to
`options.forkOptions.executable`, then `gradle_build_modifier.go` does nothing.

#### Future work

The current implementation uses simple string-based matching, without actually
understanding the structure.  If that becomes necessary in the future, it might
be better to use the existing Java libraries for `org.codehaus.groovy.ast` to
properly parse the build.gradle file and have more precise picture.  In
particular `org.codehaus.groovy.ast.CodeVisitorSupport` might be sufficient.

### Maven

Maven is handled in Java by
`com.google.devtools.kythe.platform.tools.MvnPomPreprocessor`.  It utilizes an
xml library to parse and modify the mvn `pom.xml` config file in a similar way
as described above for gradle.

#### Future work

In theory if we can find a nice xml library for golang that supports reflecting
into specific elements and modifying without knowing the whole file structure,
then we could do away with the Java binary for maven preprocessing.

### CMake

Coming soon™.  https://github.com/google/kythe/issues/2861

### Bazel

Actually we have no custom work here.  We extract compilation records from Bazel
using the extra action mechanism.  The extractrepo tool therefore doesn't handle
Bazel directly, but repositories using Bazel for languages we already support
should work without extra effort.

## RunExtractor

In addition to the custom preprocessing logic for Kythe Extraction on different
build systems, we also have a wrapper binary.  `runextractor` is a simple
executible that is used as the inner-most entrypoint for Kythe Extraction.  It
is derived from `kythe/extractors/java/maven/mvn-extract.sh`.

Use:

```
./runner \
  --builder=MAVEN \
  --mvn_pom_preprocessor=/opt/kythe/extractors/javac_extractor.jar \
  --javac_wrapper=/opt/kythe/extractors/javac-wrapper.sh
```

