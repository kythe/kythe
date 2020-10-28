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

package modifier

import (
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/beevik/etree"
)

// PreProcessPomXML takes a pom.xml file and either verifies that it already has
// the bits necessary to specify a separate compiler on commandline, or adds
// functionality by dropping in a maven-compiler-plugin to the build.
//
// Note this potentially overwrites the input file, even if it returns an error,
// so make a copy beforehand if you need to keep the original.
func PreProcessPomXML(pomXMLFile string) error {
	doc := etree.NewDocument()
	err := doc.ReadFromFile(pomXMLFile)
	if err != nil {
		return fmt.Errorf("reading XML file %s: %v", pomXMLFile, err)
	}
	hasPlugin, err := hasCompilerPlugin(doc)
	if err != nil {
		return err
	}
	if hasPlugin {
		return nil
	}
	if err := insertCompilerPlugin(doc); err != nil {
		return fmt.Errorf("adding maven-compiler-plugin: %v", err)
	}
	f, err := os.OpenFile(pomXMLFile, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
	if err != nil {
		return fmt.Errorf("opening file: %v", err)
	}
	doc.Indent(2)
	doc.WriteTo(f)
	return f.Close()
}

// hasCompilerPLugin returns whether there is a maven-compiler-plugin specified
// already.  Additionally return whether or not we optimistically think that any
// specified maven-compiler-plugin will work for us, or if we can see obvious
// errors.
func hasCompilerPlugin(doc *etree.Document) (bool, error) {
	for _, p := range doc.FindElements("//project/build/plugins/plugin") {
		a := p.FindElement("artifactId")
		if a.Text() == "maven-compiler-plugin" {
			// Now, we either check for the existence of correctly-specified
			// elements for groupId, version, and
			// configuration/[source|target], or we check for the complete
			// absence of all those elements.  For the former we check
			// validity, and for the latter we just assume that there's some
			// well-formed parent pluginManagement specifying the values.
			if len(p.ChildElements()) == 1 {
				// Assume that there's some well-formed parent
				// pluginManagement specifying the values.
				// TODO(danielmoy): handle the case where pom.xml plugin is
				// incorrectly specified, by looking at all of the pom.xml
				// files in the whole project wholistically.  This might be
				// out of scope - we might insist they be fixed upstream.
				return true, nil
			}
			return true, wellformedCompilerPlugin(p)
		}
	}
	for _, p := range doc.FindElements("//project/build/pluginManagement/plugins/plugin") {
		a := p.FindElement("artifactId")
		if a.Text() == "maven-compiler-plugin" {
			// For pluginManagement, we do need a well-specified set of
			// sibling elements.
			return true, wellformedCompilerPlugin(p)
		}
	}

	// If we didn't find anything, we can proceed with modification.
	return false, nil
}

// wellformedCompilerPlugin checks to see if the plugin fits the minimum
// supported versions we've tested:
// <plugin>
//   <artifactId>maven-compiler-plugin</artifactId>
//   <version>3.7.0</version>
//   <configuration>
//     <source>1.8</source>
//     <target>1.8</target>
//   </configuration>
// </plugin>
// TODO(danielmoy): see if we can relax some of these restrictions.
func wellformedCompilerPlugin(el *etree.Element) error {
	v := el.FindElement("version")
	if v == nil {
		return fmt.Errorf("missing maven-compiler-plugin version")
	}
	vt, err := semvar(v.Text())
	if err != nil || vt.earlierThan(versiontuple{3, 7, 0}) {
		return fmt.Errorf("unsupported maven-compiler-plugin version: %s, need 3.7.0+", v.Text())
	}

	if err := checkJavaVersion(el, "source"); err != nil {
		return err
	}
	if err := checkJavaVersion(el, "target"); err != nil {
		return err
	}

	return nil
}

func checkJavaVersion(el *etree.Element, kind string) error {
	k := el.FindElement(fmt.Sprintf("configuration/%s", kind))
	if k == nil {
		return fmt.Errorf("mising %s", kind)
	}
	if v, err := javaVersion(k.Text()); err != nil || v.earlierThan(versiontuple{1, 7}) {
		// TODO(#3075): Should test an upper bound here too.
		return fmt.Errorf("unsupported %s version: %s, need 1.7+", kind, k.Text())
	}
	return nil
}

// Some container that supports semvar (3.7.0) and java versions (1.6) for
// comparison purposes only.
type versiontuple []int

func semvar(st string) (versiontuple, error) {
	return fromString(st, 3)
}

func javaVersion(st string) (versiontuple, error) {
	return fromString(st, 2)
}

func fromString(st string, size int) (versiontuple, error) {
	v := versiontuple{}
	bits := strings.Split(st, ".")
	if len(bits) != size {
		return v, fmt.Errorf("version should have %d parts separated by '.', got: %s", size, st)
	}
	for i := 0; i < size; i++ {
		id, err := strconv.Atoi(bits[i])
		if err != nil {
			return v, fmt.Errorf("version %s has malformed index %d: %v", st, i, err)
		}
		v = append(v, id)
	}
	return v, nil
}

func (vt versiontuple) earlierThan(other versiontuple) bool {
	// Version tuples aren't of the same length, probably shouldn't be
	// compared
	if len(vt) != len(other) {
		return false
	}
	for i := 0; i < len(vt); i++ {
		if vt[i] < other[i] {
			return true
		} else if vt[i] > other[i] {
			return false
		}
	}
	return false
}

// appendCompilerPlugin modifies a doc containing the root element of a pom.xml
// configuration, by injecting a custom Maven compiler <plugin> element into the
// plugin list for the top-level project.  This allows mvn install commands to
// specify --Dmaven.compiler.executable on commandline and use a separate
// compiler.
//
// Note this is unlike the build.gradle modification, where a separate
// commandline argument is not necessary.
//
// An example modification can be found in the tests.  We would expect
// testdata/other-pom.xml to be transformed into testdata/modified-pom.xml
func insertCompilerPlugin(doc *etree.Document) error {
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
		plugins = build.CreateElement("plugins")
	}
	newPlugin := plugins.CreateElement("plugin")
	groupID := newPlugin.CreateElement("groupId")
	groupID.SetText("org.apache.maven.plugins")
	artifactID := newPlugin.CreateElement("artifactId")
	artifactID.SetText("maven-compiler-plugin")
	version := newPlugin.CreateElement("version")
	version.SetText("3.7.0")
	configuration := newPlugin.CreateElement("configuration")
	source := configuration.CreateElement("source")
	source.SetText("1.8")
	target := configuration.CreateElement("target")
	target.SetText("1.8")
	return nil
}
