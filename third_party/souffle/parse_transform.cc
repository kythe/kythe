/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

// This code was originally in <souffle root>/src/main.cpp.

#include "absl/log/log.h"
#include "third_party/souffle/parse_transform.h"

#include "Global.h"
#include "ast/Clause.h"
#include "ast/Node.h"
#include "ast/Program.h"
#include "ast/TranslationUnit.h"
#include "ast/analysis/PrecedenceGraph.h"
#include "ast/analysis/SCCGraph.h"
#include "ast/analysis/typesystem/Type.h"
#include "ast/transform/AddNullariesToAtomlessAggregates.h"
#include "ast/transform/ComponentChecker.h"
#include "ast/transform/ComponentInstantiation.h"
#include "ast/transform/Conditional.h"
#include "ast/transform/ExecutionPlanChecker.h"
#include "ast/transform/ExpandEqrels.h"
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
#include "ast/transform/NormaliseGenerators.h"
#include "ast/transform/PartitionBodyLiterals.h"
#include "ast/transform/Pipeline.h"
#include "ast/transform/PragmaChecker.h"
#include "ast/transform/ReduceExistentials.h"
#include "ast/transform/RemoveBooleanConstraints.h"
#include "ast/transform/RemoveEmptyRelations.h"
#include "ast/transform/RemoveRedundantRelations.h"
#include "ast/transform/RemoveRedundantSums.h"
#include "ast/transform/RemoveRelationCopies.h"
#include "ast/transform/ReplaceSingletonVariables.h"
#include "ast/transform/ResolveAliases.h"
#include "ast/transform/ResolveAnonymousRecordAliases.h"
#include "ast/transform/SemanticChecker.h"
#include "ast/transform/SimplifyAggregateTargetExpression.h"
#include "ast/transform/SubsumptionQualifier.h"
#include "ast/transform/UniqueAggregationVariables.h"
#include "ast2ram/TranslationStrategy.h"
#include "ast2ram/UnitTranslator.h"
#include "ast2ram/provenance/TranslationStrategy.h"
#include "ast2ram/provenance/UnitTranslator.h"
#include "ast2ram/seminaive/TranslationStrategy.h"
#include "ast2ram/seminaive/UnitTranslator.h"
#include "ast2ram/utility/TranslatorContext.h"
#include "interpreter/Engine.h"
#include "interpreter/ProgInterface.h"
#include "parser/ParserDriver.h"
#include "ram/Node.h"
#include "ram/Program.h"
#include "ram/TranslationUnit.h"
#include "ram/transform/CollapseFilters.h"
#include "ram/transform/Conditional.h"
#include "ram/transform/EliminateDuplicates.h"
#include "ram/transform/ExpandFilter.h"
#include "ram/transform/HoistAggregate.h"
#include "ram/transform/HoistConditions.h"
#include "ram/transform/IfConversion.h"
#include "ram/transform/IfExistsConversion.h"
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
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/FileUtil.h"
#include "souffle/utility/MiscUtil.h"
#include "souffle/utility/StreamUtil.h"
#include "souffle/utility/StringUtil.h"
#include "souffle/utility/SubProcess.h"
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
  auto magicPipeline = mk<ast::transform::PipelineTransformer>(
          mk<ast::transform::ConditionalTransformer>(
                  Global::config().has("magic-transform"), mk<ast::transform::ExpandEqrelsTransformer>()),
          mk<ast::transform::MagicSetTransformer>(), mk<ast::transform::ResolveAliasesTransformer>(),
          mk<ast::transform::RemoveRelationCopiesTransformer>(),
          mk<ast::transform::RemoveEmptyRelationsTransformer>(),
          mk<ast::transform::RemoveRedundantRelationsTransformer>(), clone(equivalencePipeline));

  // Partitioning pipeline
  auto partitionPipeline =
          mk<ast::transform::PipelineTransformer>(mk<ast::transform::NameUnnamedVariablesTransformer>(),
                  mk<ast::transform::PartitionBodyLiteralsTransformer>(),
                  mk<ast::transform::ReplaceSingletonVariablesTransformer>());

  // Provenance pipeline
  auto provenancePipeline = mk<ast::transform::ConditionalTransformer>(Global::config().has("provenance"),
          mk<ast::transform::PipelineTransformer>(mk<ast::transform::ExpandEqrelsTransformer>(),
                  mk<ast::transform::NameUnnamedVariablesTransformer>()));

  // Main pipeline
  auto pipeline = mk<ast::transform::PipelineTransformer>(mk<ast::transform::ComponentChecker>(),
          mk<ast::transform::ComponentInstantiationTransformer>(),
          mk<ast::transform::IODefaultsTransformer>(),
          mk<ast::transform::SimplifyAggregateTargetExpressionTransformer>(),
          mk<ast::transform::UniqueAggregationVariablesTransformer>(),
          mk<ast::transform::FixpointTransformer>(mk<ast::transform::PipelineTransformer>(
                  mk<ast::transform::ResolveAnonymousRecordAliasesTransformer>(),
                  mk<ast::transform::FoldAnonymousRecords>())),
          mk<ast::transform::SubsumptionQualifierTransformer>(), mk<ast::transform::SemanticChecker>(),
          mk<ast::transform::GroundWitnessesTransformer>(),
          mk<ast::transform::UniqueAggregationVariablesTransformer>(),
          mk<ast::transform::MaterializeSingletonAggregationTransformer>(),
          mk<ast::transform::FixpointTransformer>(
                  mk<ast::transform::MaterializeAggregationQueriesTransformer>()),
          mk<ast::transform::RemoveRedundantSumsTransformer>(),
          mk<ast::transform::NormaliseGeneratorsTransformer>(),
          mk<ast::transform::ResolveAliasesTransformer>(),
          mk<ast::transform::RemoveBooleanConstraintsTransformer>(),
          mk<ast::transform::ResolveAliasesTransformer>(), mk<ast::transform::MinimiseProgramTransformer>(),
          mk<ast::transform::InlineUnmarkExcludedTransform>(),
          mk<ast::transform::InlineRelationsTransformer>(), mk<ast::transform::GroundedTermsChecker>(),
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
          std::move(magicPipeline), mk<ast::transform::RemoveEmptyRelationsTransformer>(),
          mk<ast::transform::AddNullariesToAtomlessAggregatesTransformer>(),
          mk<ast::transform::ExecutionPlanChecker>(), std::move(provenancePipeline),
          mk<ast::transform::IOAttributesTransformer>());

  // Disable unwanted transformations
  if (Global::config().has("disable-transformers")) {
      std::vector<std::string> givenTransformers =
              splitString(Global::config().get("disable-transformers"), ',');
      pipeline->disableTransformers(
              std::set<std::string>(givenTransformers.begin(), givenTransformers.end()));
  }

  // Toggle pipeline verbosity
  pipeline->setVerbosity(Global::config().has("verbose"));

  // Apply all the transformations
  pipeline->apply(*astTranslationUnit);

  // ------- execution -------------
  /* translate AST to RAM */
  debugReport.startSection();
  auto translationStrategy =
          Global::config().has("provenance")
                  ? mk<ast2ram::TranslationStrategy, ast2ram::provenance::TranslationStrategy>()
                  : mk<ast2ram::TranslationStrategy, ast2ram::seminaive::TranslationStrategy>();
  auto unitTranslator = Own<ast2ram::UnitTranslator>(translationStrategy->createUnitTranslator());
  auto ramTranslationUnit = unitTranslator->translateUnit(*astTranslationUnit);
  debugReport.endSection("ast-to-ram", "Translate AST to RAM");

  // Apply RAM transforms
  {
      using namespace ram::transform;
      Own<Transformer> ramTransform = mk<TransformerSequence>(
              mk<LoopTransformer>(mk<TransformerSequence>(mk<ExpandFilterTransformer>(),
                      mk<HoistConditionsTransformer>(), mk<MakeIndexTransformer>())),
              mk<IfConversionTransformer>(), mk<IfExistsConversionTransformer>(),
              mk<CollapseFiltersTransformer>(), mk<TupleIdTransformer>(),
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
