/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

import com.google.common.base.Preconditions;
import com.google.common.collect.Streams;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.java.Plugin.KytheNode;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph;
import com.google.devtools.kythe.platform.java.JavaCompilationDetails;
import com.google.devtools.kythe.platform.java.JavacAnalyzer;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.MetadataLoaders;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.tools.javac.api.JavacTaskImpl;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.util.Context;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.function.BiFunction;
import java.util.function.Supplier;
import java.util.logging.Level;
import java.util.stream.Collectors;
import javax.lang.model.element.Name;
import javax.tools.JavaFileObject;

/** {@link JavacAnalyzer} to emit Kythe nodes and edges. */
public class KytheJavacAnalyzer extends JavacAnalyzer {
  private static final long serialVersionUID = 7458181626939870354L;

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private final FactEmitter emitter;
  private final JavaIndexerConfig config;
  private final MetadataLoaders metadataLoaders;
  private final List<Supplier<Plugin>> plugins = new ArrayList<>();

  // should be set in analyzeCompilationUnit before any call to analyzeFile
  private JavaEntrySets entrySets;

  public KytheJavacAnalyzer(
      JavaIndexerConfig config,
      FactEmitter emitter,
      StatisticsCollector statistics,
      MetadataLoaders metadataLoaders) {
    super(statistics);
    Preconditions.checkArgument(emitter != null, "FactEmitter must be non-null");
    Preconditions.checkArgument(config != null, "IndexerConfig must be non-null");
    this.emitter = emitter;
    this.config = config;
    this.metadataLoaders = metadataLoaders;
  }

  /**
   * Register a {@link Plugin} to be run for each {@link JCCompilationUnit} analysis. Plugins are
   * executed in the same order they are registered and a new {@link Plugin} instance will be
   * requested from the given {@link Supplier} for each {@link JCCompilationUnit} to be analyzed.
   */
  public KytheJavacAnalyzer registerPlugin(Supplier<Plugin> plugin) {
    plugins.add(plugin);
    return this;
  }

  @Override
  public void analyzeCompilationUnit(JavaCompilationDetails details) throws AnalysisException {
    Preconditions.checkState(
        entrySets == null,
        "JavaEntrySets is non-null (analyzeCompilationUnit was called concurrently?)");
    if (config.getVerboseLogging()) {
      Streams.stream(details.getCompileErrors())
          .collect(Collectors.groupingBy(d -> Optional.ofNullable(d.getSource())))
          .forEach(
              (file, errs) -> {
                logger.at(levelFor(file)).log(
                    "javac compilation errors:\n%s",
                    errs.stream().map(Object::toString).collect(Collectors.joining("\n")));
              });
    }
    CompilationUnit compilation = details.getCompilationUnit();
    entrySets =
        new JavaEntrySets(
            getStatisticsCollector(),
            emitter,
            compilation.getVName(),
            compilation.getRequiredInputList(),
            config.getIgnoreVNamePaths(),
            config.getIgnoreVNameRoots(),
            config.getOverrideJdkCorpus());
    try {
      super.analyzeCompilationUnit(details);
    } finally {
      entrySets = null; // Ensure entrySets is cleared for error-checking
    }
  }

  @Override
  public void analyzeFile(JavaCompilationDetails details, CompilationUnitTree ast)
      throws AnalysisException {
    Preconditions.checkState(
        entrySets != null, "analyzeCompilationUnit must be called to analyze each file");
    if (!details.getJavac().isPresent()) {
      if (details.getAnalysisCrash().isPresent()) {
        throw new AnalysisException(
            "No javac in details while analyzing file: " + ast.getSourceFile().getName(),
            details.getAnalysisCrash().get());
      }
      throw new AnalysisException(
          "No javac in details while analyzing file: " + ast.getSourceFile().getName());
    }
    JavacTaskImpl javac = (JavacTaskImpl) details.getJavac().get();
    Context context = javac.getContext();
    JCCompilationUnit compilation = (JCCompilationUnit) ast;
    final Map<JCTree, Plugin.KytheNode> nodes = new HashMap<>();
    SourceText src = null;
    try {
      src = new SourceText(context, compilation, details.getEncoding());
      SignatureGenerator signatureGenerator =
          new SignatureGenerator(ast, context, config.getEmitJvmSignatures());
      KytheTreeScanner.emitEntries(
          context,
          getStatisticsCollector(),
          entrySets,
          signatureGenerator,
          compilation,
          (t, v) -> nodes.put(t, new KytheNodeImpl(v)),
          src,
          details.getFileManager(),
          metadataLoaders,
          config);
    } catch (Throwable e) {
      throw new AnalysisException("Exception analyzing file: " + ast.getSourceFile().getName(), e);
    }
    if (!plugins.isEmpty()) {
      Map<Symbol, Plugin.KytheNode> symNodes = new HashMap<>();
      for (Map.Entry<Symbol, VName> symVName : entrySets.getSymbolNodes().entrySet()) {
        symNodes.put(symVName.getKey(), new KytheNodeImpl(symVName.getValue()));
      }
      Plugin.KytheGraph graph =
          new KytheGraphImpl(
              context, entrySets, src.getPositions(), symNodes, Collections.unmodifiableMap(nodes));
      for (Supplier<Plugin> p : plugins) {
        try {
          Plugin plugin = p.get();
          plugin.run(compilation, entrySets, graph);
        } catch (Throwable e) {
          logger.atWarning().withCause(e).log("Error running plugins");
        }
      }
    }
  }

