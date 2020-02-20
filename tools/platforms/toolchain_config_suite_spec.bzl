# Copyright 2020 The Kythe Authors. All rights reserved.
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

"""RBE toolchain specification."""

load("//tools/platforms/configs:versions.bzl", "TOOLCHAIN_CONFIG_AUTOGEN_SPEC")

DEFAULT_TOOLCHAIN_CONFIG_SUITE_SPEC = {
    "repo_name": "io_kythe",
    "output_base": "tools/platforms/configs",
    "container_repo": "kythe-repo/kythe-builder",
    "container_registry": "gcr.io",
    "default_java_home": "/usr/lib/jvm/11.29.3-ca-jdk11.0.2/reduced",
    "toolchain_config_suite_autogen_spec": TOOLCHAIN_CONFIG_AUTOGEN_SPEC,
}
