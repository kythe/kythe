# Kythe Extracting openjdk21

This package describes extracting OpenJDK21 from source using Kythe

## Fetch and Configure OpenJDK21

Follow the instructions outline at
https://github.com/openjdk/jdk21/blob/master/doc/building.md
for configuring OpenJDK21 for your platform.

## Extraction

Once you have a correctly configured source tree for your platform,
run:

```
$ bazel run //kythe/extractors/openjdk21:extract -- -jdk /absolute/path/to/openjdk21
```

Assuming the source tree was configured correctly, the result will be a
collection of .kzip files (one per module) in ~/kythe-openjdk21-output