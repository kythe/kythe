# Kythe Extracting openjdk11

This package describes extracting OpenJDK11 from source using Kythe

## Fetch and Configure OpenJDK11

Follow the instructions outline at
http://hg.openjdk.java.net/jdk/jdk11/raw-file/tip/doc/building.html
for configuring OpenJDK11 for your platform.

## Extraction

Once you have a correctly configured source tree for your platform,
run:

```
$ bazel run //kythe/extractors/openjdk11:extract -- /absolute/path/to/openjdk11
```

Assuming the source tree was configured correctly, the result will be a
collection of .kzip files (one per module) in ~/kythe-openjdk11-output
