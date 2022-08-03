## Javac extractor

An extractor that wraps javac to extract compilation information and write it to
an index file.

#### Usage

These instructions assume kythe is installed in /opt/kythe. If not, follow the
[installation](http://kythe.io/getting-started) instructions.

```shell
cd some/folder/with/java/files

# setup extractor
export REAL_JAVAC="$(which javac)"
export KYTHE_ROOT_DIRECTORY="$PWD"
export KYTHE_OUTPUT_DIRECTORY="/tmp/kythe"
mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

# extract
/opt/kythe/extractors/javac-wrapper.sh Foo.java Bar.java
```

kzip file will be generated in `/tmp/kythe` folder.

#### Development

Building extractor from sources:

```shell
export KYTHE_PROJECT=path/to/kythe/repo/directory
cd $KYTHE_PROJECT
bazel build //kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor
```

Running freshly built extractor:

```shell
cd some/folder/with/java/files

# setup extractor
export REAL_JAVAC="$(which javac)"
export KYTHE_ROOT_DIRECTORY="$PWD"
export KYTHE_OUTPUT_DIRECTORY="/tmp/kythe"
export JAVAC_EXTRACTOR_JAR="$KYTHE_PROJECT/bazel-bin/kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac_extractor_deploy.jar"
mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

# extract
$KYTHE_PROJECT/kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac-wrapper.sh Foo.java Bar.java
```

kzip file will be generated in `/tmp/kythe` folder.
