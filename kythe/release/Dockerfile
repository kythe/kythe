# Copyright 2014 The Kythe Authors. All rights reserved.
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

# Usage: see kythe.sh
# Volumes:
#   /repo         -- Input repository for extraction/indexing
#   /compilations -- Output directory for extracted compilations
#   /graphstore   -- Output directory for indexing
#   /root/.m2     -- Maven cache

################################## <EXTRACTORS>
# Root of repository from which to extract compilations
VOLUME /repo
# Output directory for compilations
VOLUME /compilations
# Allow Maven cache to be shared
VOLUME /root/.m2

# Maven Extractor
ADD kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac_extractor_deploy.jar /kythe/bin/javac_extractor_deploy.jar
ADD kythe/release/maven_extractor.sh /kythe/bin/maven_extractor

# TODO(fromberger): Add new Go extractor.

RUN chmod +x /kythe/bin/*_extractor
################################## </EXTRACTORS>

################################## <INDEXERS>
# C++
ADD kythe/cxx/indexer/cxx/indexer /kythe/bin/c++_indexer.bin
RUN echo 'exec /kythe/bin/c++_indexer.bin --flush_after_each_entry --ignore_unimplemented --noindex_template_instantiations "$@"' > /kythe/bin/c++_indexer

# Java
ADD kythe/java/com/google/devtools/kythe/analyzers/java/indexer_deploy.jar /kythe/bin/java_indexer_deploy.jar
ADD third_party/javac/javac-9-dev-r4023-1.jar /kythe/bin/javac9.jar
RUN echo 'exec java -Xbootclasspath/p:/kythe/bin/javac9.jar -jar /kythe/bin/java_indexer_deploy.jar "$@"' > /kythe/bin/java_indexer

# Go
ADD kythe/go/indexer/cmd/go_indexer/go_indexer /kythe/bin/go_indexer

RUN chmod +x /kythe/bin/*_indexer
################################## </INDEXERS>

################################## <STORAGE>
VOLUME /graphstore

ADD kythe/go/storage/tools/write_entries /kythe/bin/
ADD kythe/go/platform/tools/dedup_stream /kythe/bin/

ADD kythe/go/storage/tools/directory_indexer /kythe/bin/index_repository
################################## </STORAGE>

ADD kythe/release/kythe.sh /kythe/bin/
RUN chmod +x /kythe/bin/*.sh

ENTRYPOINT ["kythe.sh"]
CMD []
