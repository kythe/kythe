/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.java;

import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.GRPCCompilationAnalyzer;
import com.google.devtools.kythe.platform.java.JavacAnalysisDriver;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.CompilationAnalyzerGrpc;

import io.grpc.transport.netty.NettyServerBuilder;

import java.io.IOException;

/** Binary to run Kythe's Java indexer as a CompilationAnalyzer service. */
public class JavaIndexerServer {
  public static void main(String[] args) throws IOException {
    if (args.length > 0 && ("--help".equals(args[0]) || "-h".equals(args[0]))) {
      usage(0);
    } else if (args.length > 2) {
      System.err.println("ERROR: too many arguments: " + java.util.Arrays.toString(args));
      usage(1);
    }

    String portArg;
    switch (args.length) {
      case 2:
        if (!"--port".equals(args[0])) {
          System.err.println("ERROR: invalid flag " + args[0]);
          usage(1);
        }
        portArg = args[1];
        break;
      case 1:
        if (!args[0].startsWith("--port=")) {
          System.err.println("ERROR: invalid flag " + args[0]);
          usage(1);
        }
        portArg = args[0].substring("--port=".length());
        break;
      default:
        System.err.println("ERROR: missing --port flag");
        usage(1);
        return;
    }

    int port = Integer.parseInt(portArg);
    NettyServerBuilder.forPort(port)
        .addService(CompilationAnalyzerGrpc.bindService(new JavaCompilationAnalyzer()))
        .build()
        .start();
    System.err.println("Started Java CompilationAnalyzer server on port " + port);
  }

  private static void usage(int exitCode) {
    System.err.println("usage: java_indexer_server --port=number");
    System.exit(exitCode);
  }

  private static class JavaCompilationAnalyzer extends GRPCCompilationAnalyzer {
    private final JavacAnalysisDriver driver = new JavacAnalysisDriver();

    @Override
    public void analyzeCompilation(CompilationUnit compilation,
        FileDataProvider fileData, FactEmitter emitter) throws AnalysisException {
      driver.analyze(new KytheJavacAnalyzer(emitter, getStatisticsCollector()),
          compilation, fileData, false);
    }
  }
}
