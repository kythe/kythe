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

FROM google/kythe-base

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      asciidoc source-highlight graphviz libncurses-dev gcc \
      bundler ruby-dev make nodejs rsync patch \
 && apt-get clean

# Setup locale (needed for campfire)
RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8
ENV LC_ALL en_US.UTF-8
ENV LC_CTYPE en_US.UTF-8
ENV LC_NUMERIC en_US.UTF-8
ENV LC_TIME en_US.UTF-8
ENV LC_COLLATE en_US.UTF-8
ENV LC_MONETARY en_US.UTF-8
ENV LC_MESSAGES en_US.UTF-8
ENV LC_PAPER en_US.UTF-8
ENV LC_NAME en_US.UTF-8
ENV LC_ADDRESS en_US.UTF-8
ENV LC_TELEPHONE en_US.UTF-8
ENV LC_MEASUREMENT en_US.UTF-8
ENV LC_IDENTIFICATION en_US.UTF-8

# Helper script to set language tooling flags
ADD campfire-wrapper /kythe/bin/campfire
ADD campfire_settings /root/.campfire_settings
ENV CAMPFIRE_ROOT /repo

ENTRYPOINT ["campfire"]
