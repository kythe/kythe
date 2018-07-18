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

// Package mvn defines a default kythe.proto.ExtractionConfiguration for Java
// using Maven.
package mvn

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
          "source": "/opt/kythe/extractors/javac-wrapper.sh"
        },
        {
          "source": "/opt/kythe/extractors/javac_extractor.jar"
        },
        {
          "source": "/opt/kythe/extractors/mvn-extract.sh"
        },
        {
          "source": "/opt/kythe/extractors/mvn_pom_preprocessor.jar"
        }      
      ],
      "env_var": [
        {
          "name": "REAL_JAVAC",
          "value": "$JAVA_HOME/bin/javac"
        },
        {
          "name": "JAVAC_EXTRACTOR_JAR",
          "value": "/opt/kythe/extractors/javac_extractor.jar"
        },
        {
          "name": "JAVAC_WRAPPER",
          "value": "/opt/kythe/extractors/javac-wrapper.sh"
        },
        {
          "name": "MVN_POM_PREPROCESSOR",
          "value": "/opt/kythe/extractors/mvn_pom_preprocessor.jar"
        }
      ]
    },
    {
      "uri": "maven:latest",
      "name": "maven",
      "copy_spec": [
        {
          "source": "/usr/share/maven"
        },
        {
          "source": "/etc/ssl"
        }
      ],
      "env_var": [
        {
          "name": "MAVEN_HOME",
          "value": "/usr/share/maven"
        },
        {
          "name": "PATH",
          "value": "$MAVEN_HOME/bin:$PATH"
        }
      ]
    }
  ],
  "entry_point": ["/opt/kythe/extractors/mvn-extract.sh"]
}`

// DefaultConfig returns a reader for a valid
// kythe.proto.ExtractionConfiguration object that works for many repos.
func DefaultConfig() io.Reader {
	return strings.NewReader(defaultConfig)
}
