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
package mvn // import "kythe.io/kythe/go/extractors/config/base/mvn"

// DefaultConfig describes a kythe.proto.ExtractionConfiguration object for a
// basic Maven repository, including the extractor entrypoint using a pom.xml
// file located at the top level of the repo.
const DefaultConfig = `{
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
      "uri": "gcr.io/kythe-public/kythe-javac-extractor-artifacts",
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
        },
        {
          "source": "/opt/kythe/extractors/javac9_tools.jar"
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
        },
        {
          "name": "KYTHE_OUTPUT_FILE",
          "value": "$KYTHE_OUTPUT_DIRECTORY/extractor-output.kzip"
        },
        {
          "name": "KYTHE_JAVA_RUNTIME_OPTIONS",
          "value": "-Xbootclasspath/p:/opt/kythe/extractors/javac9_tools.jar"
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
  "entry_point": ["/opt/kythe/extractors/runextractor", "maven", "-pom_xml", "pom.xml", "-javac_wrapper", "/opt/kythe/extractors/javac-wrapper.sh"]
}`
