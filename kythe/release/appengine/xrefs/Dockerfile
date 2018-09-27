# Copyright 2015 The Kythe Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM debian:jessie

RUN apt-get update \
 && apt-get -y upgrade \
 && apt-get -y install libleveldb-dev git curl gcc python

ENV GOPATH=/tmp/gopath
RUN curl -LO https://storage.googleapis.com/golang/go1.10.3.linux-amd64.tar.gz \
 && tar -C /usr/local -xzf go1.10.3.linux-amd64.tar.gz \
 && rm -f go1.10.3.linux-amd64.tar.gz \
 && mkdir -p $GOPATH
ENV PATH="/usr/local/go/bin:${PATH}"

RUN go get --insecure kythe.io/kythe/release/appengine/xrefs \
 && mv $GOPATH/bin/xrefs /usr/local/bin/server \
 && rm -rf $GOPATH

RUN curl -LO https://storage.googleapis.com/pub/gsutil.tar.gz \
 && tar -C /usr/local/ -xzf gsutil.tar.gz \
 && rm -rf gsutil.tar.gz
ENV PATH="/usr/local/gsutil:${PATH}"

EXPOSE 8080
ADD run_server.sh /usr/local/bin/
ENTRYPOINT ["/usr/local/bin/run_server.sh"]

ADD public /srv/public/
