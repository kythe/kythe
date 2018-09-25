/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Package gradle defines a default kythe.proto.ExtractionConfiguration for Java
// using Gradle.
package gradle

import (
	"io"
	"strings"
)

const defaultConfig = `{
  "required_image": [
    {
      "uri": "openjdk:8",
      "name": "java",
      "copy_spec": [
        {
          "source": "/docker-java-home"
        },
        {
          "source": "/etc/java-8-openjdk"
        }
      ],
      "env_var": [
        {
          "name": "JAVA_HOME",
          "value": "/docker-java-home"
        },
        {
          "name": "PATH",
          "value": "$JAVA_HOME/bin:$PATH"
        }
      ]
    },
    {
      "uri": "gcr.io/kythe_repo/kythe-javac-extractor-artifacts",
      "name": "javac-extractor-artifacts",
      "copy_spec": [
        {
          "source": "/opt/kythe/extractors/runextractor"
        },
        {
          "source": "/opt/kythe/extractors/javac-wrapper.sh"
        },
        {
          "source": "/opt/kythe/extractors/javac_extractor.jar"
        }
      ],
      "env_var": [
        {
          "name": "KYTHE_CORPUS",
          "value": "testcorpus"
        },
        {
          "name": "REAL_JAVAC",
          "value": "$JAVA_HOME/bin/javac"
        },
        {
          "name": "JAVAC_EXTRACTOR_JAR",
          "value": "/opt/kythe/extractors/javac_extractor.jar"
        }
      ]
    },
    {
      "uri": "gradle:latest",
      "name": "gradle",
      "copy_spec": [
        {
          "source": "/usr/share/gradle"
        },
        {
          "source": "/etc/ssl"
        }
      ],
      "env_var": [
        {
          "name": "GRADLE_HOME",
          "value": "/usr/share/gradle"
        },
        {
          "name": "PATH",
          "value": "$GRADLE_HOME/bin:$PATH"
        }
      ]
    }
  ],
  "entry_point": ["/opt/kythe/extractors/runextractor", "gradle", "-build_file", "build.gradle", "-javac_wrapper", "/opt/kythe/extractors/javac-wrapper.sh"]
}`

// Gradle implements the builderType interface for a Gradle repo.
type Gradle struct{}

// Name is a descriptor for Gradle itself.
func (g Gradle) Name() string {
	return "gradle"
}

// BuildFile returns the default build file for a gradle repo, gradle.build.
func (g Gradle) BuildFile() string {
	return "build.gradle"
}

// DefaultConfig returns a reader for a valid
// kythe.proto.ExtractionConfiguration object that works for many repos.
func (g Gradle) DefaultConfig() io.Reader {
	return strings.NewReader(defaultConfig)
}
