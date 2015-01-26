/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Attribute deserialization code                                              *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

  switch (Kind) {
  default:
    llvm_unreachable("Unknown attribute!");
  case attr::ARMInterrupt: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    ARMInterruptAttr::InterruptType interrupt(static_cast<ARMInterruptAttr::InterruptType>(Record[Idx++]));
    New = new (Context) ARMInterruptAttr(Range, Context, interrupt, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AcquireCapability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) AcquireCapabilityAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AcquiredAfter: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) AcquiredAfterAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AcquiredBefore: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) AcquiredBeforeAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Alias: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string aliasee= ReadString(Record, Idx);
    New = new (Context) AliasAttr(Range, Context, aliasee, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AlignMac68k: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) AlignMac68kAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AlignValue: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * alignment = ReadExpr(F);
    New = new (Context) AlignValueAttr(Range, Context, alignment, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Aligned: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    bool isalignmentExpr = Record[Idx++];
    void *alignmentPtr;
    if (isalignmentExpr)
      alignmentPtr = ReadExpr(F);
    else
      alignmentPtr = GetTypeSourceInfo(F, Record, Idx);
    New = new (Context) AlignedAttr(Range, Context, isalignmentExpr, alignmentPtr, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AlwaysInline: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) AlwaysInlineAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AnalyzerNoReturn: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) AnalyzerNoReturnAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Annotate: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string annotation= ReadString(Record, Idx);
    New = new (Context) AnnotateAttr(Range, Context, annotation, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ArcWeakrefUnavailable: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ArcWeakrefUnavailableAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ArgumentWithTypeTag: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * argumentKind = GetIdentifierInfo(F, Record, Idx);
    unsigned argumentIdx = Record[Idx++];
    unsigned typeTagIdx = Record[Idx++];
    bool isPointer = Record[Idx++];
    New = new (Context) ArgumentWithTypeTagAttr(Range, Context, argumentKind, argumentIdx, typeTagIdx, isPointer, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AsmLabel: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string label= ReadString(Record, Idx);
    New = new (Context) AsmLabelAttr(Range, Context, label, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AssertCapability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * expr = ReadExpr(F);
    New = new (Context) AssertCapabilityAttr(Range, Context, expr, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AssertExclusiveLock: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) AssertExclusiveLockAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AssertSharedLock: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) AssertSharedLockAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::AssumeAligned: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * alignment = ReadExpr(F);
    Expr * offset = ReadExpr(F);
    New = new (Context) AssumeAlignedAttr(Range, Context, alignment, offset, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Availability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * platform = GetIdentifierInfo(F, Record, Idx);
    VersionTuple introduced= ReadVersionTuple(Record, Idx);
    VersionTuple deprecated= ReadVersionTuple(Record, Idx);
    VersionTuple obsoleted= ReadVersionTuple(Record, Idx);
    bool unavailable = Record[Idx++];
    std::string message= ReadString(Record, Idx);
    New = new (Context) AvailabilityAttr(Range, Context, platform, introduced, deprecated, obsoleted, unavailable, message, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Blocks: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    BlocksAttr::BlockType type(static_cast<BlocksAttr::BlockType>(Record[Idx++]));
    New = new (Context) BlocksAttr(Range, Context, type, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::C11NoReturn: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) C11NoReturnAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CDecl: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CDeclAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CFAuditedTransfer: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CFAuditedTransferAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CFConsumed: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CFConsumedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CFReturnsNotRetained: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CFReturnsNotRetainedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CFReturnsRetained: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CFReturnsRetainedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CFUnknownTransfer: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CFUnknownTransferAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDAConstant: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CUDAConstantAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDADevice: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CUDADeviceAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDAGlobal: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CUDAGlobalAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDAHost: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CUDAHostAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDAInvalidTarget: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CUDAInvalidTargetAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDALaunchBounds: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    int maxThreads = Record[Idx++];
    int minBlocks = Record[Idx++];
    New = new (Context) CUDALaunchBoundsAttr(Range, Context, maxThreads, minBlocks, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CUDAShared: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CUDASharedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CXX11NoReturn: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CXX11NoReturnAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CallableWhen: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned callableStatesSize = Record[Idx++];
    SmallVector<CallableWhenAttr::ConsumedState, 4> callableStates;
    callableStates.reserve(callableStatesSize);
    for (unsigned i = callableStatesSize; i; --i)
      callableStates.push_back(static_cast<CallableWhenAttr::ConsumedState>(Record[Idx++]));
    New = new (Context) CallableWhenAttr(Range, Context, callableStates.data(), callableStatesSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Capability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string name= ReadString(Record, Idx);
    New = new (Context) CapabilityAttr(Range, Context, name, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CapturedRecord: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CapturedRecordAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::CarriesDependency: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CarriesDependencyAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Cleanup: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    FunctionDecl * functionDecl = GetLocalDeclAs<FunctionDecl >(F, Record[Idx++]);
    New = new (Context) CleanupAttr(Range, Context, functionDecl, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Cold: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ColdAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Common: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) CommonAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Const: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ConstAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Constructor: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    int priority = Record[Idx++];
    New = new (Context) ConstructorAttr(Range, Context, priority, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Consumable: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    ConsumableAttr::ConsumedState defaultState(static_cast<ConsumableAttr::ConsumedState>(Record[Idx++]));
    New = new (Context) ConsumableAttr(Range, Context, defaultState, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ConsumableAutoCast: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ConsumableAutoCastAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ConsumableSetOnRead: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ConsumableSetOnReadAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::DLLExport: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) DLLExportAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::DLLImport: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) DLLImportAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Deprecated: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string message= ReadString(Record, Idx);
    New = new (Context) DeprecatedAttr(Range, Context, message, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Destructor: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    int priority = Record[Idx++];
    New = new (Context) DestructorAttr(Range, Context, priority, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::EnableIf: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * cond = ReadExpr(F);
    std::string message= ReadString(Record, Idx);
    New = new (Context) EnableIfAttr(Range, Context, cond, message, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ExclusiveTrylockFunction: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * successValue = ReadExpr(F);
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) ExclusiveTrylockFunctionAttr(Range, Context, successValue, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::FallThrough: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) FallThroughAttr(Range, Context, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::FastCall: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) FastCallAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Final: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) FinalAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Flatten: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) FlattenAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Format: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * type = GetIdentifierInfo(F, Record, Idx);
    int formatIdx = Record[Idx++];
    int firstArg = Record[Idx++];
    New = new (Context) FormatAttr(Range, Context, type, formatIdx, firstArg, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::FormatArg: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    int formatIdx = Record[Idx++];
    New = new (Context) FormatArgAttr(Range, Context, formatIdx, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::GNUInline: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) GNUInlineAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::GuardedBy: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * arg = ReadExpr(F);
    New = new (Context) GuardedByAttr(Range, Context, arg, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::GuardedVar: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) GuardedVarAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Hot: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) HotAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::IBAction: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) IBActionAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::IBOutlet: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) IBOutletAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::IBOutletCollection: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    TypeSourceInfo * interface = GetTypeSourceInfo(F, Record, Idx);
    New = new (Context) IBOutletCollectionAttr(Range, Context, interface, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::InitPriority: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned priority = Record[Idx++];
    New = new (Context) InitPriorityAttr(Range, Context, priority, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::InitSeg: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string section= ReadString(Record, Idx);
    New = new (Context) InitSegAttr(Range, Context, section, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::IntelOclBicc: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) IntelOclBiccAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::LockReturned: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * arg = ReadExpr(F);
    New = new (Context) LockReturnedAttr(Range, Context, arg, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::LocksExcluded: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) LocksExcludedAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::LoopHint: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    LoopHintAttr::OptionType option(static_cast<LoopHintAttr::OptionType>(Record[Idx++]));
    LoopHintAttr::LoopHintState state(static_cast<LoopHintAttr::LoopHintState>(Record[Idx++]));
    Expr * value = ReadExpr(F);
    New = new (Context) LoopHintAttr(Range, Context, option, state, value, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MSABI: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) MSABIAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MSInheritance: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    bool bestCase = Record[Idx++];
    New = new (Context) MSInheritanceAttr(Range, Context, bestCase, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MSP430Interrupt: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned number = Record[Idx++];
    New = new (Context) MSP430InterruptAttr(Range, Context, number, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MSVtorDisp: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned vdm = Record[Idx++];
    New = new (Context) MSVtorDispAttr(Range, Context, vdm, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Malloc: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) MallocAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MaxFieldAlignment: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned alignment = Record[Idx++];
    New = new (Context) MaxFieldAlignmentAttr(Range, Context, alignment, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MayAlias: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) MayAliasAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MinSize: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) MinSizeAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Mips16: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) Mips16Attr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Mode: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * mode = GetIdentifierInfo(F, Record, Idx);
    New = new (Context) ModeAttr(Range, Context, mode, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::MsStruct: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) MsStructAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NSConsumed: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NSConsumedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NSConsumesSelf: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NSConsumesSelfAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NSReturnsAutoreleased: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NSReturnsAutoreleasedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NSReturnsNotRetained: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NSReturnsNotRetainedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NSReturnsRetained: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NSReturnsRetainedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Naked: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NakedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoCommon: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoCommonAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoDebug: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoDebugAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoDuplicate: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoDuplicateAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoInline: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoInlineAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoInstrumentFunction: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoInstrumentFunctionAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoMips16: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoMips16Attr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoReturn: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoReturnAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoSanitizeAddress: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoSanitizeAddressAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoSanitizeMemory: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoSanitizeMemoryAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoSanitizeThread: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoSanitizeThreadAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoSplitStack: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoSplitStackAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoThreadSafetyAnalysis: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoThreadSafetyAnalysisAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NoThrow: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) NoThrowAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::NonNull: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<unsigned, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(Record[Idx++]);
    New = new (Context) NonNullAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::OMPThreadPrivateDecl: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) OMPThreadPrivateDeclAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCBridge: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * bridgedType = GetIdentifierInfo(F, Record, Idx);
    New = new (Context) ObjCBridgeAttr(Range, Context, bridgedType, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCBridgeMutable: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * bridgedType = GetIdentifierInfo(F, Record, Idx);
    New = new (Context) ObjCBridgeMutableAttr(Range, Context, bridgedType, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCBridgeRelated: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * relatedClass = GetIdentifierInfo(F, Record, Idx);
    IdentifierInfo * classMethod = GetIdentifierInfo(F, Record, Idx);
    IdentifierInfo * instanceMethod = GetIdentifierInfo(F, Record, Idx);
    New = new (Context) ObjCBridgeRelatedAttr(Range, Context, relatedClass, classMethod, instanceMethod, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCDesignatedInitializer: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCDesignatedInitializerAttr(Range, Context, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCException: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCExceptionAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCExplicitProtocolImpl: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCExplicitProtocolImplAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCMethodFamily: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    ObjCMethodFamilyAttr::FamilyKind family(static_cast<ObjCMethodFamilyAttr::FamilyKind>(Record[Idx++]));
    New = new (Context) ObjCMethodFamilyAttr(Range, Context, family, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCNSObject: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCNSObjectAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCPreciseLifetime: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCPreciseLifetimeAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCRequiresPropertyDefs: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCRequiresPropertyDefsAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCRequiresSuper: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCRequiresSuperAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCReturnsInnerPointer: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCReturnsInnerPointerAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCRootClass: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ObjCRootClassAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ObjCRuntimeName: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string metadataName= ReadString(Record, Idx);
    New = new (Context) ObjCRuntimeNameAttr(Range, Context, metadataName, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::OpenCLImageAccess: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) OpenCLImageAccessAttr(Range, Context, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::OpenCLKernel: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) OpenCLKernelAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::OptimizeNone: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) OptimizeNoneAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Overloadable: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) OverloadableAttr(Range, Context, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Override: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) OverrideAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Ownership: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * module = GetIdentifierInfo(F, Record, Idx);
  unsigned argsSize = Record[Idx++];
  SmallVector<unsigned, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(Record[Idx++]);
    New = new (Context) OwnershipAttr(Range, Context, module, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Packed: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) PackedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ParamTypestate: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    ParamTypestateAttr::ConsumedState paramState(static_cast<ParamTypestateAttr::ConsumedState>(Record[Idx++]));
    New = new (Context) ParamTypestateAttr(Range, Context, paramState, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Pascal: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) PascalAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Pcs: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    PcsAttr::PCSType pCS(static_cast<PcsAttr::PCSType>(Record[Idx++]));
    New = new (Context) PcsAttr(Range, Context, pCS, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::PnaclCall: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) PnaclCallAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::PtGuardedBy: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * arg = ReadExpr(F);
    New = new (Context) PtGuardedByAttr(Range, Context, arg, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::PtGuardedVar: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) PtGuardedVarAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Pure: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) PureAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ReleaseCapability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) ReleaseCapabilityAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ReqdWorkGroupSize: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned xDim = Record[Idx++];
    unsigned yDim = Record[Idx++];
    unsigned zDim = Record[Idx++];
    New = new (Context) ReqdWorkGroupSizeAttr(Range, Context, xDim, yDim, zDim, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::RequiresCapability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) RequiresCapabilityAttr(Range, Context, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ReturnTypestate: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    ReturnTypestateAttr::ConsumedState state(static_cast<ReturnTypestateAttr::ConsumedState>(Record[Idx++]));
    New = new (Context) ReturnTypestateAttr(Range, Context, state, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ReturnsNonNull: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ReturnsNonNullAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ReturnsTwice: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ReturnsTwiceAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ScopedLockable: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ScopedLockableAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Section: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string name= ReadString(Record, Idx);
    New = new (Context) SectionAttr(Range, Context, name, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::SelectAny: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) SelectAnyAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Sentinel: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    int sentinel = Record[Idx++];
    int nullPos = Record[Idx++];
    New = new (Context) SentinelAttr(Range, Context, sentinel, nullPos, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::SetTypestate: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    SetTypestateAttr::ConsumedState newState(static_cast<SetTypestateAttr::ConsumedState>(Record[Idx++]));
    New = new (Context) SetTypestateAttr(Range, Context, newState, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::SharedTrylockFunction: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * successValue = ReadExpr(F);
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) SharedTrylockFunctionAttr(Range, Context, successValue, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::StdCall: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) StdCallAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::SysVABI: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) SysVABIAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::TLSModel: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string model= ReadString(Record, Idx);
    New = new (Context) TLSModelAttr(Range, Context, model, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::TestTypestate: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    TestTypestateAttr::ConsumedState testState(static_cast<TestTypestateAttr::ConsumedState>(Record[Idx++]));
    New = new (Context) TestTypestateAttr(Range, Context, testState, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::ThisCall: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ThisCallAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Thread: {
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) ThreadAttr(Range, Context, Spelling);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::TransparentUnion: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) TransparentUnionAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::TryAcquireCapability: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    Expr * successValue = ReadExpr(F);
  unsigned argsSize = Record[Idx++];
  SmallVector<Expr *, 4> args;
  args.reserve(argsSize);
    for (unsigned i = argsSize; i; --i)
    args.push_back(ReadExpr(F));
    New = new (Context) TryAcquireCapabilityAttr(Range, Context, successValue, args.data(), argsSize, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::TypeTagForDatatype: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    IdentifierInfo * argumentKind = GetIdentifierInfo(F, Record, Idx);
    TypeSourceInfo * matchingCType = GetTypeSourceInfo(F, Record, Idx);
    bool layoutCompatible = Record[Idx++];
    bool mustBeNull = Record[Idx++];
    New = new (Context) TypeTagForDatatypeAttr(Range, Context, argumentKind, matchingCType, layoutCompatible, mustBeNull, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::TypeVisibility: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    TypeVisibilityAttr::VisibilityType visibility(static_cast<TypeVisibilityAttr::VisibilityType>(Record[Idx++]));
    New = new (Context) TypeVisibilityAttr(Range, Context, visibility, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Unavailable: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string message= ReadString(Record, Idx);
    New = new (Context) UnavailableAttr(Range, Context, message, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Unused: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) UnusedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Used: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) UsedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Uuid: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string guid= ReadString(Record, Idx);
    New = new (Context) UuidAttr(Range, Context, guid, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::VecReturn: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) VecReturnAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::VecTypeHint: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    TypeSourceInfo * typeHint = GetTypeSourceInfo(F, Record, Idx);
    New = new (Context) VecTypeHintAttr(Range, Context, typeHint, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::VectorCall: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) VectorCallAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Visibility: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    VisibilityAttr::VisibilityType visibility(static_cast<VisibilityAttr::VisibilityType>(Record[Idx++]));
    New = new (Context) VisibilityAttr(Range, Context, visibility, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::WarnUnused: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) WarnUnusedAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::WarnUnusedResult: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) WarnUnusedResultAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::Weak: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) WeakAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::WeakImport: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) WeakImportAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::WeakRef: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    std::string aliasee= ReadString(Record, Idx);
    New = new (Context) WeakRefAttr(Range, Context, aliasee, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::WorkGroupSizeHint: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    unsigned xDim = Record[Idx++];
    unsigned yDim = Record[Idx++];
    unsigned zDim = Record[Idx++];
    New = new (Context) WorkGroupSizeHintAttr(Range, Context, xDim, yDim, zDim, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  case attr::X86ForceAlignArgPointer: {
    bool isInherited = Record[Idx++];
    bool isImplicit = Record[Idx++];
    unsigned Spelling = Record[Idx++];
    New = new (Context) X86ForceAlignArgPointerAttr(Range, Context, Spelling);
    cast<InheritableAttr>(New)->setInherited(isInherited);
    New->setImplicit(isImplicit);
    break;
  }
  }
