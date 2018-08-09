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

package mavencmd

import (
	"fmt"
	"log"
	"os"

	"github.com/beevik/etree"
)

// PreProcessPomXML takes a pom.xml file and either verifies that it already has
// the bits necessary to specify a separate compiler on commandline, or adds
// functionality by dropping in a maven-compiler-plugin to the build.
//
// Note this potentially modifies the input file, so make a copy beforehand if
// you need to keep the original.
func PreProcessPomXML(pomXMLFile string) error {
	log.Printf("Reading file %s", pomXMLFile)
	doc := etree.NewDocument()
	err := doc.ReadFromFile(pomXMLFile)
	if err != nil {
		return fmt.Errorf("reading XML file %s: %v", pomXMLFile, err)
	}
	if hasCompilerPlugin(doc) {
		return nil
	}
	if err := appendCompilerPlugin(doc); err != nil {
		return err
	}
	f, err := os.OpenFile(pomXMLFile, os.O_APPEND|os.O_WRONLY, os.ModeAppend)
	if err != nil {
		return fmt.Errorf("opening file %s for append: %v", pomXMLFile, err)
	}
	return f.Close()
}

func hasCompilerPlugin(doc *etree.Document) bool {
	for _, p := range doc.FindElements("//project/build/plugins/plugin/artifactId") {
		if p.Text() == "maven-compiler-plugin" {
			return true
		}
	}
	return false
}

func appendCompilerPlugin(doc *etree.Document) error {
	project := doc.SelectElement("project")
	if project == nil {
		return fmt.Errorf("no top level <project> element")
	}
	build := project.SelectElement("build")
	if build == nil {
		build = project.CreateElement("build")
	}
	plugins := build.SelectElement("plugins")
	if plugins == nil {
		log.Printf("Adding new plugins element")
		plugins = project.CreateElement("plugins")
	}
	log.Printf("Adding new plugin element")
	newPlugin := plugins.CreateElement("plugin")
	foo := newPlugin.CreateElement("wtffoo")
	foo.SetText("Why does this not appear?")
	log.Printf("Adding new groupId element")
	groupID := newPlugin.CreateElement("groupId")
	groupID.SetText("org.apache.maven.plugins")
	artifactID := newPlugin.CreateElement("artifactId")
	artifactID.SetText("maven-compiler-plugin")
	version := newPlugin.CreateElement("version")
	version.SetText("3.7.0")
	log.Printf("Version text %s", version.Text())
	configuration := newPlugin.CreateElement("configuration")
	source := configuration.CreateElement("source")
	source.SetText("1.8")
	target := configuration.CreateElement("target")
	target.SetText("1.8")
	return nil
}
