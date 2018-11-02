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

FROM google/kythe-base
# TODO(schroederc): reuse //kythe/java/com/google/devtools/kythe/extractors/java/standalone:docker

# Output location for .kindex files
VOLUME /idx

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y mercurial curl zsh \
      ant build-essential autoconf automake binutils cpio procps gawk m4 file libmotif-dev libcups2-dev libfreetype6-dev libasound2-dev libX11-dev libxext-dev libxrender-dev libxtst-dev libxt-dev unzip zip && \
    apt-get clean -y
ENTRYPOINT ["/usr/bin/zsh"]

RUN hg clone http://hg.openjdk.java.net/jdk8/jdk8/ /tmp/jdk8
WORKDIR /tmp/jdk8

RUN sh ./get_source.sh
RUN sh ./configure \
      --with-freetype-lib=/usr/lib/x86_64-linux-gnu --with-freetype-include=/usr/include/freetype2

# Fix make4.0 issue
RUN curl http://hg.openjdk.java.net/jdk9/dev/hotspot/raw-rev/e8d4d0db1f06 \
    | tail -n+10 \
    | patch -d hotspot -p1

# Ensure all dependencies are present
RUN LOG= make JOBS=4 jdk

ADD kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac_extractor_deploy.jar /kythe/bin/
ADD kythe/extractors/openjdk/extract.sh /kythe/bin/

# Extract compilations when run
ENTRYPOINT ["/kythe/bin/extract.sh"]
