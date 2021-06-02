#!/usr/bin/env python2
#
# Copyright 2016 The Kythe Authors. All rights reserved.
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

# This script (re)generates the source for Go protobuf packages so that Go
# packages can be fetched and installed with the "go get" command. It also
# generates the Rust protobuf code.
#
# This script must be run with its current working directory inside a Bazel
# workspace root.
#
# N.B.: This script depends on some conventions about how we name our proto
# rules. Specifically, that the name of the go_proto_library rule for
# "foo_proto" is "foo_go_proto".

from subprocess import check_output
from subprocess import call

import glob
import os
import re
import shlex
import shutil
import stat
import sys

# Find the locations of the workspace root and the generated files directory.
workspace = check_output(['bazel', 'info', 'workspace']).strip()
bazel_bin = check_output(['bazel', 'info', 'bazel-bin']).strip()
targets = '//kythe/...'
import_base = 'kythe.io'

def do_lang(lang, ext):
    protos = check_output([
        'bazel',
        'query',
        'kind("%s_proto_library", %s)' % (lang, targets),
    ]).split()

    # Each rule has the form //foo/bar:baz_proto.
    # First build all the rules to ensure we have the output files.
    # Then strip off each :baz_proto, convert it to a filename "baz.proto",
    # and copy the generated output "baz.pb.go" into the source tree.
    if call(['bazel', 'build'] + protos) != 0:
        print('Build failed')
        sys.exit(1)

    for rule in protos:
        # Example: //kythe/proto:blah_go_proto -> kythe/proto, blah_go_proto
        rule_dir, proto = rule.lstrip('/').rsplit(':', 1)
        # Example: $ROOT/kythe/proto/blah_go_proto
        output_dir = os.path.join(workspace, rule_dir, proto)
        # Example: blah_go_proto -> blah.proto
        proto_file = re.sub('_%s_proto$' % lang, '.proto', proto)

        print('Copying generated protobuf source for %s' % rule)
        generated_file = re.sub('.proto$', '.%s' % ext, proto_file)
        check_path = os.path.join(bazel_bin, rule_dir, proto + '_', import_base,
                                  rule_dir, proto, generated_file)
        if lang == 'rust':
            check_path = os.path.join(bazel_bin, rule_dir,
                                      proto + '.proto.rust', generated_file)
        generated_path = glob.glob(check_path).pop()

        if os.path.isdir(output_dir):
            print('Deleting and recreating old protobuf directory: %s'
                  % output_dir)
            shutil.rmtree(output_dir)
        else:
            print('Creating new generated protobuf: %s' % generated_file)

        # Ensure the output directory exists, and update permissions after copying.
        os.makedirs(output_dir, 0o755)
        shutil.copy(generated_path, output_dir)
        os.chmod(os.path.join(output_dir, generated_file), 0o644)
        if lang == 'rust':
            lib_rs = os.path.join(bazel_bin, rule_dir, proto + '.proto.rust',
                                  'lib.rs')
            shutil.copy(lib_rs, output_dir)
            os.chmod(os.path.join(output_dir, 'lib.rs'), 0o644)
            with open(os.path.join(output_dir, 'lib.rs'), 'a') as f:
                f.write('\n')

do_lang(lang='go', ext='pb.go')
do_lang(lang='rust', ext='rs')
