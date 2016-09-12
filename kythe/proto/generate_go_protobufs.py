# Copyright 2016 Google Inc. All rights reserved.
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

from subprocess import check_output
from subprocess import call

import os
import re
import shlex
import shutil
import stat

workspace = check_output(['bazel', 'info', 'workspace']).strip()
bazel_genfiles = check_output(['bazel', 'info', 'bazel-genfiles']).strip()
relative_path = os.path.relpath(os.getcwd(), workspace)

package = "//" + relative_path + "/..."

go_protos = check_output(['bazel', 'query', 'attr("gen_go", 1, %s)' % package]).split()
for proto_rule in go_protos:
  # proto_rule is now something like //foo/bar:baz_proto
  proto = proto_rule.rsplit(':', 1)[-1]
  # proto is now something like baz_proto
  out_dir = proto
  filename = re.sub('_proto$', '.proto', proto)
  # filename is now something like baz.proto

  print "Updating Go protobuf for %s" % filename
  gen_go_src = re.sub('.proto$', '.pb.go', filename)
  # gen_go_src is now something like baz.pb.go

  # Build the Go proto
  go_proto_rule = proto_rule + "_go"
  if call(['bazel', 'build', go_proto_rule]) != 0:
    print "Build failed"
    system.exit(1)
  output_gopb = os.path.join(bazel_genfiles, relative_path, gen_go_src)

  if os.path.isdir(out_dir):
    print "Deleting and recreating old protobuf directory: %s" % out_dir
    shutil.rmtree(out_dir)
  else:
    print "Creating new Go protobuf: %s" % gen_go_src

  os.makedirs(out_dir, 0755)
  shutil.copy(output_gopb, out_dir)
  dest = os.path.join(out_dir, gen_go_src)
  os.chmod(dest, 0644)
