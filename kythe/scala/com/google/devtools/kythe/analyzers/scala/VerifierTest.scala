/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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
package com.google.devtools.kythe.analyzers.scala

import java.io.File
import java.nio.file.Paths

import org.scalatest.{FlatSpec, Matchers}
import third_party.utils.src.test.io.bazel.rulesscala.utils.TestUtil

import scala.io.Source
import scala.sys.process.Process
import scala.tools.nsc.{CompilerCommand, Global, Settings}
import collection.JavaConverters._

class VerifierTest extends FlatSpec with Matchers {
  private val baseDir = System.getProperty("user.dir")
  private def pathOf(jvmFlag: String) = {
    val jar = System.getProperty(jvmFlag)
    val libPath = Paths.get(baseDir, jar).toAbsolutePath
    libPath.toString
  }
  "Files" should "be verified" in {
    val settings = new Settings()
    val command = new CompilerCommand(
      List(
        "-classpath",
        pathOf("scala.library.location"),
        "-Yrangepos",
        "-Xplugin:kythe/scala/com/google/devtools/kythe/analyzers/scala/kythe-plugin.jar"
      ),
      settings
    )
    val global = new Global(command.settings)
    val filesToTest =
      new File(
        "kythe/scala/com/google/devtools/kythe/analyzers/scala/testdata/verified"
      ).listFiles()
    val verifierBinaryLocationProperty = System.getProperty("verifier.location")
    val dedupBinaryLocationProperty = System.getProperty("dedup.location")
    // Bazel passes us the path starting from bazel-out but we want the location relative to the
    // current binary.
    val binaryStart =
      verifierBinaryLocationProperty.indexOf("kythe/cxx/verifier")
    val verifierBinaryLocation =
      verifierBinaryLocationProperty.substring(binaryStart)
    val dedupBinaryLocation = dedupBinaryLocationProperty.substring(binaryStart)
    for (fileToTest <- filesToTest) {
      System.out.println("STARTING TEST FOR " + fileToTest)
      new global.Run()
        .compile(List(fileToTest.toString))
      val code = (Process(
        "cat" :: new File("./index")
          .listFiles()
          .toList
          .map((f) => f.getAbsoluteFile.toString)
      ) #|
        Process(
          Seq(
            dedupBinaryLocation
          )
        ) #|
        Process(
          Seq(
            verifierBinaryLocation,
            "--show_protos",
            "--show_goals",
            fileToTest.toString
          )
        )).!
      code should equal(0)
    }
  }
}
