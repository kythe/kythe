# Build Config Preprocessing

A number of builders require custom modification to a repo's build config before
Kythe extraction will work correctly.

## Preprocessor

This is the top level binary for doing preprocessing.  The default operation is
simply `./preprocess <build-file>`, which will detect what format the build file
is in and perform necessary steps.

## Invoking via docker / Cloud Build

Insert the following step into your Cloud Build to preprocess a `pom.xml` or
`build.gradle` file for use with Kythe extraction:

```
- name: 'gcr.io/kythe-public/build-preprocessor'
  args: ['/workspace/path/to/pom.xml']
```

If your repo is not copied to `/workspace/` but instead lives in another volume,
you will have to specify the volume in your build step:

```
- name: 'gcr.io/kythe-public/build-preprocessor'
  args: ['/other/volume/pom.xml']
  volumes:
    - name: 'volname'
      path: '/other/volume/'
```

## Specific Builders

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

If the input file already contains a reference to
`options.forkOptions.executable`, then `gradle_build_modifier.go` does nothing.

#### Future work

The current implementation uses simple string-based matching, without actually
understanding the structure.  If that becomes necessary in the future, it might
be better to use the existing Java libraries for `org.codehaus.groovy.ast` to
properly parse the build.gradle file and have more precise picture.  In
particular `org.codehaus.groovy.ast.CodeVisitorSupport` might be sufficient.
Ideally we find such a library in golang.

### Maven

Maven's build config is handled by `pom_xml_modifier.go`.  One notable
difference between gradle and maven here is that gradle actually embeds the
reference to the javac wrapper directly in the build file, while the
modifications to the maven pom xml file merely allow future configuration at
runtime.

The example snippet it drops in is:

```
<build>
  <plugins>
    <plugin>
      <groupId>org.apache.maven.plugins</groupId>
      <artifactId>maven-compiler-plugin</artifactId>
      <version>3.7.0</version>
      <configuration>
        <source>1.8</source>
        <target>1.8</target>
      </configuration>
    </plugin>
  </plugins>
</build>
```
