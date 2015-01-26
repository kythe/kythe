/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Attribute serialization code                                                *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

  switch (A->getKind()) {
  default:
    llvm_unreachable("Unknown attribute kind!");
    break;
  case attr::ARMInterrupt: {
    const ARMInterruptAttr *SA = cast<ARMInterruptAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getInterrupt());
    break;
  }
  case attr::AcquireCapability: {
    const AcquireCapabilityAttr *SA = cast<AcquireCapabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::AcquiredAfter: {
    const AcquiredAfterAttr *SA = cast<AcquiredAfterAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::AcquiredBefore: {
    const AcquiredBeforeAttr *SA = cast<AcquiredBeforeAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::Alias: {
    const AliasAttr *SA = cast<AliasAttr>(A);
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getAliasee(), Record);
    break;
  }
  case attr::AlignMac68k: {
    const AlignMac68kAttr *SA = cast<AlignMac68kAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::AlignValue: {
    const AlignValueAttr *SA = cast<AlignValueAttr>(A);
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getAlignment());
    break;
  }
  case attr::Aligned: {
    const AlignedAttr *SA = cast<AlignedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->isAlignmentExpr());
    if (SA->isAlignmentExpr())
      AddStmt(SA->getAlignmentExpr());
    else
      AddTypeSourceInfo(SA->getAlignmentType(), Record);
    break;
  }
  case attr::AlwaysInline: {
    const AlwaysInlineAttr *SA = cast<AlwaysInlineAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::AnalyzerNoReturn: {
    const AnalyzerNoReturnAttr *SA = cast<AnalyzerNoReturnAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Annotate: {
    const AnnotateAttr *SA = cast<AnnotateAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getAnnotation(), Record);
    break;
  }
  case attr::ArcWeakrefUnavailable: {
    const ArcWeakrefUnavailableAttr *SA = cast<ArcWeakrefUnavailableAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ArgumentWithTypeTag: {
    const ArgumentWithTypeTagAttr *SA = cast<ArgumentWithTypeTagAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getArgumentKind(), Record);
    Record.push_back(SA->getArgumentIdx());
    Record.push_back(SA->getTypeTagIdx());
    Record.push_back(SA->getIsPointer());
    break;
  }
  case attr::AsmLabel: {
    const AsmLabelAttr *SA = cast<AsmLabelAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getLabel(), Record);
    break;
  }
  case attr::AssertCapability: {
    const AssertCapabilityAttr *SA = cast<AssertCapabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getExpr());
    break;
  }
  case attr::AssertExclusiveLock: {
    const AssertExclusiveLockAttr *SA = cast<AssertExclusiveLockAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::AssertSharedLock: {
    const AssertSharedLockAttr *SA = cast<AssertSharedLockAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::AssumeAligned: {
    const AssumeAlignedAttr *SA = cast<AssumeAlignedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getAlignment());
    AddStmt(SA->getOffset());
    break;
  }
  case attr::Availability: {
    const AvailabilityAttr *SA = cast<AvailabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getPlatform(), Record);
    AddVersionTuple(SA->getIntroduced(), Record);
    AddVersionTuple(SA->getDeprecated(), Record);
    AddVersionTuple(SA->getObsoleted(), Record);
    Record.push_back(SA->getUnavailable());
    AddString(SA->getMessage(), Record);
    break;
  }
  case attr::Blocks: {
    const BlocksAttr *SA = cast<BlocksAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getType());
    break;
  }
  case attr::C11NoReturn: {
    const C11NoReturnAttr *SA = cast<C11NoReturnAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CDecl: {
    const CDeclAttr *SA = cast<CDeclAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CFAuditedTransfer: {
    const CFAuditedTransferAttr *SA = cast<CFAuditedTransferAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CFConsumed: {
    const CFConsumedAttr *SA = cast<CFConsumedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CFReturnsNotRetained: {
    const CFReturnsNotRetainedAttr *SA = cast<CFReturnsNotRetainedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CFReturnsRetained: {
    const CFReturnsRetainedAttr *SA = cast<CFReturnsRetainedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CFUnknownTransfer: {
    const CFUnknownTransferAttr *SA = cast<CFUnknownTransferAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CUDAConstant: {
    const CUDAConstantAttr *SA = cast<CUDAConstantAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CUDADevice: {
    const CUDADeviceAttr *SA = cast<CUDADeviceAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CUDAGlobal: {
    const CUDAGlobalAttr *SA = cast<CUDAGlobalAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CUDAHost: {
    const CUDAHostAttr *SA = cast<CUDAHostAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CUDAInvalidTarget: {
    const CUDAInvalidTargetAttr *SA = cast<CUDAInvalidTargetAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CUDALaunchBounds: {
    const CUDALaunchBoundsAttr *SA = cast<CUDALaunchBoundsAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getMaxThreads());
    Record.push_back(SA->getMinBlocks());
    break;
  }
  case attr::CUDAShared: {
    const CUDASharedAttr *SA = cast<CUDASharedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CXX11NoReturn: {
    const CXX11NoReturnAttr *SA = cast<CXX11NoReturnAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CallableWhen: {
    const CallableWhenAttr *SA = cast<CallableWhenAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->callableStates_size());
    for (CallableWhenAttr::callableStates_iterator i = SA->callableStates_begin(), e = SA->callableStates_end(); i != e; ++i)
      Record.push_back((*i));
    break;
  }
  case attr::Capability: {
    const CapabilityAttr *SA = cast<CapabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getName(), Record);
    break;
  }
  case attr::CapturedRecord: {
    const CapturedRecordAttr *SA = cast<CapturedRecordAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::CarriesDependency: {
    const CarriesDependencyAttr *SA = cast<CarriesDependencyAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Cleanup: {
    const CleanupAttr *SA = cast<CleanupAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddDeclRef(SA->getFunctionDecl(), Record);
    break;
  }
  case attr::Cold: {
    const ColdAttr *SA = cast<ColdAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Common: {
    const CommonAttr *SA = cast<CommonAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Const: {
    const ConstAttr *SA = cast<ConstAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Constructor: {
    const ConstructorAttr *SA = cast<ConstructorAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getPriority());
    break;
  }
  case attr::Consumable: {
    const ConsumableAttr *SA = cast<ConsumableAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getDefaultState());
    break;
  }
  case attr::ConsumableAutoCast: {
    const ConsumableAutoCastAttr *SA = cast<ConsumableAutoCastAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ConsumableSetOnRead: {
    const ConsumableSetOnReadAttr *SA = cast<ConsumableSetOnReadAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::DLLExport: {
    const DLLExportAttr *SA = cast<DLLExportAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::DLLImport: {
    const DLLImportAttr *SA = cast<DLLImportAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Deprecated: {
    const DeprecatedAttr *SA = cast<DeprecatedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getMessage(), Record);
    break;
  }
  case attr::Destructor: {
    const DestructorAttr *SA = cast<DestructorAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getPriority());
    break;
  }
  case attr::EnableIf: {
    const EnableIfAttr *SA = cast<EnableIfAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getCond());
    AddString(SA->getMessage(), Record);
    break;
  }
  case attr::ExclusiveTrylockFunction: {
    const ExclusiveTrylockFunctionAttr *SA = cast<ExclusiveTrylockFunctionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getSuccessValue());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::FallThrough: {
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::FastCall: {
    const FastCallAttr *SA = cast<FastCallAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Final: {
    const FinalAttr *SA = cast<FinalAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Flatten: {
    const FlattenAttr *SA = cast<FlattenAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Format: {
    const FormatAttr *SA = cast<FormatAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getType(), Record);
    Record.push_back(SA->getFormatIdx());
    Record.push_back(SA->getFirstArg());
    break;
  }
  case attr::FormatArg: {
    const FormatArgAttr *SA = cast<FormatArgAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getFormatIdx());
    break;
  }
  case attr::GNUInline: {
    const GNUInlineAttr *SA = cast<GNUInlineAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::GuardedBy: {
    const GuardedByAttr *SA = cast<GuardedByAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getArg());
    break;
  }
  case attr::GuardedVar: {
    const GuardedVarAttr *SA = cast<GuardedVarAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Hot: {
    const HotAttr *SA = cast<HotAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::IBAction: {
    const IBActionAttr *SA = cast<IBActionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::IBOutlet: {
    const IBOutletAttr *SA = cast<IBOutletAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::IBOutletCollection: {
    const IBOutletCollectionAttr *SA = cast<IBOutletCollectionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddTypeSourceInfo(SA->getInterfaceLoc(), Record);
    break;
  }
  case attr::InitPriority: {
    const InitPriorityAttr *SA = cast<InitPriorityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getPriority());
    break;
  }
  case attr::InitSeg: {
    const InitSegAttr *SA = cast<InitSegAttr>(A);
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getSection(), Record);
    break;
  }
  case attr::IntelOclBicc: {
    const IntelOclBiccAttr *SA = cast<IntelOclBiccAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::LockReturned: {
    const LockReturnedAttr *SA = cast<LockReturnedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getArg());
    break;
  }
  case attr::LocksExcluded: {
    const LocksExcludedAttr *SA = cast<LocksExcludedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::LoopHint: {
    const LoopHintAttr *SA = cast<LoopHintAttr>(A);
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getOption());
Record.push_back(SA->getState());
    AddStmt(SA->getValue());
    break;
  }
  case attr::MSABI: {
    const MSABIAttr *SA = cast<MSABIAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::MSInheritance: {
    const MSInheritanceAttr *SA = cast<MSInheritanceAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getBestCase());
    break;
  }
  case attr::MSP430Interrupt: {
    const MSP430InterruptAttr *SA = cast<MSP430InterruptAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getNumber());
    break;
  }
  case attr::MSVtorDisp: {
    const MSVtorDispAttr *SA = cast<MSVtorDispAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getVdm());
    break;
  }
  case attr::Malloc: {
    const MallocAttr *SA = cast<MallocAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::MaxFieldAlignment: {
    const MaxFieldAlignmentAttr *SA = cast<MaxFieldAlignmentAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getAlignment());
    break;
  }
  case attr::MayAlias: {
    const MayAliasAttr *SA = cast<MayAliasAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::MinSize: {
    const MinSizeAttr *SA = cast<MinSizeAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Mips16: {
    const Mips16Attr *SA = cast<Mips16Attr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Mode: {
    const ModeAttr *SA = cast<ModeAttr>(A);
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getMode(), Record);
    break;
  }
  case attr::MsStruct: {
    const MsStructAttr *SA = cast<MsStructAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NSConsumed: {
    const NSConsumedAttr *SA = cast<NSConsumedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NSConsumesSelf: {
    const NSConsumesSelfAttr *SA = cast<NSConsumesSelfAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NSReturnsAutoreleased: {
    const NSReturnsAutoreleasedAttr *SA = cast<NSReturnsAutoreleasedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NSReturnsNotRetained: {
    const NSReturnsNotRetainedAttr *SA = cast<NSReturnsNotRetainedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NSReturnsRetained: {
    const NSReturnsRetainedAttr *SA = cast<NSReturnsRetainedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Naked: {
    const NakedAttr *SA = cast<NakedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoCommon: {
    const NoCommonAttr *SA = cast<NoCommonAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoDebug: {
    const NoDebugAttr *SA = cast<NoDebugAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoDuplicate: {
    const NoDuplicateAttr *SA = cast<NoDuplicateAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoInline: {
    const NoInlineAttr *SA = cast<NoInlineAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoInstrumentFunction: {
    const NoInstrumentFunctionAttr *SA = cast<NoInstrumentFunctionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoMips16: {
    const NoMips16Attr *SA = cast<NoMips16Attr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoReturn: {
    const NoReturnAttr *SA = cast<NoReturnAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoSanitizeAddress: {
    const NoSanitizeAddressAttr *SA = cast<NoSanitizeAddressAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoSanitizeMemory: {
    const NoSanitizeMemoryAttr *SA = cast<NoSanitizeMemoryAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoSanitizeThread: {
    const NoSanitizeThreadAttr *SA = cast<NoSanitizeThreadAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoSplitStack: {
    const NoSplitStackAttr *SA = cast<NoSplitStackAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoThreadSafetyAnalysis: {
    const NoThreadSafetyAnalysisAttr *SA = cast<NoThreadSafetyAnalysisAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NoThrow: {
    const NoThrowAttr *SA = cast<NoThrowAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::NonNull: {
    const NonNullAttr *SA = cast<NonNullAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      Record.push_back(Val);
    break;
  }
  case attr::OMPThreadPrivateDecl: {
    const OMPThreadPrivateDeclAttr *SA = cast<OMPThreadPrivateDeclAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCBridge: {
    const ObjCBridgeAttr *SA = cast<ObjCBridgeAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getBridgedType(), Record);
    break;
  }
  case attr::ObjCBridgeMutable: {
    const ObjCBridgeMutableAttr *SA = cast<ObjCBridgeMutableAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getBridgedType(), Record);
    break;
  }
  case attr::ObjCBridgeRelated: {
    const ObjCBridgeRelatedAttr *SA = cast<ObjCBridgeRelatedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getRelatedClass(), Record);
    AddIdentifierRef(SA->getClassMethod(), Record);
    AddIdentifierRef(SA->getInstanceMethod(), Record);
    break;
  }
  case attr::ObjCDesignatedInitializer: {
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCException: {
    const ObjCExceptionAttr *SA = cast<ObjCExceptionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCExplicitProtocolImpl: {
    const ObjCExplicitProtocolImplAttr *SA = cast<ObjCExplicitProtocolImplAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCMethodFamily: {
    const ObjCMethodFamilyAttr *SA = cast<ObjCMethodFamilyAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getFamily());
    break;
  }
  case attr::ObjCNSObject: {
    const ObjCNSObjectAttr *SA = cast<ObjCNSObjectAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCPreciseLifetime: {
    const ObjCPreciseLifetimeAttr *SA = cast<ObjCPreciseLifetimeAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCRequiresPropertyDefs: {
    const ObjCRequiresPropertyDefsAttr *SA = cast<ObjCRequiresPropertyDefsAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCRequiresSuper: {
    const ObjCRequiresSuperAttr *SA = cast<ObjCRequiresSuperAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCReturnsInnerPointer: {
    const ObjCReturnsInnerPointerAttr *SA = cast<ObjCReturnsInnerPointerAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCRootClass: {
    const ObjCRootClassAttr *SA = cast<ObjCRootClassAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ObjCRuntimeName: {
    const ObjCRuntimeNameAttr *SA = cast<ObjCRuntimeNameAttr>(A);
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getMetadataName(), Record);
    break;
  }
  case attr::OpenCLImageAccess: {
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::OpenCLKernel: {
    const OpenCLKernelAttr *SA = cast<OpenCLKernelAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::OptimizeNone: {
    const OptimizeNoneAttr *SA = cast<OptimizeNoneAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Overloadable: {
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Override: {
    const OverrideAttr *SA = cast<OverrideAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Ownership: {
    const OwnershipAttr *SA = cast<OwnershipAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getModule(), Record);
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      Record.push_back(Val);
    break;
  }
  case attr::Packed: {
    const PackedAttr *SA = cast<PackedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ParamTypestate: {
    const ParamTypestateAttr *SA = cast<ParamTypestateAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getParamState());
    break;
  }
  case attr::Pascal: {
    const PascalAttr *SA = cast<PascalAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Pcs: {
    const PcsAttr *SA = cast<PcsAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getPCS());
    break;
  }
  case attr::PnaclCall: {
    const PnaclCallAttr *SA = cast<PnaclCallAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::PtGuardedBy: {
    const PtGuardedByAttr *SA = cast<PtGuardedByAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getArg());
    break;
  }
  case attr::PtGuardedVar: {
    const PtGuardedVarAttr *SA = cast<PtGuardedVarAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Pure: {
    const PureAttr *SA = cast<PureAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ReleaseCapability: {
    const ReleaseCapabilityAttr *SA = cast<ReleaseCapabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::ReqdWorkGroupSize: {
    const ReqdWorkGroupSizeAttr *SA = cast<ReqdWorkGroupSizeAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getXDim());
    Record.push_back(SA->getYDim());
    Record.push_back(SA->getZDim());
    break;
  }
  case attr::RequiresCapability: {
    const RequiresCapabilityAttr *SA = cast<RequiresCapabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::ReturnTypestate: {
    const ReturnTypestateAttr *SA = cast<ReturnTypestateAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getState());
    break;
  }
  case attr::ReturnsNonNull: {
    const ReturnsNonNullAttr *SA = cast<ReturnsNonNullAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ReturnsTwice: {
    const ReturnsTwiceAttr *SA = cast<ReturnsTwiceAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::ScopedLockable: {
    const ScopedLockableAttr *SA = cast<ScopedLockableAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Section: {
    const SectionAttr *SA = cast<SectionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getName(), Record);
    break;
  }
  case attr::SelectAny: {
    const SelectAnyAttr *SA = cast<SelectAnyAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Sentinel: {
    const SentinelAttr *SA = cast<SentinelAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getSentinel());
    Record.push_back(SA->getNullPos());
    break;
  }
  case attr::SetTypestate: {
    const SetTypestateAttr *SA = cast<SetTypestateAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getNewState());
    break;
  }
  case attr::SharedTrylockFunction: {
    const SharedTrylockFunctionAttr *SA = cast<SharedTrylockFunctionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getSuccessValue());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::StdCall: {
    const StdCallAttr *SA = cast<StdCallAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::SysVABI: {
    const SysVABIAttr *SA = cast<SysVABIAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::TLSModel: {
    const TLSModelAttr *SA = cast<TLSModelAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getModel(), Record);
    break;
  }
  case attr::TestTypestate: {
    const TestTypestateAttr *SA = cast<TestTypestateAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getTestState());
    break;
  }
  case attr::ThisCall: {
    const ThisCallAttr *SA = cast<ThisCallAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Thread: {
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::TransparentUnion: {
    const TransparentUnionAttr *SA = cast<TransparentUnionAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::TryAcquireCapability: {
    const TryAcquireCapabilityAttr *SA = cast<TryAcquireCapabilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddStmt(SA->getSuccessValue());
    Record.push_back(SA->args_size());
    for (auto &Val : SA->args())
      AddStmt(Val);
    break;
  }
  case attr::TypeTagForDatatype: {
    const TypeTagForDatatypeAttr *SA = cast<TypeTagForDatatypeAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddIdentifierRef(SA->getArgumentKind(), Record);
    AddTypeSourceInfo(SA->getMatchingCTypeLoc(), Record);
    Record.push_back(SA->getLayoutCompatible());
    Record.push_back(SA->getMustBeNull());
    break;
  }
  case attr::TypeVisibility: {
    const TypeVisibilityAttr *SA = cast<TypeVisibilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getVisibility());
    break;
  }
  case attr::Unavailable: {
    const UnavailableAttr *SA = cast<UnavailableAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getMessage(), Record);
    break;
  }
  case attr::Unused: {
    const UnusedAttr *SA = cast<UnusedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Used: {
    const UsedAttr *SA = cast<UsedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Uuid: {
    const UuidAttr *SA = cast<UuidAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getGuid(), Record);
    break;
  }
  case attr::VecReturn: {
    const VecReturnAttr *SA = cast<VecReturnAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::VecTypeHint: {
    const VecTypeHintAttr *SA = cast<VecTypeHintAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddTypeSourceInfo(SA->getTypeHintLoc(), Record);
    break;
  }
  case attr::VectorCall: {
    const VectorCallAttr *SA = cast<VectorCallAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Visibility: {
    const VisibilityAttr *SA = cast<VisibilityAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
Record.push_back(SA->getVisibility());
    break;
  }
  case attr::WarnUnused: {
    const WarnUnusedAttr *SA = cast<WarnUnusedAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::WarnUnusedResult: {
    const WarnUnusedResultAttr *SA = cast<WarnUnusedResultAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::Weak: {
    const WeakAttr *SA = cast<WeakAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::WeakImport: {
    const WeakImportAttr *SA = cast<WeakImportAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  case attr::WeakRef: {
    const WeakRefAttr *SA = cast<WeakRefAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    AddString(SA->getAliasee(), Record);
    break;
  }
  case attr::WorkGroupSizeHint: {
    const WorkGroupSizeHintAttr *SA = cast<WorkGroupSizeHintAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    Record.push_back(SA->getXDim());
    Record.push_back(SA->getYDim());
    Record.push_back(SA->getZDim());
    break;
  }
  case attr::X86ForceAlignArgPointer: {
    const X86ForceAlignArgPointerAttr *SA = cast<X86ForceAlignArgPointerAttr>(A);
    Record.push_back(SA->isInherited());
    Record.push_back(A->isImplicit());
    Record.push_back(A->getSpellingListIndex());
    break;
  }
  }
