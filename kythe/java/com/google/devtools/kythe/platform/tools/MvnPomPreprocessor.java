/*
 * Copyright 2018 Google Inc. All rights reserved.
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

import static java.nio.charset.StandardCharsets.UTF_8;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Splitter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.Writer;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.util.stream.Collectors;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

/**
 * Pre-processes MVN POM files to ensure the required compiler plugin is configured in order to
 * successfully hook javac for extraction.
 *
 * <p>Usage: java -jar mvn_pom_preprocessor_deploy.jar -pom <pom_file_path>
 */
public class MvnPomPreprocessor {
  @Parameter(
    names = {"-pom"},
    description = "The POM file to pre-process",
    required = true
  )
  private String pomFilePath;

  public static void main(String[] args) {
    MvnPomPreprocessor mvnPomPreprocessor = new MvnPomPreprocessor();
    JCommander jc = new JCommander(mvnPomPreprocessor);
    jc.parse(args);
    mvnPomPreprocessor.run();
  }

  private void run() {
    // Load the POM file into DOM
    Document pomDom = null;
    try {
      pomDom = loadDOMDocument(new FileInputStream(new File(pomFilePath)));
    } catch (FileNotFoundException e) {
      throw new IllegalArgumentException("Failed to find specified POM file", e);
    }

    // check to see if there is an existing maven-compiler-plugin, if so nothing needs to be done
    if (queryForNodeByXPath(
            pomDom, "/project/build/plugins/plugin[artifactId='maven-compiler-plugin']")
        != null) {
      return;
    }

    // otherwise, insert a default compiler plugin to allow for the javac-extractor shim
    appendPomCompilerPluginFragment(pomDom);

    // write the modified POM file
    try (Writer writer = Files.newBufferedWriter(Paths.get(pomFilePath), UTF_8)) {
      DOMSource source = new DOMSource(pomDom);
      StreamResult result = new StreamResult(writer);
      TransformerFactory transformerFactory = TransformerFactory.newInstance();
      Transformer transformer = transformerFactory.newTransformer();
      transformer.transform(source, result);
      writer.flush();
      writer.close();
    } catch (IOException | TransformerException e) {
      throw new IllegalStateException("Failed to write pre-processed POM file", e);
    }
  }

  /**
   * Queries the specified DOM document for a single node corresponding to the specified XPath
   * query. Assumes the specified XPath query resolves to a single node.
   *
   * @param dom The DOM document to be queried.
   * @param xpathQuery The XPath query to be executed.
   * @return A single node matching the query.
   */
  @VisibleForTesting
  static Node queryForNodeByXPath(Document dom, String xpathQuery) {
    XPath xpath = XPathFactory.newInstance().newXPath();
    try {
      return (Node) xpath.compile(xpathQuery).evaluate(dom, XPathConstants.NODE);
    } catch (XPathExpressionException e) {
      throw new IllegalArgumentException("Failed xpath query", e);
    }
  }

  /**
   * Loads a DOM document from the specified input stream.
   *
   * @param inputStream The input stream to be parsed.
   * @return A loaded DOM document.
   */
  @VisibleForTesting
  static Document loadDOMDocument(InputStream inputStream) {
    try {
      DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance();
      domFactory.setIgnoringComments(true);
      DocumentBuilder domBuilder = domFactory.newDocumentBuilder();
      return domBuilder.parse(inputStream);
    } catch (IOException | SAXException | ParserConfigurationException e) {
      throw new IllegalArgumentException("Failed to parse test DOM file", e);
    }
  }

  /**
   * Ensures the singleton path elements exists within the specified DOM document, for the specified
   * child element XPath. Will create those path elements if they don't already exists. Assumes the
   * specified XPath corresponds to a singleton sub-tree of the document.
   *
   * @return The node of the child element corresponding to the specified XPath.
   */
  @VisibleForTesting
  static Node ensureSingletonPathElementsExist(Document dom, String childElementXPath) {
    Node node = queryForNodeByXPath(dom, childElementXPath);
    if (node == null) {
      List<String> pathElements = Splitter.on("/").splitToList(childElementXPath);
      String childElement = pathElements.get(pathElements.size() - 1);
      List<String> parentElements = pathElements.subList(0, pathElements.size() - 1);
      Node parentNode =
          ensureSingletonPathElementsExist(
              dom, parentElements.stream().collect(Collectors.joining("/")));
      node = dom.createElement(childElement);
      parentNode.appendChild(node);
    }

    return node;
  }

  /**
   * Appends a compiler plugin fragement to the specified DOM document.
   *
   * @param dom The DOM document being appended to.
   */
  @VisibleForTesting
  static void appendPomCompilerPluginFragment(Document dom) {
    Node compilerPluginFragment =
        loadDOMDocument(
                MvnPomPreprocessor.class
                    .getClassLoader()
                    .getResourceAsStream(
                        "com/google/devtools/kythe/platform/tools/pom_compiler_plugin_fragment.xml"))
            .getDocumentElement();
    Node pluginsNode = ensureSingletonPathElementsExist(dom, "/project/build/plugins");
    compilerPluginFragment = dom.importNode(compilerPluginFragment, true);
    pluginsNode.appendChild(compilerPluginFragment);
  }
}
