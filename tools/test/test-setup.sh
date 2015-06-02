#!/bin/bash

# shift stderr to stdout.
exec 2>&1

# Executing the test log will page it.
echo 'exec ${PAGER:-/usr/bin/less} "$0" || exit 1'

# normal commands are run in the exec-root where they have access to
# the entire source tree. By chdir'ing to the runfiles root, tests only
# have direct access to their declared dependencies.
cd "$TEST_SRCDIR" || { echo "Could not chdir $TEST_SRCDIR"; exit 1; }

"$@"
