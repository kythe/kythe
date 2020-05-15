#!/usr/bin/python

# Copyright 2017 Google Inc. All rights reserved.
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

# ==============================================================================
# This .ycm_extra_conf will be picked up automatically for code completion using
# YouCompleteMe.
#
# See https://valloric.github.io/YouCompleteMe/ for instructions on setting up
# YouCompleteMe using Vim. This .ycm_extra_conf file also works with any other
# completion engine that uses YCMD (https://github.com/Valloric/ycmd).
#
# Code completion uses `bazel print_action` for the file and pulls out the
# included command line flags.
# ==============================================================================

import os
import re
import subprocess
import logging

BAZEL_PRINT_FLAGS = [
    # Run bazel.
    'bazel',
    # Dump the text proto action on STDOUT.
    'print_action',
    # Be quiet.
    '--noshow_progress',
    '--noshow_loading_progress',
    '--show_result=0',
    # Don't build anything and keep going.
    '--nobuild',
    '--keep_going',
    '--workspace_status_command=/bin/true',
    # Execute a single arbitrary action for the file.
    '--compile_one_dependency',
]

FLAG_PATTERN = re.compile(r'^\s+compiler_option: "([^"]*)"')

# Workspace path.
WORKSPACE_PATH = None

# Execution root.
EXECUTION_ROOT = None


def InitBazelConfig():
  """Initialize globals based on Bazel configuration.

  Initialize COMPILATION_DATABASE_PATH, WORKSPACE_PATH, and
  CANONICAL_SOURCE_FILE based on Bazel. These values are not expected to change
  during the session.
  """
  global WORKSPACE_PATH
  global EXECUTION_ROOT
  EXECUTION_ROOT = subprocess.check_output(['bazel', 'info',
                                            'execution_root']).strip()
  WORKSPACE_PATH = subprocess.check_output(['bazel', 'info',
                                            'workspace']).strip()


def ExpandAndNormalizePath(filename, basepath=None):
  """Resolves |filename| relative to |basepath| and expands symlinks."""
  if basepath is None:
    basepath = WORKSPACE_PATH
  if not os.path.isabs(filename) and basepath:
    filename = os.path.join(basepath, filename)
  filename = os.path.realpath(filename)
  return str(filename)


def RelativePath(filename, root=None):
  """Resolves |filename| and returns a path relative to |root| if possible."""
  if root is None:
    root = WORKSPACE_PATH
  path = ExpandAndNormalizePath(filename, basepath=root)
  if path.startswith(root):
    path = path[len(root):].lstrip('/')
  return path


# Entrypoint for YouCompleteMe.
def Settings(**kwargs):
  if kwargs['language'] != 'cfamily':
    return {}

  if EXECUTION_ROOT is None:
    InitBazelConfig()

  filename = RelativePath(kwargs.pop('filename'))
  logging.info('Calling bazel print_action for %s', filename)
  try:
    action = subprocess.check_output(
        BAZEL_PRINT_FLAGS + [filename], stderr=subprocess.STDOUT)
  except subprocess.CalledProcessError as err:
    logging.error('Error calling bazel %s (%s)', err, err.output)
    return {}

  flags = [
      FLAG_PATTERN.match(line).group(1)
      for line in action.split('\n')
      if FLAG_PATTERN.match(line)
  ]

  logging.info('Found flags %s', flags)

  return {
      # Always indicate C++.
      'flags': ['-x', 'c++'] + flags,
      'include_paths_relative_to_dir': EXECUTION_ROOT,
  }


# For testing.
if __name__ == '__main__':
  import sys
  print Settings(filename=sys.argv[1])
