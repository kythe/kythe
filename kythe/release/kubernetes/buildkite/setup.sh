#!/bin/bash
CLUSTER=buildkite-agent-cluster
ZONE=us-central1-a
PROJECT=kythe-repo

gcloud container clusters get-credentials $CLUSTER --zone $ZONE --project $PROJECT
