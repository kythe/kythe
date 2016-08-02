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

import com.beust.jcommander.Parameter;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.GRPCCompilationAnalyzer;
import com.google.devtools.kythe.platform.java.JavacAnalysisDriver;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import io.grpc.netty.NettyServerBuilder;
import java.io.IOException;

/** Binary to run Kythe's Java indexer as a CompilationAnalyzer service. */
public class JavaIndexerServer {
  public static void main(String[] args) {
    ServerConfig config = new ServerConfig();
    config.parseCommandLine(args);

    try {
      NettyServerBuilder.forPort(config.getPort())
          .addService(new JavaCompilationAnalyzer(config))
          .build()
          .start()
          .awaitTermination();
    } catch (InterruptedException | IOException e) {
      e.printStackTrace();
      System.exit(1);
    }
    System.err.println("Started Java CompilationAnalyzer server on port " + config.getPort());
  }

  private static class JavaCompilationAnalyzer extends GRPCCompilationAnalyzer {
    private final JavacAnalysisDriver driver = new JavacAnalysisDriver();
    private final IndexerConfig config;

    public JavaCompilationAnalyzer(IndexerConfig config) {
      this.config = config;
    }

    @Override
    public void analyzeCompilation(
        CompilationUnit compilation, FileDataProvider fileData, FactEmitter emitter)
        throws AnalysisException {
      driver.analyze(
          new KytheJavacAnalyzer(config, emitter, getStatisticsCollector()),
          compilation,
          fileData,
          false);
    }
  }

  private static class ServerConfig extends IndexerConfig {
    @Parameter(
      names = {"-p", "--port"},
      required = true,
      description = "Port for GRPC listening server"
    )
    private int port;

    public ServerConfig() {
      super("java-indexer-server");
    }

    public final int getPort() {
      return port;
    }
  }
}
