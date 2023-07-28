#
# Copyright 2017 The Kythe Authors. All rights reserved.
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

"""This module provides rules for kzip extraction."""

def kzip_extractor(
        name,
        corpus,
        language,
        rules = "",
        mnemonics = None,
        include = "",
        exclude = "",
        sources = "",
        source_args = "",
        scoped = True,
        encoding = None,
        extractor = "//kythe/go/extractors/cmd/bazel:extract_kzip",
        max_file_size = None):
    """This macro creates extra_action and action_listener for extract_kzip.

    This macro expands to an extra action listener that invokes extract_kzip
    on matching spawn actions to produce a Kythe compilation record in .kzip
    format.

    Args:
      name: name of the build rule ("_extra_action" is appended in output)
      corpus: the required corpus passed to the kzip extractor
      language: the required language passed to the kzip extractor
      rules: the rules passed to the kzip extractor
      mnemonics: the required mnemonics passed to the action listener
      include: optional RE2 matching files to include in the kzip extractor
      exclude: optional RE2 matching files to exclude in the kzip extractor
      sources: optional RE2 matching source files for the kzip extractor
      source_args: optional RE2 matching arguments to consider source files in
        kzip extractor
      scoped: optional boolean whether to match source paths only in target pkg
      encoding: optional encoding (proto|json) for the generated kzip
      extractor: optional label of the extract_kzip tool to use
      max_file_size: optional maximum size of required_input files to include
    """

    if not mnemonics:
        fail("Missing extra action mnemonics")
    if not language:
        fail("The 'language' attribute must be non-empty")
    if not corpus:
        fail("The 'corpus' attribute must be non-empty")

    xa_name = name + "_extra_action"
    xa_output = "$(ACTION_ID).%s.kzip" % language
    xa_args = {
        "corpus": corpus,
        "extra_action": "$(EXTRA_ACTION_FILE)",
        "language": language,
        "output": "$(output %s)" % xa_output,
        "scoped": "true" if scoped else "false",
    }
    if include:
        xa_args["include"] = _quote_re(include)
    if exclude:
        xa_args["exclude"] = _quote_re(exclude)
    if sources:
        xa_args["source"] = _quote_re(sources)
    if source_args:
        xa_args["args"] = _quote_re(source_args)
    if rules:
        xa_args["rules"] = "$(location %s)" % rules
    if max_file_size:
        xa_args["max_file_size"] = max_file_size
    cmdprefix = ""
    if encoding:
        cmdprefix = "env KYTHE_KZIP_ENCODING=%s " % encoding
    native.extra_action(
        name = xa_name,
        data = [rules] if rules else [],
        out_templates = [xa_output],
        tools = [extractor],
        cmd = (
            cmdprefix + "$(location %s) " % extractor + " ".join(sorted([
                "--%s=%s" % (key, value)
                for key, value in xa_args.items()
            ]))
        ),
    )

    native.action_listener(
        name = name,
        extra_actions = [":" + xa_name],
        mnemonics = mnemonics,
        visibility = ["//visibility:public"],
    )

# Quote elements of a regular expression string that will otherwise trigger
# special handling from Bazel ($) or the shell (').
def _quote_re(re):
    return "'%s'" % re.replace("$", "$$").replace("'", "'\\''")
