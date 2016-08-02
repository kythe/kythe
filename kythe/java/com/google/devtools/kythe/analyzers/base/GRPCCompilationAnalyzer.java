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

package com.google.devtools.kythe.analyzers.base;

import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.platform.shared.RemoteFileData;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.AnalysisOutput;
import com.google.devtools.kythe.proto.Analysis.AnalysisRequest;
import com.google.devtools.kythe.proto.CompilationAnalyzerGrpc.CompilationAnalyzerImplBase;
import io.grpc.BindableService;
import io.grpc.ServerServiceDefinition;
import io.grpc.stub.StreamObserver;

/** GRPC-based {@link AbstractCompilationAnalyzer} implementation. */
public abstract class GRPCCompilationAnalyzer extends AbstractCompilationAnalyzer
    implements BindableService {
  public GRPCCompilationAnalyzer() {
    super();
  }

  public GRPCCompilationAnalyzer(StatisticsCollector statistics) {
    super(statistics);
  }

  private final CompilationAnalyzerImplBase compilationAnalyzerImpl =
      new CompilationAnalyzerImplBase() {
        @Override
        public void analyze(AnalysisRequest req, StreamObserver<AnalysisOutput> stream) {
          try {
            analyzeRequest(req, new StreamEmitter(stream));
          } catch (Throwable t) {
            stream.onError(t);
            return;
          }
          stream.onCompleted();
        }
      };

  @Override
  public ServerServiceDefinition bindService() {
    return compilationAnalyzerImpl.bindService();
  }

  @Override
  protected FileDataProvider parseFileDataService(String fileDataService) {
    return new RemoteFileData(fileDataService);
  }

  private static class StreamEmitter extends AnalysisOutputEmitter {
    private final StreamObserver<AnalysisOutput> stream;

    public StreamEmitter(StreamObserver<AnalysisOutput> stream) {
      this.stream = stream;
    }

    @Override
    public void emitOutput(AnalysisOutput output) {
      stream.onNext(output);
    }
  }
}
