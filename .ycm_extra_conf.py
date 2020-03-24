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
# Code completion depends on a Clang compilation database. This is placed in a
# file named `compile_commands.json` in your execution root path. I.e. it will
# be at the path returned by `bazel info execution_root`.
#
# If the compilation database isn't available, this script will generate one
# using tools/cpp/generate_compilation_database.sh. This process can be slow if
# you haven't built the sources yet. It's always a good idea to run
# generate_compilation_database.sh manually so that you can see the build output
# including any errors encountered during compile command generation.
# ==============================================================================

import json
import os
import shlex
import subprocess
import time
import logging

# If all else fails, then return this list of flags.
DEFAULT_FLAGS = []

CANONICAL_SOURCE_FILE = 'kythe/cxx/extractor/cxx_extractor_main.cc'

# Full path to directory containing compilation database. This is usually
# |execution_root|/compile_commands.json.
COMPILATION_DATABASE_PATH = None

# Workspace path.
WORKSPACE_PATH = None

# The compilation database. This is a mapping from the absolute normalized path
# of the source file to it's compile command broken down into an array.
COMPILATION_DATABASE = {}

# If loading the compilation database failed for some reason,
# LAST_INIT_FAILURE_TIME contains the value of time.clock() at the time the
# failure was encountered.
LAST_INIT_FAILURE_TIME = None

# If this many seconds have passed since the last failure, then try to generate
# the compilation database again.
RETRY_TIMEOUT_SECONDS = 120

HEADER_EXTENSIONS = ['.h', '.hpp', '.hh', '.hxx']
SOURCE_EXTENSIONS = ['.cc', '.cpp', '.c', '.m', '.mm', '.cxx']

NORMALIZE_PATH = 1
REMOVE = 2

# List of clang options and what to do with them. Use the '-foo' form for flags
# that could be used as '-foo <arg>' and '-foo=<arg>' forms, and use '-foo=' for
# flags that can only be used as '-foo=<arg>'.
#
# Mapping a flag to NORMALIZE_PATH causes its argument to be normalized against
# the build directory via ExpandAndNormalizePath(). REMOVE causes both the flag
# and its value to be removed.
CLANG_OPTION_DISPOSITION = {
    '-I': NORMALIZE_PATH,
    '-MF': REMOVE,
    '-cxx-isystem': NORMALIZE_PATH,
    '-dependency-dot': REMOVE,
    '-dependency-file': REMOVE,
    '-fbuild-session-file': REMOVE,
    '-fmodule-file': NORMALIZE_PATH,
    '-fmodule-map-file': NORMALIZE_PATH,
    '-foptimization-record-file': REMOVE,
    '-fprebuilt-module-path': NORMALIZE_PATH,
    '-fprofile-generate=': REMOVE,
    '-fprofile-instrument-generate=': REMOVE,
    '-fprofile-user=': REMOVE,
    '-gcc-tollchain=': NORMALIZE_PATH,
    '-idirafter': NORMALIZE_PATH,
    '-iframework': NORMALIZE_PATH,
    '-imacros': NORMALIZE_PATH,
    '-include': NORMALIZE_PATH,
    '-include-pch': NORMALIZE_PATH,
    '-iprefix': NORMALIZE_PATH,
    '-iquote': NORMALIZE_PATH,
    '-isysroot': NORMALIZE_PATH,
    '-isystem': NORMALIZE_PATH,
    '-isystem-after': NORMALIZE_PATH,
    '-ivfsoverlay': NORMALIZE_PATH,
    '-iwithprefixbefore': NORMALIZE_PATH,
    '-iwithsysroot': NORMALIZE_PATH,
    '-o': REMOVE,
    '-working-directory': NORMALIZE_PATH,
}


def ProcessOutput(args):
  """Run the program described by |args| and return its stdout as a stream.

  |stderr| and |stdin| will be set to /dev/null. Will raise CalledProcessError
  if the subprocess doesn't complete successfully.
  """
  output = ''
  with open(os.devnull, 'w') as err:
    with open(os.devnull, 'r') as inp:
      output = subprocess.check_output(args, stderr=err, stdin=inp)
  return str(output).strip()


def InitBazelConfig():
  """Initialize globals based on Bazel configuration.

  Initialize COMPILATION_DATABASE_PATH, WORKSPACE_PATH, and
  CANONICAL_SOURCE_FILE based on Bazel. These values are not expected to change
  during the session.
  """
  global COMPILATION_DATABASE_PATH
  global WORKSPACE_PATH
  global CANONICAL_SOURCE_FILE
  execution_root = ProcessOutput(['bazel', 'info', 'execution_root'])
  COMPILATION_DATABASE_PATH = os.path.join(execution_root,
                                           'compile_commands.json')
  WORKSPACE_PATH = ProcessOutput(['bazel', 'info', 'workspace'])
  CANONICAL_SOURCE_FILE = ExpandAndNormalizePath(CANONICAL_SOURCE_FILE,
                                                 WORKSPACE_PATH)