  private static class KytheGraphImpl implements Plugin.KytheGraph {
    private final Context javaContext;
    private final JavaEntrySets entrySets;
    private final SourceText.Positions filePositions;
    private final Map<JCTree, Plugin.KytheNode> treeNodes;
    private final Map<Symbol, Plugin.KytheNode> symNodes;

    KytheGraphImpl(
        Context javaContext,
        JavaEntrySets entrySets,
        SourceText.Positions filePositions,
        Map<Symbol, Plugin.KytheNode> symNodes,
        Map<JCTree, Plugin.KytheNode> treeNodes) {
      this.javaContext = javaContext;
      this.entrySets = entrySets;
      this.filePositions = filePositions;
      this.symNodes = symNodes;
      this.treeNodes = treeNodes;
    }

    @Override
    public Context getJavaContext() {
      return javaContext;
    }

    @Override
    public Optional<Plugin.KytheNode> getNode(JavaFileObject file) {
      return Optional.ofNullable(file).map(entrySets::getFileVName).map(KytheNodeImpl::new);
    }

    @Override
    public Optional<Plugin.KytheNode> getNode(JCTree tree) {
      return Optional.ofNullable(treeNodes.get(tree));
    }

    @Override
    public Optional<Plugin.KytheNode> getNode(Symbol sym) {
      return Optional.ofNullable(symNodes.get(sym));
    }

    @Override
    public Optional<Plugin.KytheNode> getJvmNode(Symbol sym) {
      CorpusPath corpusPath = entrySets.jvmCorpusPath(sym);
      switch (sym.getKind()) {
        case CLASS:
        case ENUM:
        case INTERFACE:
          return referenceJvmType(sym)
              .map(t -> JvmGraph.getReferenceVName(corpusPath, t))
              .map(KytheNodeImpl::new);
        case METHOD:
        case CONSTRUCTOR:
          return forMethodAndEnclosingClass(
              sym,
              (method, enclosingClass) ->
                  JvmGraph.getMethodVName(
                      corpusPath, enclosingClass, sym.getSimpleName().toString(), method));
        case FIELD:
          return referenceJvmType(sym.enclClass())
              .map(
                  classType ->
                      JvmGraph.getFieldVName(corpusPath, classType, sym.getSimpleName().toString()))
              .map(KytheNodeImpl::new);

        case PARAMETER:
          MethodSymbol enclosingMethod = (MethodSymbol) sym.getEnclosingElement();
          return forMethodAndEnclosingClass(
              enclosingMethod,
              (methodType, enclosingClass) ->
                  JvmGraph.getParameterVName(
                      corpusPath,
                      enclosingClass,
                      enclosingMethod.getSimpleName().toString(),
                      methodType,
                      enclosingMethod.getParameters().indexOf(sym)));

          // TODO(schroederc): other ElementKinds
        default:
          return Optional.empty();
      }
    }

    private Optional<JvmGraph.Type.ReferenceType> referenceJvmType(Symbol symbol) {
      return Optional.ofNullable(symbol)
          .map(Symbol::asType)
          .map(KytheTreeScanner::toJvmType)
          .map(JvmGraph.Type.ReferenceType.class::cast);
    }

    private Optional<KytheNode> forMethodAndEnclosingClass(
        Symbol methodSymbol,
        BiFunction<JvmGraph.Type.MethodType, JvmGraph.Type.ReferenceType, VName>
            methodAndEnclosingClassFunction) {
      return Optional.ofNullable(methodSymbol.asType())
          .map(Type::asMethodType)
          .map(KytheTreeScanner::toMethodJvmType)
          .flatMap(
              methodType ->
                  referenceJvmType(methodSymbol.enclClass())
                      .map(
                          enclosingClass ->
                              methodAndEnclosingClassFunction.apply(methodType, enclosingClass)))
          .map(KytheNodeImpl::new);
    }

    @Override
    public Optional<Span> getSpan(JCTree tree) {
      return validSpan(filePositions.getSpan(tree));
    }

    @Override
    public Optional<Span> findIdentifier(Name name, int startOffset) {
      return validSpan(filePositions.findIdentifier(name, startOffset));
    }

    private static Optional<Span> validSpan(Span s) {
      return Optional.ofNullable(s).filter(Span::isValid);
    }
  }

  private static final class KytheNodeImpl implements Plugin.KytheNode {
    private final VName vName;

    KytheNodeImpl(VName vName) {
      this.vName = vName;
    }

    @Override
    public VName getVName() {
      return vName;
    }

    @Override
    public boolean equals(Object o) {
      return o instanceof KytheNodeImpl && Objects.equals(this.vName, ((KytheNodeImpl) o).vName);
    }

    @Override
    public int hashCode() {
      return Objects.hashCode(vName);
    }

    @Override
    public String toString() {
      return "KytheNode{" + vName.toString().replace("\n", " ").trim() + "}";
    }
  }

  private static Level levelFor(Optional<JavaFileObject> file) {
    return file.map(
            f ->
                f.isNameCompatible("module-info", JavaFileObject.Kind.SOURCE)
                    ? Level.INFO
                    : Level.WARNING)
        .orElse(Level.WARNING);
  }
}
