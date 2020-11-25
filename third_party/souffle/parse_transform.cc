/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

// This code was originally in <souffle root>/src/main.cpp.

#include "glog/logging.h"
#include "third_party/souffle/parse_transform.h"

#include "Global.h"
#include "ast/Node.h"
#include "ast/Program.h"
#include "ast/TranslationUnit.h"
#include "ast/analysis/PrecedenceGraph.h"
#include "ast/analysis/SCCGraph.h"
#include "ast/analysis/Type.h"
#include "ast/transform/ADTtoRecords.h"
#include "ast/transform/AddNullariesToAtomlessAggregates.h"
#include "ast/transform/ComponentChecker.h"
#include "ast/transform/ComponentInstantiation.h"
#include "ast/transform/Conditional.h"
#include "ast/transform/ExecutionPlanChecker.h"
#include "ast/transform/Fixpoint.h"
#include "ast/transform/FoldAnonymousRecords.h"
#include "ast/transform/GroundWitnesses.h"
#include "ast/transform/GroundedTermsChecker.h"
#include "ast/transform/IOAttributes.h"
#include "ast/transform/IODefaults.h"
#include "ast/transform/InlineRelations.h"
#include "ast/transform/MagicSet.h"
#include "ast/transform/MaterializeAggregationQueries.h"
#include "ast/transform/MaterializeSingletonAggregation.h"
#include "ast/transform/MinimiseProgram.h"
#include "ast/transform/NameUnnamedVariables.h"
#include "ast/transform/NormaliseMultiResultFunctors.h"
#include "ast/transform/PartitionBodyLiterals.h"
#include "ast/transform/Pipeline.h"
#include "ast/transform/PolymorphicObjects.h"
#include "ast/transform/PragmaChecker.h"
#include "ast/transform/Provenance.h"
#include "ast/transform/ReduceExistentials.h"
#include "ast/transform/RemoveBooleanConstraints.h"
#include "ast/transform/RemoveEmptyRelations.h"
#include "ast/transform/RemoveRedundantRelations.h"
#include "ast/transform/RemoveRedundantSums.h"
#include "ast/transform/RemoveRelationCopies.h"
#include "ast/transform/RemoveTypecasts.h"
#include "ast/transform/ReorderLiterals.h"
#include "ast/transform/ReplaceSingletonVariables.h"
#include "ast/transform/ResolveAliases.h"
#include "ast/transform/ResolveAnonymousRecordAliases.h"
#include "ast/transform/SemanticChecker.h"
#include "ast/transform/SimplifyAggregateTargetExpression.h"
#include "ast/transform/UniqueAggregationVariables.h"
#include "ast2ram/AstToRamTranslator.h"
#include "interpreter/Engine.h"
#include "interpreter/ProgInterface.h"
#include "parser/ParserDriver.h"
#include "ram/Node.h"
#include "ram/Program.h"
#include "ram/TranslationUnit.h"
#include "ram/transform/ChoiceConversion.h"
#include "ram/transform/CollapseFilters.h"
#include "ram/transform/Conditional.h"
#include "ram/transform/EliminateDuplicates.h"
#include "ram/transform/ExpandFilter.h"
#include "ram/transform/HoistAggregate.h"
#include "ram/transform/HoistConditions.h"
#include "ram/transform/IfConversion.h"
#include "ram/transform/IndexedInequality.h"
#include "ram/transform/Loop.h"
#include "ram/transform/MakeIndex.h"
#include "ram/transform/Parallel.h"
#include "ram/transform/ReorderConditions.h"
#include "ram/transform/ReorderFilterBreak.h"
#include "ram/transform/ReportIndex.h"
#include "ram/transform/Sequence.h"
#include "ram/transform/Transformer.h"
#include "ram/transform/TupleId.h"
#include "reports/DebugReport.h"
#include "reports/ErrorReport.h"
#include "souffle/RamTypes.h"
#include "souffle/profile/Tui.h"
#include "souffle/provenance/Explain.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/FileUtil.h"
#include "souffle/utility/MiscUtil.h"
#include "souffle/utility/StreamUtil.h"
#include "souffle/utility/StringUtil.h"
#include "synthesiser/Synthesiser.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace souffle {