def GenerateCompilationDatabaseSlowly(filename):
  """Generate compilation database. May take a while."""
  script_path = os.path.join(WORKSPACE_PATH, 'tools', 'cpp',
                             'generate_compilation_database.sh')
  ProcessOutput([script_path, filename])


def ExpandAndNormalizePath(filename, basepath=WORKSPACE_PATH):
  """Resolves |filename| relative to |basepath| and expands symlinks."""
  if not os.path.isabs(filename) and basepath:
    filename = os.path.join(basepath, filename)
  filename = os.path.realpath(filename)
  return str(filename)


def PrepareCompileFlags(compile_command, basepath):
  flags = shlex.split(compile_command)
  flags_to_return = []
  use_next_flag_as_value_for = None

  def HandleFlag(name, value, combine):
    disposition = CLANG_OPTION_DISPOSITION.get(name, None)
    if disposition is None and combine:
      disposition = CLANG_OPTION_DISPOSITION.get(name + '=', None)

    if disposition == REMOVE:
      return

    if disposition == NORMALIZE_PATH:
      value = ExpandAndNormalizePath(value, basepath)

    if combine:
      flags_to_return.append('{}={}'.format(name, value))
    else:
      flags_to_return.extend([name, value])

  for flag in flags:
    if use_next_flag_as_value_for is not None:
      name = use_next_flag_as_value_for
      use_next_flag_as_value_for = None
      HandleFlag(name, flag, combine=False)
      continue

    if '=' in flag:  # -foo=bar
      name, value = flag.split('=', 1)
      HandleFlag(name, value, combine=True)
      continue

    if flag in CLANG_OPTION_DISPOSITION:
      use_next_flag_as_value_for = flag
      continue

    if flag.startswith('-I'):
      HandleFlag('-I', flag[2:], combine=False)
      continue

    flags_to_return.append(flag)

  return flags_to_return


def LoadCompilationDatabase(filename):
  if not os.path.exists(COMPILATION_DATABASE_PATH):
    GenerateCompilationDatabaseSlowly(filename)

  with open(COMPILATION_DATABASE_PATH, 'r') as database:
    database_dict = json.load(database)

  global COMPILATION_DATABASE
  COMPILATION_DATABASE = {}

  for entry in database_dict:
    filename = ExpandAndNormalizePath(entry['file'], WORKSPACE_PATH)
    directory = entry['directory']
    command = entry['command']
    COMPILATION_DATABASE[filename] = {
        'command': command,
        'directory': directory
    }


def IsHeaderFile(filename):
  extension = os.path.splitext(filename)[1]
  return extension in HEADER_EXTENSIONS


def FindAlternateFile(filename):
  if IsHeaderFile(filename):
    basename = os.path.splitext(filename)[0]
    for extension in SOURCE_EXTENSIONS:
      new_filename = basename + extension
      if new_filename in COMPILATION_DATABASE:
        return new_filename

  # Try something in the same directory.
  directory = os.path.dirname(filename)
  for key in COMPILATION_DATABASE.iterkeys():
    if key.startswith(directory) and os.path.dirname(key) == directory:
      return key

  return CANONICAL_SOURCE_FILE


# Entrypoint for YouCompleteMe.
def FlagsForFile(filename, **kwargs):
  global LAST_INIT_FAILURE_TIME

  if len(COMPILATION_DATABASE) == 0 or filename not in COMPILATION_DATABASE:
    if LAST_INIT_FAILURE_TIME is not None and time.clock(
    ) - LAST_INIT_FAILURE_TIME < RETRY_TIMEOUT_SECONDS:
      return {'flags': DEFAULT_FLAGS}

    try:
      InitBazelConfig()
      LoadCompilationDatabase(filename)
    except Exception as e:
      LAST_INIT_FAILURE_TIME = time.clock()
      return {'flags': DEFAULT_FLAGS}

  filename = str(os.path.realpath(filename))
  if filename not in COMPILATION_DATABASE:
    filename = FindAlternateFile(filename)

  if filename not in COMPILATION_DATABASE:
    return {'flags': DEFAULT_FLAGS}

  result_dict = COMPILATION_DATABASE[filename]
  return {
      'flags':
          PrepareCompileFlags(result_dict['command'], result_dict['directory'])
  }


# For testing.
if __name__ == '__main__':
  import sys
  print FlagsForFile(sys.argv[1])
