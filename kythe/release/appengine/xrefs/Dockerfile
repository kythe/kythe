# Copyright 2014 Google Inc. All rights reserved.
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

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
      ca-certificates nginx python wget unzip rsync gawk && \
    apt-get clean

EXPOSE 8080

RUN wget https://dl.google.com/dl/cloudsdk/release/google-cloud-sdk.zip && \
    unzip google-cloud-sdk.zip && \
    rm google-cloud-sdk.zip
RUN google-cloud-sdk/install.sh --usage-reporting=true --path-update=true --bash-completion=true --rc-path=/.bashrc --disable-installation-options
ENV PATH /google-cloud-sdk/bin:$PATH

ADD run.sh /usr/local/bin/
ENTRYPOINT ["/usr/local/bin/run.sh"]
CMD []

ADD update_graphstore.sh /usr/local/bin/
ADD update_schema.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/*.sh

ADD public /srv/public/

ADD nginx.conf /etc/nginx/nginx.conf
ADD server /usr/local/bin/
