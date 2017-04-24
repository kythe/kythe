#
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
#

def kindex_extractor(name, corpus, language, rules='', mnemonics=None,
                     include='', exclude='', sources='', source_args='',
                     scoped=True):
  """This macro expands to an extra action listener that invokes extract_kindex
  on matching spawn actions to produce a Kythe compilation record in .kindex
  format.
  """

  if not mnemonics:
    fail("Missing extra action mnemonics")
  if not language:
    fail("The 'language' attribute must be non-empty")
  if not corpus:
    fail("The 'corpus' attribute must be non-empty")

  xa_name = name + "_extra_action"
  xa_tool = "//kythe/go/extractors/cmd/bazel:extract_kindex"
  xa_output = "$(ACTION_ID).%s.kindex" % language
  xa_args = {
      "extra_action": "$(EXTRA_ACTION_FILE)",
      "corpus":       corpus,
      "language":     language,
      "output":       "$(output %s)" % xa_output,
      "scoped":       "true" if scoped else "false",
  }
  if include:
    xa_args['include'] = _quote_re(include)
  if exclude:
    xa_args['exclude'] = _quote_re(exclude)
  if sources:
    xa_args['source'] = _quote_re(sources)
  if source_args:
    xa_args['args'] = _quote_re(source_args)
  if rules:
    xa_args['rules'] = "$(location %s)" % rules

  native.extra_action(
      name = xa_name,
      data = [rules] if rules else [],
      out_templates = [xa_output],
      tools = [xa_tool],
      cmd = (
        "$(location %s) " % xa_tool + " ".join(sorted([
            "--%s=%s" % (key, value) for key, value in xa_args.items()
        ]))
      ),
  )

  native.action_listener(
      name = name,
      extra_actions = [":"+xa_name],
      mnemonics = mnemonics,
      visibility = ['//visibility:public'],
  )


# Quote elements of a regular expression string that will otherwise trigger
# special handling from Bazel ($) or the shell (').
def _quote_re(re):
  return "'%s'" % re.replace("$", "$$").replace("'", "'\\''")
