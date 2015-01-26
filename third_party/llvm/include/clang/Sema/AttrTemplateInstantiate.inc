/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Template instantiation code for attributes                                  *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

namespace clang {
namespace sema {

Attr *instantiateTemplateAttribute(const Attr *At, ASTContext &C, Sema &S,
        const MultiLevelTemplateArgumentList &TemplateArgs) {
  switch (At->getKind()) {
    default:
      break;
    case attr::ARMInterrupt: {
      const ARMInterruptAttr *A = cast<ARMInterruptAttr>(At);
      return A->clone(C);
    }
    case attr::AcquireCapability: {
      const AcquireCapabilityAttr *A = cast<AcquireCapabilityAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) AcquireCapabilityAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::AcquiredAfter: {
      const AcquiredAfterAttr *A = cast<AcquiredAfterAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) AcquiredAfterAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::AcquiredBefore: {
      const AcquiredBeforeAttr *A = cast<AcquiredBeforeAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) AcquiredBeforeAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::Alias: {
      const AliasAttr *A = cast<AliasAttr>(At);
      return A->clone(C);
    }
    case attr::AlignMac68k: {
      const AlignMac68kAttr *A = cast<AlignMac68kAttr>(At);
      return A->clone(C);
    }
    case attr::AlignValue: {
      const AlignValueAttr *A = cast<AlignValueAttr>(At);
      return A->clone(C);
    }
    case attr::Aligned: {
      const AlignedAttr *A = cast<AlignedAttr>(At);
      return A->clone(C);
    }
    case attr::AlwaysInline: {
      const AlwaysInlineAttr *A = cast<AlwaysInlineAttr>(At);
      return A->clone(C);
    }
    case attr::AnalyzerNoReturn: {
      const AnalyzerNoReturnAttr *A = cast<AnalyzerNoReturnAttr>(At);
      return A->clone(C);
    }
    case attr::Annotate: {
      const AnnotateAttr *A = cast<AnnotateAttr>(At);
      return A->clone(C);
    }
    case attr::ArcWeakrefUnavailable: {
      const ArcWeakrefUnavailableAttr *A = cast<ArcWeakrefUnavailableAttr>(At);
      return A->clone(C);
    }
    case attr::ArgumentWithTypeTag: {
      const ArgumentWithTypeTagAttr *A = cast<ArgumentWithTypeTagAttr>(At);
      return A->clone(C);
    }
    case attr::AsmLabel: {
      const AsmLabelAttr *A = cast<AsmLabelAttr>(At);
      return A->clone(C);
    }
    case attr::AssertCapability: {
      const AssertCapabilityAttr *A = cast<AssertCapabilityAttr>(At);
      Expr * tempInstExpr;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getExpr(), TemplateArgs);
        tempInstExpr = Result.getAs<Expr>();
      }
      return new (C) AssertCapabilityAttr(A->getLocation(), C, tempInstExpr, A->getSpellingListIndex());
    }
    case attr::AssertExclusiveLock: {
      const AssertExclusiveLockAttr *A = cast<AssertExclusiveLockAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) AssertExclusiveLockAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::AssertSharedLock: {
      const AssertSharedLockAttr *A = cast<AssertSharedLockAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) AssertSharedLockAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::AssumeAligned: {
      const AssumeAlignedAttr *A = cast<AssumeAlignedAttr>(At);
      return A->clone(C);
    }
    case attr::Availability: {
      const AvailabilityAttr *A = cast<AvailabilityAttr>(At);
      return A->clone(C);
    }
    case attr::Blocks: {
      const BlocksAttr *A = cast<BlocksAttr>(At);
      return A->clone(C);
    }
    case attr::C11NoReturn: {
      const C11NoReturnAttr *A = cast<C11NoReturnAttr>(At);
      return A->clone(C);
    }
    case attr::CDecl: {
      const CDeclAttr *A = cast<CDeclAttr>(At);
      return A->clone(C);
    }
    case attr::CFAuditedTransfer: {
      const CFAuditedTransferAttr *A = cast<CFAuditedTransferAttr>(At);
      return A->clone(C);
    }
    case attr::CFConsumed: {
      const CFConsumedAttr *A = cast<CFConsumedAttr>(At);
      return A->clone(C);
    }
    case attr::CFReturnsNotRetained: {
      const CFReturnsNotRetainedAttr *A = cast<CFReturnsNotRetainedAttr>(At);
      return A->clone(C);
    }
    case attr::CFReturnsRetained: {
      const CFReturnsRetainedAttr *A = cast<CFReturnsRetainedAttr>(At);
      return A->clone(C);
    }
    case attr::CFUnknownTransfer: {
      const CFUnknownTransferAttr *A = cast<CFUnknownTransferAttr>(At);
      return A->clone(C);
    }
    case attr::CUDAConstant: {
      const CUDAConstantAttr *A = cast<CUDAConstantAttr>(At);
      return A->clone(C);
    }
    case attr::CUDADevice: {
      const CUDADeviceAttr *A = cast<CUDADeviceAttr>(At);
      return A->clone(C);
    }
    case attr::CUDAGlobal: {
      const CUDAGlobalAttr *A = cast<CUDAGlobalAttr>(At);
      return A->clone(C);
    }
    case attr::CUDAHost: {
      const CUDAHostAttr *A = cast<CUDAHostAttr>(At);
      return A->clone(C);
    }
    case attr::CUDAInvalidTarget: {
      const CUDAInvalidTargetAttr *A = cast<CUDAInvalidTargetAttr>(At);
      return A->clone(C);
    }
    case attr::CUDALaunchBounds: {
      const CUDALaunchBoundsAttr *A = cast<CUDALaunchBoundsAttr>(At);
      return A->clone(C);
    }
    case attr::CUDAShared: {
      const CUDASharedAttr *A = cast<CUDASharedAttr>(At);
      return A->clone(C);
    }
    case attr::CXX11NoReturn: {
      const CXX11NoReturnAttr *A = cast<CXX11NoReturnAttr>(At);
      return A->clone(C);
    }
    case attr::CallableWhen: {
      const CallableWhenAttr *A = cast<CallableWhenAttr>(At);
      return A->clone(C);
    }
    case attr::Capability: {
      const CapabilityAttr *A = cast<CapabilityAttr>(At);
      return A->clone(C);
    }
    case attr::CapturedRecord: {
      const CapturedRecordAttr *A = cast<CapturedRecordAttr>(At);
      return A->clone(C);
    }
    case attr::CarriesDependency: {
      const CarriesDependencyAttr *A = cast<CarriesDependencyAttr>(At);
      return A->clone(C);
    }
    case attr::Cleanup: {
      const CleanupAttr *A = cast<CleanupAttr>(At);
      return A->clone(C);
    }
    case attr::Cold: {
      const ColdAttr *A = cast<ColdAttr>(At);
      return A->clone(C);
    }
    case attr::Common: {
      const CommonAttr *A = cast<CommonAttr>(At);
      return A->clone(C);
    }
    case attr::Const: {
      const ConstAttr *A = cast<ConstAttr>(At);
      return A->clone(C);
    }
    case attr::Constructor: {
      const ConstructorAttr *A = cast<ConstructorAttr>(At);
      return A->clone(C);
    }
    case attr::Consumable: {
      const ConsumableAttr *A = cast<ConsumableAttr>(At);
      return A->clone(C);
    }
    case attr::ConsumableAutoCast: {
      const ConsumableAutoCastAttr *A = cast<ConsumableAutoCastAttr>(At);
      return A->clone(C);
    }
    case attr::ConsumableSetOnRead: {
      const ConsumableSetOnReadAttr *A = cast<ConsumableSetOnReadAttr>(At);
      return A->clone(C);
    }
    case attr::DLLExport: {
      const DLLExportAttr *A = cast<DLLExportAttr>(At);
      return A->clone(C);
    }
    case attr::DLLImport: {
      const DLLImportAttr *A = cast<DLLImportAttr>(At);
      return A->clone(C);
    }
    case attr::Deprecated: {
      const DeprecatedAttr *A = cast<DeprecatedAttr>(At);
      return A->clone(C);
    }
    case attr::Destructor: {
      const DestructorAttr *A = cast<DestructorAttr>(At);
      return A->clone(C);
    }
    case attr::EnableIf: {
      const EnableIfAttr *A = cast<EnableIfAttr>(At);
      Expr * tempInstCond;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getCond(), TemplateArgs);
        tempInstCond = Result.getAs<Expr>();
      }
      return new (C) EnableIfAttr(A->getLocation(), C, tempInstCond, A->getMessage(), A->getSpellingListIndex());
    }
    case attr::ExclusiveTrylockFunction: {
      const ExclusiveTrylockFunctionAttr *A = cast<ExclusiveTrylockFunctionAttr>(At);
      Expr * tempInstSuccessValue;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getSuccessValue(), TemplateArgs);
        tempInstSuccessValue = Result.getAs<Expr>();
      }
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) ExclusiveTrylockFunctionAttr(A->getLocation(), C, tempInstSuccessValue, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::FallThrough: {
      const FallThroughAttr *A = cast<FallThroughAttr>(At);
      return A->clone(C);
    }
    case attr::FastCall: {
      const FastCallAttr *A = cast<FastCallAttr>(At);
      return A->clone(C);
    }
    case attr::Final: {
      const FinalAttr *A = cast<FinalAttr>(At);
      return A->clone(C);
    }
    case attr::Flatten: {
      const FlattenAttr *A = cast<FlattenAttr>(At);
      return A->clone(C);
    }
    case attr::Format: {
      const FormatAttr *A = cast<FormatAttr>(At);
      return A->clone(C);
    }
    case attr::FormatArg: {
      const FormatArgAttr *A = cast<FormatArgAttr>(At);
      return A->clone(C);
    }
    case attr::GNUInline: {
      const GNUInlineAttr *A = cast<GNUInlineAttr>(At);
      return A->clone(C);
    }
    case attr::GuardedBy: {
      const GuardedByAttr *A = cast<GuardedByAttr>(At);
      Expr * tempInstArg;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getArg(), TemplateArgs);
        tempInstArg = Result.getAs<Expr>();
      }
      return new (C) GuardedByAttr(A->getLocation(), C, tempInstArg, A->getSpellingListIndex());
    }
    case attr::GuardedVar: {
      const GuardedVarAttr *A = cast<GuardedVarAttr>(At);
      return A->clone(C);
    }
    case attr::Hot: {
      const HotAttr *A = cast<HotAttr>(At);
      return A->clone(C);
    }
    case attr::IBAction: {
      const IBActionAttr *A = cast<IBActionAttr>(At);
      return A->clone(C);
    }
    case attr::IBOutlet: {
      const IBOutletAttr *A = cast<IBOutletAttr>(At);
      return A->clone(C);
    }
    case attr::IBOutletCollection: {
      const IBOutletCollectionAttr *A = cast<IBOutletCollectionAttr>(At);
      return A->clone(C);
    }
    case attr::InitPriority: {
      const InitPriorityAttr *A = cast<InitPriorityAttr>(At);
      return A->clone(C);
    }
    case attr::InitSeg: {
      const InitSegAttr *A = cast<InitSegAttr>(At);
      return A->clone(C);
    }
    case attr::IntelOclBicc: {
      const IntelOclBiccAttr *A = cast<IntelOclBiccAttr>(At);
      return A->clone(C);
    }
    case attr::LockReturned: {
      const LockReturnedAttr *A = cast<LockReturnedAttr>(At);
      Expr * tempInstArg;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getArg(), TemplateArgs);
        tempInstArg = Result.getAs<Expr>();
      }
      return new (C) LockReturnedAttr(A->getLocation(), C, tempInstArg, A->getSpellingListIndex());
    }
    case attr::LocksExcluded: {
      const LocksExcludedAttr *A = cast<LocksExcludedAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) LocksExcludedAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::LoopHint: {
      const LoopHintAttr *A = cast<LoopHintAttr>(At);
      return A->clone(C);
    }
    case attr::MSABI: {
      const MSABIAttr *A = cast<MSABIAttr>(At);
      return A->clone(C);
    }
    case attr::MSInheritance: {
      const MSInheritanceAttr *A = cast<MSInheritanceAttr>(At);
      return A->clone(C);
    }
    case attr::MSP430Interrupt: {
      const MSP430InterruptAttr *A = cast<MSP430InterruptAttr>(At);
      return A->clone(C);
    }
    case attr::MSVtorDisp: {
      const MSVtorDispAttr *A = cast<MSVtorDispAttr>(At);
      return A->clone(C);
    }
    case attr::Malloc: {
      const MallocAttr *A = cast<MallocAttr>(At);
      return A->clone(C);
    }
    case attr::MaxFieldAlignment: {
      const MaxFieldAlignmentAttr *A = cast<MaxFieldAlignmentAttr>(At);
      return A->clone(C);
    }
    case attr::MayAlias: {
      const MayAliasAttr *A = cast<MayAliasAttr>(At);
      return A->clone(C);
    }
    case attr::MinSize: {
      const MinSizeAttr *A = cast<MinSizeAttr>(At);
      return A->clone(C);
    }
    case attr::Mips16: {
      const Mips16Attr *A = cast<Mips16Attr>(At);
      return A->clone(C);
    }
    case attr::Mode: {
      const ModeAttr *A = cast<ModeAttr>(At);
      return A->clone(C);
    }
    case attr::MsStruct: {
      const MsStructAttr *A = cast<MsStructAttr>(At);
      return A->clone(C);
    }
    case attr::NSConsumed: {
      const NSConsumedAttr *A = cast<NSConsumedAttr>(At);
      return A->clone(C);
    }
    case attr::NSConsumesSelf: {
      const NSConsumesSelfAttr *A = cast<NSConsumesSelfAttr>(At);
      return A->clone(C);
    }
    case attr::NSReturnsAutoreleased: {
      const NSReturnsAutoreleasedAttr *A = cast<NSReturnsAutoreleasedAttr>(At);
      return A->clone(C);
    }
    case attr::NSReturnsNotRetained: {
      const NSReturnsNotRetainedAttr *A = cast<NSReturnsNotRetainedAttr>(At);
      return A->clone(C);
    }
    case attr::NSReturnsRetained: {
      const NSReturnsRetainedAttr *A = cast<NSReturnsRetainedAttr>(At);
      return A->clone(C);
    }
    case attr::Naked: {
      const NakedAttr *A = cast<NakedAttr>(At);
      return A->clone(C);
    }
    case attr::NoCommon: {
      const NoCommonAttr *A = cast<NoCommonAttr>(At);
      return A->clone(C);
    }
    case attr::NoDebug: {
      const NoDebugAttr *A = cast<NoDebugAttr>(At);
      return A->clone(C);
    }
    case attr::NoDuplicate: {
      const NoDuplicateAttr *A = cast<NoDuplicateAttr>(At);
      return A->clone(C);
    }
    case attr::NoInline: {
      const NoInlineAttr *A = cast<NoInlineAttr>(At);
      return A->clone(C);
    }
    case attr::NoInstrumentFunction: {
      const NoInstrumentFunctionAttr *A = cast<NoInstrumentFunctionAttr>(At);
      return A->clone(C);
    }
    case attr::NoMips16: {
      const NoMips16Attr *A = cast<NoMips16Attr>(At);
      return A->clone(C);
    }
    case attr::NoReturn: {
      const NoReturnAttr *A = cast<NoReturnAttr>(At);
      return A->clone(C);
    }
    case attr::NoSanitizeAddress: {
      const NoSanitizeAddressAttr *A = cast<NoSanitizeAddressAttr>(At);
      return A->clone(C);
    }
    case attr::NoSanitizeMemory: {
      const NoSanitizeMemoryAttr *A = cast<NoSanitizeMemoryAttr>(At);
      return A->clone(C);
    }
    case attr::NoSanitizeThread: {
      const NoSanitizeThreadAttr *A = cast<NoSanitizeThreadAttr>(At);
      return A->clone(C);
    }
    case attr::NoSplitStack: {
      const NoSplitStackAttr *A = cast<NoSplitStackAttr>(At);
      return A->clone(C);
    }
    case attr::NoThreadSafetyAnalysis: {
      const NoThreadSafetyAnalysisAttr *A = cast<NoThreadSafetyAnalysisAttr>(At);
      return A->clone(C);
    }
    case attr::NoThrow: {
      const NoThrowAttr *A = cast<NoThrowAttr>(At);
      return A->clone(C);
    }
    case attr::NonNull: {
      const NonNullAttr *A = cast<NonNullAttr>(At);
      return A->clone(C);
    }
    case attr::OMPThreadPrivateDecl: {
      const OMPThreadPrivateDeclAttr *A = cast<OMPThreadPrivateDeclAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCBridge: {
      const ObjCBridgeAttr *A = cast<ObjCBridgeAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCBridgeMutable: {
      const ObjCBridgeMutableAttr *A = cast<ObjCBridgeMutableAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCBridgeRelated: {
      const ObjCBridgeRelatedAttr *A = cast<ObjCBridgeRelatedAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCDesignatedInitializer: {
      const ObjCDesignatedInitializerAttr *A = cast<ObjCDesignatedInitializerAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCException: {
      const ObjCExceptionAttr *A = cast<ObjCExceptionAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCExplicitProtocolImpl: {
      const ObjCExplicitProtocolImplAttr *A = cast<ObjCExplicitProtocolImplAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCMethodFamily: {
      const ObjCMethodFamilyAttr *A = cast<ObjCMethodFamilyAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCNSObject: {
      const ObjCNSObjectAttr *A = cast<ObjCNSObjectAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCPreciseLifetime: {
      const ObjCPreciseLifetimeAttr *A = cast<ObjCPreciseLifetimeAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCRequiresPropertyDefs: {
      const ObjCRequiresPropertyDefsAttr *A = cast<ObjCRequiresPropertyDefsAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCRequiresSuper: {
      const ObjCRequiresSuperAttr *A = cast<ObjCRequiresSuperAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCReturnsInnerPointer: {
      const ObjCReturnsInnerPointerAttr *A = cast<ObjCReturnsInnerPointerAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCRootClass: {
      const ObjCRootClassAttr *A = cast<ObjCRootClassAttr>(At);
      return A->clone(C);
    }
    case attr::ObjCRuntimeName: {
      const ObjCRuntimeNameAttr *A = cast<ObjCRuntimeNameAttr>(At);
      return A->clone(C);
    }
    case attr::OpenCLImageAccess: {
      const OpenCLImageAccessAttr *A = cast<OpenCLImageAccessAttr>(At);
      return A->clone(C);
    }
    case attr::OpenCLKernel: {
      const OpenCLKernelAttr *A = cast<OpenCLKernelAttr>(At);
      return A->clone(C);
    }
    case attr::OptimizeNone: {
      const OptimizeNoneAttr *A = cast<OptimizeNoneAttr>(At);
      return A->clone(C);
    }
    case attr::Overloadable: {
      const OverloadableAttr *A = cast<OverloadableAttr>(At);
      return A->clone(C);
    }
    case attr::Override: {
      const OverrideAttr *A = cast<OverrideAttr>(At);
      return A->clone(C);
    }
    case attr::Ownership: {
      const OwnershipAttr *A = cast<OwnershipAttr>(At);
      return A->clone(C);
    }
    case attr::Packed: {
      const PackedAttr *A = cast<PackedAttr>(At);
      return A->clone(C);
    }
    case attr::ParamTypestate: {
      const ParamTypestateAttr *A = cast<ParamTypestateAttr>(At);
      return A->clone(C);
    }
    case attr::Pascal: {
      const PascalAttr *A = cast<PascalAttr>(At);
      return A->clone(C);
    }
    case attr::Pcs: {
      const PcsAttr *A = cast<PcsAttr>(At);
      return A->clone(C);
    }
    case attr::PnaclCall: {
      const PnaclCallAttr *A = cast<PnaclCallAttr>(At);
      return A->clone(C);
    }
    case attr::PtGuardedBy: {
      const PtGuardedByAttr *A = cast<PtGuardedByAttr>(At);
      Expr * tempInstArg;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getArg(), TemplateArgs);
        tempInstArg = Result.getAs<Expr>();
      }
      return new (C) PtGuardedByAttr(A->getLocation(), C, tempInstArg, A->getSpellingListIndex());
    }
    case attr::PtGuardedVar: {
      const PtGuardedVarAttr *A = cast<PtGuardedVarAttr>(At);
      return A->clone(C);
    }
    case attr::Pure: {
      const PureAttr *A = cast<PureAttr>(At);
      return A->clone(C);
    }
    case attr::ReleaseCapability: {
      const ReleaseCapabilityAttr *A = cast<ReleaseCapabilityAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) ReleaseCapabilityAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::ReqdWorkGroupSize: {
      const ReqdWorkGroupSizeAttr *A = cast<ReqdWorkGroupSizeAttr>(At);
      return A->clone(C);
    }
    case attr::RequiresCapability: {
      const RequiresCapabilityAttr *A = cast<RequiresCapabilityAttr>(At);
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) RequiresCapabilityAttr(A->getLocation(), C, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::ReturnTypestate: {
      const ReturnTypestateAttr *A = cast<ReturnTypestateAttr>(At);
      return A->clone(C);
    }
    case attr::ReturnsNonNull: {
      const ReturnsNonNullAttr *A = cast<ReturnsNonNullAttr>(At);
      return A->clone(C);
    }
    case attr::ReturnsTwice: {
      const ReturnsTwiceAttr *A = cast<ReturnsTwiceAttr>(At);
      return A->clone(C);
    }
    case attr::ScopedLockable: {
      const ScopedLockableAttr *A = cast<ScopedLockableAttr>(At);
      return A->clone(C);
    }
    case attr::Section: {
      const SectionAttr *A = cast<SectionAttr>(At);
      return A->clone(C);
    }
    case attr::SelectAny: {
      const SelectAnyAttr *A = cast<SelectAnyAttr>(At);
      return A->clone(C);
    }
    case attr::Sentinel: {
      const SentinelAttr *A = cast<SentinelAttr>(At);
      return A->clone(C);
    }
    case attr::SetTypestate: {
      const SetTypestateAttr *A = cast<SetTypestateAttr>(At);
      return A->clone(C);
    }
    case attr::SharedTrylockFunction: {
      const SharedTrylockFunctionAttr *A = cast<SharedTrylockFunctionAttr>(At);
      Expr * tempInstSuccessValue;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getSuccessValue(), TemplateArgs);
        tempInstSuccessValue = Result.getAs<Expr>();
      }
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) SharedTrylockFunctionAttr(A->getLocation(), C, tempInstSuccessValue, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::StdCall: {
      const StdCallAttr *A = cast<StdCallAttr>(At);
      return A->clone(C);
    }
    case attr::SysVABI: {
      const SysVABIAttr *A = cast<SysVABIAttr>(At);
      return A->clone(C);
    }
    case attr::TLSModel: {
      const TLSModelAttr *A = cast<TLSModelAttr>(At);
      return A->clone(C);
    }
    case attr::TestTypestate: {
      const TestTypestateAttr *A = cast<TestTypestateAttr>(At);
      return A->clone(C);
    }
    case attr::ThisCall: {
      const ThisCallAttr *A = cast<ThisCallAttr>(At);
      return A->clone(C);
    }
    case attr::Thread: {
      const ThreadAttr *A = cast<ThreadAttr>(At);
      return A->clone(C);
    }
    case attr::TransparentUnion: {
      const TransparentUnionAttr *A = cast<TransparentUnionAttr>(At);
      return A->clone(C);
    }
    case attr::TryAcquireCapability: {
      const TryAcquireCapabilityAttr *A = cast<TryAcquireCapabilityAttr>(At);
      Expr * tempInstSuccessValue;
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        ExprResult Result = S.SubstExpr(A->getSuccessValue(), TemplateArgs);
        tempInstSuccessValue = Result.getAs<Expr>();
      }
      Expr * *tempInstArgs = new (C, 16) Expr *[A->args_size()];
      {
        EnterExpressionEvaluationContext Unevaluated(S, Sema::Unevaluated);
        Expr * *TI = tempInstArgs;
        Expr * *I = A->args_begin();
        Expr * *E = A->args_end();
        for (; I != E; ++I, ++TI) {
          ExprResult Result = S.SubstExpr(*I, TemplateArgs);
          *TI = Result.getAs<Expr>();
        }
      }
      return new (C) TryAcquireCapabilityAttr(A->getLocation(), C, tempInstSuccessValue, tempInstArgs, A->args_size(), A->getSpellingListIndex());
    }
    case attr::TypeTagForDatatype: {
      const TypeTagForDatatypeAttr *A = cast<TypeTagForDatatypeAttr>(At);
      return A->clone(C);
    }
    case attr::TypeVisibility: {
      return NULL;
    }
    case attr::Unavailable: {
      const UnavailableAttr *A = cast<UnavailableAttr>(At);
      return A->clone(C);
    }
    case attr::Unused: {
      const UnusedAttr *A = cast<UnusedAttr>(At);
      return A->clone(C);
    }
    case attr::Used: {
      const UsedAttr *A = cast<UsedAttr>(At);
      return A->clone(C);
    }
    case attr::Uuid: {
      const UuidAttr *A = cast<UuidAttr>(At);
      return A->clone(C);
    }
    case attr::VecReturn: {
      const VecReturnAttr *A = cast<VecReturnAttr>(At);
      return A->clone(C);
    }
    case attr::VecTypeHint: {
      const VecTypeHintAttr *A = cast<VecTypeHintAttr>(At);
      return A->clone(C);
    }
    case attr::VectorCall: {
      const VectorCallAttr *A = cast<VectorCallAttr>(At);
      return A->clone(C);
    }
    case attr::Visibility: {
      return NULL;
    }
    case attr::WarnUnused: {
      const WarnUnusedAttr *A = cast<WarnUnusedAttr>(At);
      return A->clone(C);
    }
    case attr::WarnUnusedResult: {
      const WarnUnusedResultAttr *A = cast<WarnUnusedResultAttr>(At);
      return A->clone(C);
    }
    case attr::Weak: {
      const WeakAttr *A = cast<WeakAttr>(At);
      return A->clone(C);
    }
    case attr::WeakImport: {
      const WeakImportAttr *A = cast<WeakImportAttr>(At);
      return A->clone(C);
    }
    case attr::WeakRef: {
      const WeakRefAttr *A = cast<WeakRefAttr>(At);
      return A->clone(C);
    }
    case attr::WorkGroupSizeHint: {
      const WorkGroupSizeHintAttr *A = cast<WorkGroupSizeHintAttr>(At);
      return A->clone(C);
    }
    case attr::X86ForceAlignArgPointer: {
      const X86ForceAlignArgPointerAttr *A = cast<X86ForceAlignArgPointerAttr>(At);
      return A->clone(C);
    }
  } // end switch
  llvm_unreachable("Unknown attribute!");
  return 0;
}

} // end namespace sema
} // end namespace clang
