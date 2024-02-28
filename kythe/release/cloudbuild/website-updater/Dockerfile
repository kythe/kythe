FROM gcr.io/kythe-repo/bazelisk-builder-client

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
    asciidoc asciidoctor source-highlight graphviz ruby-dev build-essential \
    rsync jq

# install `gcloud` cli tool
RUN curl -O https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-466.0.0-linux-x86_64.tar.gz
RUN tar -xf google-cloud-cli-466.0.0-linux-x86_64.tar.gz
RUN ./google-cloud-sdk/install.sh
ENV PATH="${PATH}:/google-cloud-sdk/bin"
RUN which gcloud

# install extra components needed for website deployment
RUN gcloud components install app-engine-go cloud-datastore-emulator app-engine-python-extras
