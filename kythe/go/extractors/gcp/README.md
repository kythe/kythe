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

