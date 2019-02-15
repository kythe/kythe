# Google Cloud Build -- Bazel Kythe extraction

Generic configuration for using `gcr.io/kythe-public/bazel-extractor` as a
Google Cloud Builder for a git repository.

## Configuration variables

* `COMMIT_SHA`: git repository commit to checkout, build, and extract
* `REPO_NAME`: source git repository URL
* `_BAZEL_TARGET`: Bazel target to build (defaults to `//...`)
* `_BUCKET_NAME`: GCS bucket name to store extracted compilations
* `_KYTHE_CORPUS`: Kythe corpus label

## Example usage

```shell
# Extract github.com/angular/angular at commit 8accc98
gcloud builds submit --no-source --config kythe/extractors/gcp/bazel/bazel.yaml \
  --substitutions=REPO_NAME=https://github.com/angular/angular.git,\
COMMIT_SHA=8accc98d28249628e84136d7306fdbbe1f4caaef,\
_BUCKET_NAME=<GCS_BUCKET_NAME>,\
_KYTHE_CORPUS=github.com/angular/angular

# Extract github.com/bazelbuild/bazel at commit 22d375b
gcloud builds submit --no-source --config kythe/extractors/gcp/bazel/bazel.yaml \
  --substitutions=REPO_NAME=https://github.com/bazelbuild/bazel.git,\
COMMIT_SHA=22d375bd532b04bb83f18a7770e5080e23a1d517,\
_BUCKET_NAME=<GCS_BUCKET_NAME>,\
_KYTHE_CORPUS=github.com/bazelbuild/bazel

# Extract github.com/protocolbuffers/protobuf at commit e728325
gcloud builds submit --no-source --config kythe/extractors/gcp/bazel/bazel.yaml \
  --substitutions=REPO_NAME=https://github.com/protocolbuffers/protobuf.git,\
COMMIT_SHA=e7283254d6eb01ddfdb63cc3c89cd312e2d354d5,\
_BUCKET_NAME=<GCS_BUCKET_NAME>,\
_KYTHE_CORPUS=github.com/protocolbuffers/protobuf
```
