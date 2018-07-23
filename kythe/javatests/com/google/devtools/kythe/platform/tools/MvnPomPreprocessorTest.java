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

package com.google.devtools.kythe.platform.tools;

import static com.google.common.truth.Truth.assertThat;

import java.io.File;
import java.io.FileInputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.w3c.dom.Document;
import org.w3c.dom.Node;

/** Tests {@link MvnPomProcessor}. */
@RunWith(JUnit4.class)
public class MvnPomPreprocessorTest {
  private static final String TEST_DATA_DIR =
      "kythe/javatests/com/google/devtools/kythe/platform/tools/testdata";

  @Test
  public void testQueryForNodeByXPathReturnsProperlyWithSingleMatch() throws Exception {
    Document testPomDocument = loadTestPomDocument("test_pom_with_plugin.xml");
    Node node =
        MvnPomPreprocessor.queryForNodeByXPath(
            testPomDocument, "/project/build/plugins/plugin[artifactId='maven-compiler-plugin']");
    assertThat(node).isNotNull();
    assertThat(node.getNodeName()).isEqualTo("plugin");
  }

  @Test
  public void testQueryForNodeByXPathReturnsProperlyWithNoMatch() throws Exception {
    Document testPomDocument = loadTestPomDocument("test_pom_without_plugin.xml");
    Node node =
        MvnPomPreprocessor.queryForNodeByXPath(
            testPomDocument, "/project/build/plugins/plugin[artifactId='maven-compiler-plugin']");
    assertThat(node).isNull();
  }

  @Test
  public void testEnsureSingletonPathElementsExistsProperlyCreatesElements() throws Exception {
    Document testPomDocument = loadTestPomDocument("test_pom_without_plugin.xml");
    Node node = MvnPomPreprocessor.queryForNodeByXPath(testPomDocument, "/project/build/plugins");
    assertThat(node).isNull();
    Node newNode =
        MvnPomPreprocessor.ensureSingletonPathElementsExist(
            testPomDocument, "/project/build/plugins");
    assertThat(newNode).isNotNull();
    assertThat(newNode.getNodeName()).isEqualTo("plugins");
    node = MvnPomPreprocessor.queryForNodeByXPath(testPomDocument, "/project/build/plugins");
    assertThat(node).isEqualTo(newNode);
  }

  @Test
  public void testEnsureSingletonPathElementsExistsProperlyReturnsExistingElements()
      throws Exception {
    Document testPomDocument = loadTestPomDocument("test_pom_with_plugin.xml");
    Node node = MvnPomPreprocessor.queryForNodeByXPath(testPomDocument, "/project/build/plugins");
    Node existingNode =
        MvnPomPreprocessor.ensureSingletonPathElementsExist(
            testPomDocument, "/project/build/plugins");
    assertThat(existingNode).isEqualTo(node);
  }

  @Test
  public void testAppendPomCompilerPluginFragementProperlyAppendsElements() throws Exception {
    Document testPomDocument = loadTestPomDocument("test_pom_without_plugin.xml");
    Node node =
        MvnPomPreprocessor.queryForNodeByXPath(
            testPomDocument, "/project/build/plugins/plugin[artifactId='maven-compiler-plugin']");
    assertThat(node).isNull();
    MvnPomPreprocessor.appendPomCompilerPluginFragment(testPomDocument);

    node =
        MvnPomPreprocessor.queryForNodeByXPath(
            testPomDocument, "/project/build/plugins/plugin[artifactId='maven-compiler-plugin']");
    assertThat(node).isNotNull();
  }

  @Test
  public void testLoadDOMDocumentReturnsProperly() throws Exception {
    Path workingDirPath = Paths.get(System.getProperty("user.dir"));
    Path testPomFilePath =
        Paths.get(workingDirPath.toString(), TEST_DATA_DIR, "test_pom_with_plugin.xml");
    Document pomDom =
        MvnPomPreprocessor.loadDOMDocument(
            new FileInputStream(new File(testPomFilePath.toString())));
    assertThat(pomDom).isNotNull();
    assertThat(pomDom.getChildNodes().getLength()).isNotEqualTo(0);
  }

  private static Document loadTestPomDocument(String testPomFileName) throws Exception {
    Path workingDirPath = Paths.get(System.getProperty("user.dir"));
    Path testPomFilePath = Paths.get(workingDirPath.toString(), TEST_DATA_DIR, testPomFileName);
    return MvnPomPreprocessor.loadDOMDocument(
        new FileInputStream(new File(testPomFilePath.toString())));
  }
}