std::unique_ptr<ram::TranslationUnit> ParseTransform(const std::string& code) {
  Global::config().set("jobs", "1");
  Global::config().has("verbose", "1");
  souffle::ErrorReport errorReport(/*nowarn=*/true);
  souffle::DebugReport debugReport;
  souffle::Own<souffle::ast::TranslationUnit> astTranslationUnit =
      souffle::ParserDriver::parseTranslationUnit(code, errorReport, debugReport);
  auto& report = astTranslationUnit->getErrorReport();
  if (report.getNumIssues() != 0) {
      LOG(ERROR) << "Souffle parser reported a nonzero diagnostic count.";
      std::stringstream diagnostics;
      diagnostics << report;
      LOG(ERROR) << diagnostics.str();
      return nullptr;
  }

  // ------- rewriting / optimizations -------------

  /* set up additional global options based on pragma declaratives */
  (mk<ast::transform::PragmaChecker>())->apply(*astTranslationUnit);

  /* construct the transformation pipeline */

  // Equivalence pipeline
  auto equivalencePipeline =
          mk<ast::transform::PipelineTransformer>(mk<ast::transform::NameUnnamedVariablesTransformer>(),
                  mk<ast::transform::FixpointTransformer>(mk<ast::transform::MinimiseProgramTransformer>()),
                  mk<ast::transform::ReplaceSingletonVariablesTransformer>(),
                  mk<ast::transform::RemoveRelationCopiesTransformer>(),
                  mk<ast::transform::RemoveEmptyRelationsTransformer>(),
                  mk<ast::transform::RemoveRedundantRelationsTransformer>());

  // Magic-Set pipeline
  auto magicPipeline = mk<ast::transform::PipelineTransformer>(mk<ast::transform::MagicSetTransformer>(),
          mk<ast::transform::ResolveAliasesTransformer>(),
          mk<ast::transform::RemoveRelationCopiesTransformer>(),
          mk<ast::transform::RemoveEmptyRelationsTransformer>(),
          mk<ast::transform::RemoveRedundantRelationsTransformer>(), souffle::clone(equivalencePipeline));

  // Partitioning pipeline
  auto partitionPipeline =
          mk<ast::transform::PipelineTransformer>(mk<ast::transform::NameUnnamedVariablesTransformer>(),
                  mk<ast::transform::PartitionBodyLiteralsTransformer>(),
                  mk<ast::transform::ReplaceSingletonVariablesTransformer>());

  // Provenance pipeline
  auto provenancePipeline = mk<ast::transform::ConditionalTransformer>(Global::config().has("provenance"),
          mk<ast::transform::PipelineTransformer>(mk<ast::transform::ProvenanceTransformer>(),
                  mk<ast::transform::PolymorphicObjectsTransformer>()));

  // Main pipeline
  auto pipeline = mk<ast::transform::PipelineTransformer>(mk<ast::transform::ComponentChecker>(),
          mk<ast::transform::ComponentInstantiationTransformer>(),
          mk<ast::transform::IODefaultsTransformer>(),
          mk<ast::transform::SimplifyAggregateTargetExpressionTransformer>(),
          mk<ast::transform::UniqueAggregationVariablesTransformer>(),
          mk<ast::transform::FixpointTransformer>(mk<ast::transform::PipelineTransformer>(
                  mk<ast::transform::ResolveAnonymousRecordAliasesTransformer>(),
                  mk<ast::transform::FoldAnonymousRecords>())),
          mk<ast::transform::PolymorphicObjectsTransformer>(), mk<ast::transform::SemanticChecker>(),
          mk<ast::transform::ADTtoRecordsTransformer>(), mk<ast::transform::GroundWitnessesTransformer>(),
          mk<ast::transform::UniqueAggregationVariablesTransformer>(),
          mk<ast::transform::NormaliseMultiResultFunctorsTransformer>(),
          mk<ast::transform::MaterializeSingletonAggregationTransformer>(),
          mk<ast::transform::FixpointTransformer>(
                  mk<ast::transform::MaterializeAggregationQueriesTransformer>()),
          mk<ast::transform::ResolveAliasesTransformer>(), mk<ast::transform::RemoveTypecastsTransformer>(),
          mk<ast::transform::RemoveBooleanConstraintsTransformer>(),
          mk<ast::transform::ResolveAliasesTransformer>(), mk<ast::transform::MinimiseProgramTransformer>(),
          mk<ast::transform::InlineRelationsTransformer>(),
          mk<ast::transform::PolymorphicObjectsTransformer>(), mk<ast::transform::GroundedTermsChecker>(),
          mk<ast::transform::ResolveAliasesTransformer>(),
          mk<ast::transform::RemoveRedundantRelationsTransformer>(),
          mk<ast::transform::RemoveRelationCopiesTransformer>(),
          mk<ast::transform::RemoveEmptyRelationsTransformer>(),
          mk<ast::transform::ReplaceSingletonVariablesTransformer>(),
          mk<ast::transform::FixpointTransformer>(mk<ast::transform::PipelineTransformer>(
                  mk<ast::transform::ReduceExistentialsTransformer>(),
                  mk<ast::transform::RemoveRedundantRelationsTransformer>())),
          mk<ast::transform::RemoveRelationCopiesTransformer>(), std::move(partitionPipeline),
          std::move(equivalencePipeline), mk<ast::transform::RemoveRelationCopiesTransformer>(),
          std::move(magicPipeline), mk<ast::transform::ReorderLiteralsTransformer>(),
          mk<ast::transform::RemoveRedundantSumsTransformer>(),
          mk<ast::transform::RemoveEmptyRelationsTransformer>(),
          mk<ast::transform::AddNullariesToAtomlessAggregatesTransformer>(),
          mk<ast::transform::PolymorphicObjectsTransformer>(),
          mk<ast::transform::ReorderLiteralsTransformer>(), mk<ast::transform::ExecutionPlanChecker>(),
          std::move(provenancePipeline), mk<ast::transform::IOAttributesTransformer>());

  // Disable unwanted transformations
  if (Global::config().has("disable-transformers")) {
      std::vector<std::string> givenTransformers =
              splitString(Global::config().get("disable-transformers"), ',');
      pipeline->disableTransformers(
              std::set<std::string>(givenTransformers.begin(), givenTransformers.end()));
  }

#if 0
  // Set up the debug report if necessary
  if (Global::config().has("debug-report")) {
      auto parser_end = std::chrono::high_resolution_clock::now();
      std::stringstream ss;

      // Add current time
      std::time_t time = std::time(nullptr);
      ss << "Executed at ";
      ss << std::put_time(std::localtime(&time), "%F %T") << "\n";

      // Add config
      ss << "(\n";
      ss << join(Global::config().data(), ",\n", [](std::ostream& out, const auto& arg) {
          out << "  \"" << arg.first << "\" -> \"" << arg.second << '"';
      });
      ss << "\n)";

      debugReport.addSection("Configuration", "Configuration", ss.str());

      // Add parsing runtime
      std::string runtimeStr =
              "(" + std::to_string(std::chrono::duration<double>(parser_end - parser_start).count()) + "s)";
      debugReport.addSection("Parsing", "Parsing " + runtimeStr, "");

      pipeline->setDebugReport();
  }
#endif

  // Toggle pipeline verbosity
  pipeline->setVerbosity(Global::config().has("verbose"));

  // Apply all the transformations
  pipeline->apply(*astTranslationUnit);

  if (Global::config().has("show")) {
      // Output the transformed datalog and return
      if (Global::config().get("show") == "transformed-datalog") {
          std::cout << astTranslationUnit->getProgram() << std::endl;
          return 0;
      }

      // Output the precedence graph in graphviz dot format and return
      if (Global::config().get("show") == "precedence-graph") {
          astTranslationUnit->getAnalysis<ast::analysis::PrecedenceGraphAnalysis>()->print(std::cout);
          std::cout << std::endl;
          return 0;
      }

      // Output the scc graph in graphviz dot format and return
      if (Global::config().get("show") == "scc-graph") {
          astTranslationUnit->getAnalysis<ast::analysis::SCCGraphAnalysis>()->print(std::cout);
          std::cout << std::endl;
          return 0;
      }

      // Output the type analysis
      if (Global::config().get("show") == "type-analysis") {
          astTranslationUnit->getAnalysis<ast::analysis::TypeAnalysis>()->print(std::cout);
          std::cout << std::endl;
          return 0;
      }
  }

  // ------- execution -------------
  /* translate AST to RAM */
  debugReport.startSection();
  auto ramTranslationUnit = ast2ram::AstToRamTranslator().translateUnit(*astTranslationUnit);
  debugReport.endSection("ast-to-ram", "Translate AST to RAM");

  // Apply RAM transforms
  {
      using namespace ram::transform;
      Own<Transformer> ramTransform = mk<TransformerSequence>(
              mk<LoopTransformer>(mk<TransformerSequence>(mk<ExpandFilterTransformer>(),
                      mk<HoistConditionsTransformer>(), mk<MakeIndexTransformer>())),
              mk<LoopTransformer>(mk<IndexedInequalityTransformer>()), mk<IfConversionTransformer>(),
              mk<ChoiceConversionTransformer>(), mk<CollapseFiltersTransformer>(), mk<TupleIdTransformer>(),
              mk<LoopTransformer>(
                      mk<TransformerSequence>(mk<HoistAggregateTransformer>(), mk<TupleIdTransformer>())),
              mk<ExpandFilterTransformer>(), mk<HoistConditionsTransformer>(),
              mk<CollapseFiltersTransformer>(), mk<EliminateDuplicatesTransformer>(),
              mk<ReorderConditionsTransformer>(), mk<LoopTransformer>(mk<ReorderFilterBreak>()),
              mk<ConditionalTransformer>(
                      // job count of 0 means all cores are used.
                      []() -> bool { return std::stoi(Global::config().get("jobs")) != 1; },
                      mk<ParallelTransformer>()),
              mk<ReportIndexTransformer>());

      ramTransform->apply(*ramTranslationUnit);
  }

  if (ramTranslationUnit->getErrorReport().getNumIssues() != 0) {
      std::cerr << ramTranslationUnit->getErrorReport();
  }

  return ramTranslationUnit;
}

}
