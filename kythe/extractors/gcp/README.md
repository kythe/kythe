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
`kythe/extractors/gcp/examples/helloworld`, which you can run as follows:

```
gcloud builds submit --config examples/helloworld/helloworld.yaml \
  --substitutions=_BUCKET_NAME="$BUCKET_NAME"\
  examples/helloworld
```

If that fails, you have to go back up to the [Cloud Build](#cloud-build) section
and follow the installation steps.  Of note, you will have to install `gcloud`,
authorize it, associate it with a valid project id, create a test gs bucket.

## Maven Proof of Concept

To extract a maven repository using Kythe on Cloud Build, use
`examples/mvn.yaml`.  This assumes that you will specify a maven repository
in `_REPO`, and that the repository has a top-level `pom.xml` file (right
now it is a hard-coded location, but in the future it will be configurable).
This also assumes you specify `$BUCKET_NAME` as per the Hello World Test above.
`_CORPUS` can be any identifying string for your repo, for example: "guava".

```
gcloud builds submit --config examples/mvn.yaml \
  --substitutions=\
_BUCKET_NAME=$BUCKET_NAME,\
_REPO=https://github.com/project-name/repo-name,\
_COMMIT=<version-hash>,\
_CORPUS=repo-name\
  --no-source
```

### Guava specific example

To extract multiple parts of https://github.com/google/guava, use
`examples/guava-mvn.yaml`.

```
gcloud builds submit --config examples/guava-mvn.yaml \
  --substitutions=\
_BUCKET_NAME=$BUCKET_NAME,\
_COMMIT=<commit-hash>,\
  --no-source
```

This outputs `guava-<commit-hash>.kzip` to `$BUCKET_NAME` on Google Cloud Storage.

This is a reasonable example of a maven project which has already specified
the requisite `maven-compiler-plugin` bits in their `pom.xml` files to support
Kythe extraction, and also a project which has multiple modules.

Note however not all directories from guava extract with the top-level action.
For example if you want to extract the android copy of guava that lives inside
of the guava tree, you would need a slightly different action:

```
gcloud builds submit --config examples/guava-android-mvn.yaml \
  --substitutions=\
_BUCKET_NAME=$BUCKET_NAME,\
_COMMIT=<commit-hash>\
  --no-source
```

This outputs `guava-android-<commit-hash>kzip` to `$BUCKET_NAME` on GCS.

## Gradle Proof of Concept

Gradle is extracted similarly:

```
gcloud builds submit --config examples/gradle.yaml \
  --substitutions=\
_BUCKET_NAME=$BUCKET_NAME,\
_REPO=https://github.com/project-name/repo-name,\
_COMMIT=<version-hash>,\
_CORPUS=repo-name\
  --no-source
```

## Bazel Extraction

* `_COMMIT`: git repository commit to checkout, build, and extract
* `_REPO`: source git repository URL
* `_BUCKET_NAME`: GCS bucket name to store extracted compilations
* `_CORPUS`: Kythe corpus label

```shell
# Extract github.com/angular/angular at commit 8accc98
gcloud builds submit --no-source --config kythe/extractors/gcp/bazel/bazel.yaml \
  --substitutions=_REPO=https://github.com/angular/angular.git,\
_COMMIT=8accc98d28249628e84136d7306fdbbe1f4caaef,\
_BUCKET_NAME=$BUCKET_NAME>,\
_KYTHE_CORPUS=github.com/angular/angular

# Extract github.com/bazelbuild/bazel at commit 22d375b
gcloud builds submit --no-source --config kythe/extractors/gcp/bazel/bazel.yaml \
  --substitutions=_REPO=https://github.com/bazelbuild/bazel.git,\
_COMMIT=22d375bd532b04bb83f18a7770e5080e23a1d517,\
_BUCKET_NAME=$BUCKET_NAME>,\
_KYTHE_CORPUS=github.com/bazelbuild/bazel

# Extract github.com/protocolbuffers/protobuf at commit e728325
gcloud builds submit --no-source --config kythe/extractors/gcp/bazel/bazel.yaml \
  --substitutions=_REPO=https://github.com/protocolbuffers/protobuf.git,\
_COMMIT=e7283254d6eb01ddfdb63cc3c89cd312e2d354d5,\
_BUCKET_NAME=$BUCKET_NAME>,\
_KYTHE_CORPUS=github.com/protocolbuffers/protobuf
```

## Cloud Build REST API

Cloud Build has a REST API described at
https://cloud.google.com/cloud-build/docs/api/reference/rest/.  For Kythe
extraction, we have a test binary that lets you isolate authentication problems
before dealing with real builds.

You will need access to your project's service credentials:

https://cloud.google.com/docs/authentication/production#obtaining_and_providing_service_account_credentials_manually

If your team already has credentials made for this purpose, see if you can
re-use them.

If not, you can use these steps to create new credentials:

1. In your GCP console, click on the top left hamburger icon
2. Click on APIs & Services
3. In the dropdown, click on Credentials
4. Now you can mostly follow the instructions from the [above link](https://cloud.google.com/docs/authentication/production#obtaining_and_providing_service_account_credentials_manually),
   however note:
5. When making a service account key, you can select the Cloud Build roles,
   instead of "project owner", to have better limiting of resources.
6. You will still download the json file and set environment variable
   `GOOGLE_APPLICATION_CREDENTIALS` as described in the above link.

To test, run

```
bazel build kythe/extractors/gcp/examples/restcheck:rest_auth_check
./bazel-bin/kythe/extractors/gcp/examples/restcheck/rest_auth_check -project_id=some-project
```

If that returns with a 403 error, you likely did the authentication steps above
incorrectly.

## Associated extractor images

Kythe team maintains a few images useful for extracting Kythe data on Google
Cloud Build.  Many of these are used in example scripts and other generated GCB
executions in Kythe.

### gcr.io/kythe-public/kythe-javac-extractor-artifacts

Created from
[kythe/java/com/google/devtools/kythe/extractors/java/artifacts](https://github.com/kythe/kythe/blob/master/kythe/java/com/google/devtools/kythe/extractors/java/artifacts),
this image contains:

* `javac-wrapper.sh` script which calls Kythe extraction and then an actual java
  compiler
* `javac_extractor.jar` which is the Kythe java extractor
* `javac9_tools.jar` which contains javac langtools for JDK 9, but targets JRE 8

### gcr.io/kythe-public/bazel-extractor

Created from
[kythe/extractors/bazel](https://github.com/kythe/kythe/blob/master/kythe/extractors/bazel),
this image contains all of the pieces of kythe necessary to extract supported
languages - bazel itself, all of the kythe extractors, and the `.bazelrc`.
Additionally, it contains necessary tools (including a copy of `kzip-tools`
described below), and some required scripts.

When running this docker image, you must set environment variable
`$KYTHE_OUTPUT_DIRECTORY`.

### gcr.io/kythe-public/build-preprocessor

This is a simple wrapper around
[kythe/go/extractors/config/preprocessor](https://github.com/kythe/kythe/blob/master/kythe/go/extractors/config/preprocessor/preprocessor.go),
which we use to preprocess build configurations to be able to
specify all of the above custom javac extraction logic.  Supports maven
`pom.xml` files and gradle `build.gradle` files.  Ironically, bazel extraction
doesn't need its `BUILD` files modified, because you can pass extractors
directly as `extra_action`, so `build-preprocessor` doesn't support `BUILD`
files.

### gcr.io/kythe-public/kzip-tools

This image exposes the binary
[kythe/go/platform/tools/kzip](https://github.com/kythe/kythe/blob/master/kythe/go/platform/tools/kzip/kzip.go),
which currently supports merging multiple kzips together and creating trivial
kzips from the command line.

## Troubleshooting

### Generic failure to use gcloud

Make sure you've followed the setup setps above in [Cloud Build](#cloud-build),
especially `gcloud auth login`.

### Step #N: fatal: could not read Username for 'https://github.com': No such device or address

This, confusingly, could be two completely separate errors.  First, and simpler
to check, you could have just spelled the repo incorrectly.  If you have a
typo in the repo name, instead of telling you "repo doesn't exist" or something,
the failure message is the above error about "could not read Username".

If you have verified that the repo name is spelled correctly, then you may be
trying to access a private git repo.  It is possible to clone out of a private
git repo, but you need to follow some extra steps.  This will involve using
Cloud KMS, and the steps are described in this
[Cloud Build Help
Doc](https://cloud.google.com/cloud-build/docs/access-private-github-repos).
This will involve adding extra steps to your `.yaml` file for decrypting a
provided key and using it to authenticate with git.  Finally, your existing git
clone step will need to be modified to use the same root volume as your two new
steps.
