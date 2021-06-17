# Kythe Extractor Wrapping

This package contains a binary and supporting libraries for doing extraction
on repos using Kythe.  It is intended as the last step where extraction is
actually performed by invoking the builds, along with any immediate
preprocessing required.

## RunExtractor

`runextractor` is an executible that is used as the inner-most entrypoint for
Kythe Extraction.  This binary is intended to encapsulate any logic required for
extracting that is common to the build-system.  So for example any configuration
that is on a per-repo basis should be handled upstream, not in this binary.

`runextractor` is expected to be run from the root of a repository, so that
any access to config files (`build.gradle`, `pom.xml`, etc) is sensible, and
also so that execution of build/compile commands works.

Use:

```
./runner \
  --builder=MAVEN \
  --mvn_pom_preprocessor=/opt/kythe/extractors/javac_extractor.jar \
  --javac_wrapper=/opt/kythe/extractors/javac-wrapper.sh
```

### Environment Variables

Because `runextractor` is invoked in a manner not conducive to cleanly passing
commandline flags, there is non-trivial setup done with environment variables.
When calling `runextractor`, here are the required environment variables:

* **KYTHE_ROOT_DIRECTORY**: The absolute path for file input to be extracted.
* **KYTHE_OUTPUT_DIRECTORY**: The absolute path for storing output.
* **KYTHE_CORPUS**: The corpus label for extracted files.

For Java there are other required env vars:
* **JAVAC_EXTRACTOR_JAR**: A absolute path to a jar file containing the java
  extractor.
* **REAL_JAVAC**: A path to a "normal" javac binary (not a wrapped binary).

TODO(#156): If we can get rid of docker in docker, working dir relative paths
  might be easier to work with.

## Build System Extractors

We support Kythe extraction on a few different build systems.

### Gradle

In order to run Kythe extraction on a gradle repo, we must first modify the
`build.gradle` file to hook into a separate javac wrapper binary.
`gradle_build_modifier.go` takes an input `build.gradle` file and appends the
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

Maven's build config is handled in Java by `mavencmd/pom_xml_modifier.go`.  It
utilizes the etree xml library to parse and modify the mvn `pom.xml` config file
in a similar way as described above for gradle.  One notable difference between
gradle and maven here is that gradle actually embeds the refrence to the javac
wrapper directly into the build file, while the modifications to the maven pom
xml file merely allow future configuration at runtime.

### CMake

CMake repositories are extracted from `compile_commands.json` after building
the repository with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`. It then invokes the
`cxx_extractor` binary as if it were a compiler for each of those commands.

### Bazel

Actually we have no custom work here.  We extract compilation records from Bazel
using the extra action mechanism.  The extractrepo tool therefore doesn't handle
Bazel directly, but repositories using Bazel for languages we already support
should work without extra effort.
