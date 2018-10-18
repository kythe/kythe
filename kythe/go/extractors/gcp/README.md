# Kythe Extracting on GCP

This package contains nothing of note, but will eventually support extracting
Kythe Compilation Units on Google Cloud Platform.

## Cloud Build

Documentation for Cloud Build itself is available at
https://cloud.google.com/cloud-build/.

For the rest of this test documentation, we'll assume you've run those setup
instructions.  Additionally, you should make an environment variable for your
gs bucket:

```
export BUCKET_NAME="your-bucket-name"
```

## Hello World Test

To make sure you have done setup correctly, we have an example binary at
`kythe/go/extractors/gcp/helloworld`, which you can run as follows:

```
gcloud builds submit --config cloudbuild.yaml --substitutions=_BUCKET_NAME="$BUCKET_NAME" .
```

If that fails, you have to go back up to the [Cloud Build](#cloud-build) section
and follow the installation steps.  Of note, you will have to install `gcloud`,
authorize it, associate it with a valid project id, create a test gs bucket.

## Maven Proof of Concept

To extract a maven repository using Kythe on Cloud Build, use
`mvnpoc/cloudbuild.yaml`.  This assumes that you will specify a maven repository
in `_REPO_NAME`, and that the repository has a top-level `pom.xml` file (right
now it is a hard-coded location, but in the future it will be configurable).
This also assumes you specify `_BUCKET_NAME` as per the Hello World Test above.

```
gcloud builds submit --config cloudbuild.yaml \
--substitutions=\
_BUCKET_NAME="$BUCKET_NAME",\
_REPO_NAME=https://github.com/project-name/repo-name
```

This Cloud Build uses two artifacts in gcr.io/kythe-public:

### Extractor Artifacts

gcr.io/kythe-public/kythe-javac-extractor-artifacts created from
`kythe/java/com/google/devtools/kythe/extractors/java/artifacts` contains:

* `javac-wrapper.sh` script which calls Kythe extraction and then an actual java
  compiler
* `javac_extractor.jar` which is the Kythe java extractor
* `javac9_tools.jar` which contains javac langtools for JDK 9, but targets JRE 8

gcr.io/kythe-public/build-preprocessor is just
[kythe/go/extractors/config/preprocessor](https://github.com/google/kythe/blob/master/kythe/go/extractors/config/preprocessor/preprocessor.go),
which we use to preprocess the `pom.xml` build configuration to be able to
specify all of the above custom javac extraction logic.
