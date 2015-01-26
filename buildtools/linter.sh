#!/bin/sh -e
# Kythe's arcanist script-and-regex linter. Expected output format:
#   /^((?P<name>.+?)::)?(?P<severity>warning|error|advice):(?P<line>\\d+)? (?P<message>.*)$/m
#
# Usage: linter.sh <file>
#
# Arcanist Documentation:
#   https://secure.phabricator.com/book/phabricator/article/arcanist_lint_script_and_regex/

readonly file="$1"
readonly name="$(basename "$1")"

lint_campfire() {
  if ! ./campfire camper -c "$file"; then
    echo 'campfire camper::error: CAMPFIRE file is not correctly formatted'
  fi
}

case "$name" in
  CAMPFIRE) lint_campfire;;
esac
