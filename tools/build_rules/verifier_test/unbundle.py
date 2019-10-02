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
#
"""Extract a bundled C++ indexer test into the specified directory."""
import argparse
import contextlib
import errno
import os
import os.path


class BundleWriter(object):

    def __init__(self, root):
        self.includes = []
        self.root = os.path.join(root, "test_bundle")
        self.current = None
        self.open("test.cc")

    def open(self, path):
        make_dirs(os.path.dirname(os.path.join(self.root, path)))
        if self.current is not None:
            self.current.close()
        self.current = open(os.path.join(self.root, path), "w")

    def add_include(self, path):
        self.includes.append(path)

    def close(self):
        self.current.close()
        self.current = None

        cflagspath = os.path.join(os.path.dirname(self.root), "cflags")
        with open(cflagspath, "w") as cflags:
            for path in self.includes:
                cflags.write("-I")
                cflags.write(os.path.join(self.root, path))
                cflags.write("\n")

    def write(self, line):
        self.current.write(line)


@contextlib.contextmanager
def open_writer(root):
    writer = BundleWriter(root)
    try:
        yield writer
    finally:
        writer.close()


def make_dirs(path):
    try:
        os.makedirs(path)
    except OSError as error:
        # Suppress creation failure for even the leaf directory.
        if error.errno != errno.EEXIST:
            raise


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=argparse.FileType("r"))
    parser.add_argument("output_root")

    args = parser.parse_args()
    with open_writer(args.output_root) as writer:
        for line in args.input:
            if line.startswith('#example '):
                writer.open(line.split(None, 1)[1].strip())
            elif line.startswith('#incdir '):
                writer.add_include(line.split(None, 1)[1].strip())
            else:
                writer.write(line)


if __name__ == "__main__":
    main()
