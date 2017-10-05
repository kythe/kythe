## Maven extractor

An extractor that builds index files from a maven-based project. It uses [javac
extractor](../standalone/README.md) as compiler for maven. There is no special
maven-related code and maven extractor is just a way to use javac extractor.

#### Usage

These instructions assume kythe is installed in /opt/kythe. If not, follow the
[installation](http://kythe.io/getting-started) instructions.

```shell
cd some/maven/project

# setup extractor
export REAL_JAVAC="$(which javac)"
export KYTHE_ROOT_DIRECTORY="$PWD"
export KYTHE_OUTPUT_DIRECTORY="/tmp/kythe"
mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

# extract
mvn clean compile \
  -Dmaven.compiler.forceJavacCompilerUse=true \
  -Dmaven.compiler.fork=true \
  -Dmaven.compiler.executable=/opt/kythe/extractors/javac-wrapper.sh
```

kindex files will be generated in `/tmp/kythe` folder.

#### Development

See development instructions for javac extractor.
