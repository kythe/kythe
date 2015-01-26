/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Parsed attribute helpers                                                    *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

static bool defaultAppertainsTo(Sema &, const AttributeList &,const Decl *) {
  return true;
}

static bool defaultDiagnoseLangOpts(Sema &, const AttributeList &) {
  return true;
}

static bool defaultTargetRequirements(const llvm::Triple &) {
  return true;
}

static unsigned defaultSpellingIndexToSemanticSpelling(const AttributeList &Attr) {
  return UINT_MAX;
}

static bool isTargetarmthumbmsp430(const llvm::Triple &T) {
  llvm::Triple::ArchType Arch = T.getArch();
  return (Arch == llvm::Triple::arm || Arch == llvm::Triple::thumb || Arch == llvm::Triple::msp430);
}

static bool checkAcquireCapabilityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned AcquireCapabilityAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_acquire_capability,
    CXX11_clang_acquire_capability,
    GNU_acquire_shared_capability,
    CXX11_clang_acquire_shared_capability,
    GNU_exclusive_lock_function,
    GNU_shared_lock_function
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_acquire_capability;
    case 1: return CXX11_clang_acquire_capability;
    case 2: return GNU_acquire_shared_capability;
    case 3: return CXX11_clang_acquire_shared_capability;
    case 4: return GNU_exclusive_lock_function;
    case 5: return GNU_shared_lock_function;
  }
}

static bool isSharedVar(const Decl *D) {
  if (const VarDecl *S = dyn_cast<VarDecl>(D))
    return S->hasGlobalStorage() && !S->getTLSKind();
  return false;
}

static bool checkAcquiredAfterAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FieldDecl>(D) && !isSharedVar(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFieldOrGlobalVar;
    return false;
  }
  return true;
}

static bool checkAcquiredBeforeAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FieldDecl>(D) && !isSharedVar(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFieldOrGlobalVar;
    return false;
  }
  return true;
}

static bool checkAlignValueAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D) && !isa<TypedefNameDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariableOrTypedef;
    return false;
  }
  return true;
}

static unsigned AlignedAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_aligned,
    CXX11_gnu_aligned,
    Declspec_align,
    Keyword_alignas,
    Keyword_Alignas
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_aligned;
    case 1: return CXX11_gnu_aligned;
    case 2: return Declspec_align;
    case 3: return Keyword_alignas;
    case 4: return Keyword_Alignas;
  }
}

static bool checkAlwaysInlineAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned AlwaysInlineAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_always_inline,
    CXX11_gnu_always_inline,
    Keyword_forceinline
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_always_inline;
    case 1: return CXX11_gnu_always_inline;
    case 2: return Keyword_forceinline;
  }
}

static bool checkArcWeakrefUnavailableAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCInterfaceDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjectiveCInterface;
    return false;
  }
  return true;
}

static unsigned ArgumentWithTypeTagAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_argument_with_type_tag,
    GNU_pointer_with_type_tag
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_argument_with_type_tag;
    case 1: return GNU_pointer_with_type_tag;
  }
}

static bool checkAssertCapabilityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned AssertCapabilityAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_assert_capability,
    CXX11_clang_assert_capability,
    GNU_assert_shared_capability,
    CXX11_clang_assert_shared_capability
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_assert_capability;
    case 1: return CXX11_clang_assert_capability;
    case 2: return GNU_assert_shared_capability;
    case 3: return CXX11_clang_assert_shared_capability;
  }
}

static bool checkAssertExclusiveLockAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkAssertSharedLockAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkAssumeAlignedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return false;
  }
  return true;
}

static bool checkCFAuditedTransferAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkCFConsumedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ParmVarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedParameter;
    return false;
  }
  return true;
}

static bool checkCFUnknownTransferAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkCUDAConstantAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkCUDALangOpts(Sema &S, const AttributeList &Attr) {
  if (S.LangOpts.CUDA)
    return true;

  S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
  return false;
}

static bool checkCUDADeviceAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariableOrFunction;
    return false;
  }
  return true;
}

static bool checkCUDAGlobalAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkCUDAHostAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkCUDAInvalidTargetAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool isFunctionLike(const Decl *D) {
  if (const Decl *S = dyn_cast<Decl>(D))
    return S->getFunctionType(false) != NULL;
  return false;
}

static bool checkCUDALaunchBoundsAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isFunctionLike(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return false;
  }
  return true;
}

static bool checkCUDASharedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkCXX11NoReturnAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkCallableWhenAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethod;
    return false;
  }
  return true;
}

static bool isStruct(const Decl *D) {
  if (const RecordDecl *S = dyn_cast<RecordDecl>(D))
    return !S->isUnion();
  return false;
}

static bool checkCapabilityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isStruct(D) && !isa<TypedefNameDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedStructOrTypedef;
    return false;
  }
  return true;
}

static unsigned CapabilityAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_capability,
    CXX11_clang_capability,
    GNU_shared_capability,
    CXX11_clang_shared_capability
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_capability;
    case 1: return CXX11_clang_capability;
    case 2: return GNU_shared_capability;
    case 3: return CXX11_clang_shared_capability;
  }
}

static bool checkCarriesDependencyAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ParmVarDecl>(D) && !isa<ObjCMethodDecl>(D) && !isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionMethodOrParameter;
    return false;
  }
  return true;
}

static bool checkCleanupAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkColdAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkCommonAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkConstructorAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkConsumableAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedClass;
    return false;
  }
  return true;
}

static bool checkConsumableAutoCastAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedClass;
    return false;
  }
  return true;
}

static bool checkConsumableSetOnReadAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedClass;
    return false;
  }
  return true;
}

static bool checkDLLExportAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isa<VarDecl>(D) && !isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedFunctionVariableOrClass : ExpectedVariableOrFunction);
    return false;
  }
  return true;
}

static bool isTargetx86x86_64armthumbWin32(const llvm::Triple &T) {
  llvm::Triple::ArchType Arch = T.getArch();
  llvm::Triple::OSType OS = T.getOS();
  return (Arch == llvm::Triple::x86 || Arch == llvm::Triple::x86_64 || Arch == llvm::Triple::arm || Arch == llvm::Triple::thumb) && (OS == llvm::Triple::Win32);
}

static bool checkDLLImportAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isa<VarDecl>(D) && !isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedFunctionVariableOrClass : ExpectedVariableOrFunction);
    return false;
  }
  return true;
}

static bool checkDestructorAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkEnableIfAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkExclusiveTrylockFunctionAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkExtVectorTypeAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<TypedefNameDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedType;
    return false;
  }
  return true;
}

static bool checkFlattenAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool isHasFunctionProto(const Decl *D) {
  if (const Decl *S = dyn_cast<Decl>(D))
    return (S->getFunctionType(true) != NULL &&
                              isa<FunctionProtoType>(S->getFunctionType())) ||
                                       isa<ObjCMethodDecl>(S) ||
                                       isa<BlockDecl>(S);
  return false;
}

static bool checkFormatAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isa<BlockDecl>(D) && !isHasFunctionProto(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkFormatArgAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isHasFunctionProto(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkGNUInlineAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkGuardedByAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FieldDecl>(D) && !isSharedVar(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFieldOrGlobalVar;
    return false;
  }
  return true;
}

static bool checkGuardedVarAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FieldDecl>(D) && !isSharedVar(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFieldOrGlobalVar;
    return false;
  }
  return true;
}

static bool checkHotAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool isObjCInstanceMethod(const Decl *D) {
  if (const ObjCMethodDecl *S = dyn_cast<ObjCMethodDecl>(D))
    return S->isInstanceMethod();
  return false;
}

static bool checkIBActionAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isObjCInstanceMethod(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjCInstanceMethod;
    return false;
  }
  return true;
}

static bool checkInitPriorityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkLockReturnedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkLockableAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool checkLocksExcludedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned LoopHintAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    Pragma_clang_loop,
    Pragma_unroll,
    Pragma_nounroll
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Pragma_clang_loop;
    case 1: return Pragma_unroll;
    case 2: return Pragma_nounroll;
  }
}

static bool checkMicrosoftExtLangOpts(Sema &S, const AttributeList &Attr) {
  if (S.LangOpts.MicrosoftExt)
    return true;

  S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
  return false;
}

static unsigned MSInheritanceAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    Keyword_single_inheritance,
    Keyword_multiple_inheritance,
    Keyword_virtual_inheritance,
    Keyword_unspecified_inheritance
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Keyword_single_inheritance;
    case 1: return Keyword_multiple_inheritance;
    case 2: return Keyword_virtual_inheritance;
    case 3: return Keyword_unspecified_inheritance;
  }
}

static bool checkMinSizeAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isa<ObjCMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return false;
  }
  return true;
}

static bool checkMips16AppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool isTargetmipsmipsel(const llvm::Triple &T) {
  llvm::Triple::ArchType Arch = T.getArch();
  return (Arch == llvm::Triple::mips || Arch == llvm::Triple::mipsel);
}

static bool checkMsStructAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool checkNSConsumedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ParmVarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedParameter;
    return false;
  }
  return true;
}

static bool checkNSConsumesSelfAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethod;
    return false;
  }
  return true;
}

static bool checkNakedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoCommonAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkNoDuplicateAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoInlineAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoInstrumentFunctionAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoMips16AppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoSanitizeAddressAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned NoSanitizeAddressAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_no_address_safety_analysis,
    CXX11_gnu_no_address_safety_analysis,
    GNU_no_sanitize_address,
    CXX11_gnu_no_sanitize_address
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_no_address_safety_analysis;
    case 1: return CXX11_gnu_no_address_safety_analysis;
    case 2: return GNU_no_sanitize_address;
    case 3: return CXX11_gnu_no_sanitize_address;
  }
}

static bool checkNoSanitizeMemoryAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoSanitizeThreadAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoSplitStackAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNoThreadSafetyAnalysisAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkNonNullAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isHasFunctionProto(D) && !isa<ParmVarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionMethodOrParameter;
    return false;
  }
  return true;
}

static bool checkObjCBridgeAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool checkObjCBridgeMutableAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool checkObjCBridgeRelatedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool isObjCInterfaceDeclInitMethod(const Decl *D) {
  if (const ObjCMethodDecl *S = dyn_cast<ObjCMethodDecl>(D))
    return S->getMethodFamily() == OMF_init &&
                                 (isa<ObjCInterfaceDecl>(S->getDeclContext()) ||
                                  (isa<ObjCCategoryDecl>(S->getDeclContext()) &&
            cast<ObjCCategoryDecl>(S->getDeclContext())->IsClassExtension()));
  return false;
}

static bool checkObjCDesignatedInitializerAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isObjCInterfaceDeclInitMethod(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjCInterfaceDeclInitMethod;
    return false;
  }
  return true;
}

static bool checkObjCExceptionAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCInterfaceDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjectiveCInterface;
    return false;
  }
  return true;
}

static bool checkObjCExplicitProtocolImplAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCProtocolDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjectiveCProtocol;
    return false;
  }
  return true;
}

static bool checkObjCMethodFamilyAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethod;
    return false;
  }
  return true;
}

static bool checkObjCPreciseLifetimeAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkObjCRequiresPropertyDefsAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCInterfaceDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjectiveCInterface;
    return false;
  }
  return true;
}

static bool checkObjCRequiresSuperAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethod;
    return false;
  }
  return true;
}

static bool checkObjCReturnsInnerPointerAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isa<ObjCPropertyDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethodOrProperty;
    return false;
  }
  return true;
}

static bool checkObjCRootClassAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCInterfaceDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjectiveCInterface;
    return false;
  }
  return true;
}

static bool checkObjCRuntimeNameAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCInterfaceDecl>(D) && !isa<ObjCProtocolDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedObjectiveCInterfaceOrProtocol;
    return false;
  }
  return true;
}

static bool checkOpenCLImageAccessAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ParmVarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedParameter;
    return false;
  }
  return true;
}

static unsigned OpenCLImageAccessAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    Keyword_read_only,
    Keyword_write_only,
    Keyword_read_write
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Keyword_read_only;
    case 1: return Keyword_read_only;
    case 2: return Keyword_write_only;
    case 3: return Keyword_write_only;
    case 4: return Keyword_read_write;
    case 5: return Keyword_read_write;
  }
}

static bool checkOpenCLKernelAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkOptimizeNoneAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isa<ObjCMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return false;
  }
  return true;
}

static bool checkOverloadableAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkOwnershipAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isHasFunctionProto(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned OwnershipAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_ownership_holds,
    GNU_ownership_returns,
    GNU_ownership_takes
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_ownership_holds;
    case 1: return GNU_ownership_returns;
    case 2: return GNU_ownership_takes;
  }
}

static bool checkParamTypestateAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ParmVarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedParameter;
    return false;
  }
  return true;
}

static bool checkPtGuardedByAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FieldDecl>(D) && !isSharedVar(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFieldOrGlobalVar;
    return false;
  }
  return true;
}

static bool checkPtGuardedVarAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FieldDecl>(D) && !isSharedVar(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFieldOrGlobalVar;
    return false;
  }
  return true;
}

static bool checkReleaseCapabilityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned ReleaseCapabilityAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_release_capability,
    CXX11_clang_release_capability,
    GNU_release_shared_capability,
    CXX11_clang_release_shared_capability,
    GNU_release_generic_capability,
    CXX11_clang_release_generic_capability,
    GNU_unlock_function
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_release_capability;
    case 1: return CXX11_clang_release_capability;
    case 2: return GNU_release_shared_capability;
    case 3: return CXX11_clang_release_shared_capability;
    case 4: return GNU_release_generic_capability;
    case 5: return CXX11_clang_release_generic_capability;
    case 6: return GNU_unlock_function;
  }
}

static bool checkReqdWorkGroupSizeAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkRequiresCapabilityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned RequiresCapabilityAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_requires_capability,
    CXX11_clang_requires_capability,
    GNU_exclusive_locks_required,
    GNU_requires_shared_capability,
    CXX11_clang_requires_shared_capability,
    GNU_shared_locks_required
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_requires_capability;
    case 1: return CXX11_clang_requires_capability;
    case 2: return GNU_exclusive_locks_required;
    case 3: return GNU_requires_shared_capability;
    case 4: return CXX11_clang_requires_shared_capability;
    case 5: return GNU_shared_locks_required;
  }
}

static bool checkReturnTypestateAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isa<ParmVarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionMethodOrParameter;
    return false;
  }
  return true;
}

static bool checkReturnsNonNullAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return false;
  }
  return true;
}

static bool checkReturnsTwiceAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkScopedLockableAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool isGlobalVar(const Decl *D) {
  if (const VarDecl *S = dyn_cast<VarDecl>(D))
    return S->hasGlobalStorage();
  return false;
}

static bool checkSectionAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D) && !isGlobalVar(D) && !isa<ObjCMethodDecl>(D) && !isa<ObjCPropertyDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionGlobalVarMethodOrProperty;
    return false;
  }
  return true;
}

static unsigned SectionAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_section,
    CXX11_gnu_section,
    Declspec_allocate
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_section;
    case 1: return CXX11_gnu_section;
    case 2: return Declspec_allocate;
  }
}

static bool checkSetTypestateAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethod;
    return false;
  }
  return true;
}

static bool checkSharedTrylockFunctionAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool isTLSVar(const Decl *D) {
  if (const VarDecl *S = dyn_cast<VarDecl>(D))
    return S->getTLSKind() != 0;
  return false;
}

static bool checkTLSModelAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isTLSVar(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedTLSVar;
    return false;
  }
  return true;
}

static bool checkTestTypestateAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedMethod;
    return false;
  }
  return true;
}

static bool checkThreadAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return false;
  }
  return true;
}

static bool checkTryAcquireCapabilityAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static unsigned TryAcquireCapabilityAttrSpellingMap(const AttributeList &Attr) {
  enum Spelling {
    GNU_try_acquire_capability,
    CXX11_clang_try_acquire_capability,
    GNU_try_acquire_shared_capability,
    CXX11_clang_try_acquire_shared_capability
  };

  unsigned Idx = Attr.getAttributeSpellingListIndex();
  switch (Idx) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_try_acquire_capability;
    case 1: return CXX11_clang_try_acquire_capability;
    case 2: return GNU_try_acquire_shared_capability;
    case 3: return CXX11_clang_try_acquire_shared_capability;
  }
}

static bool checkUnusedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D) && !isa<ObjCIvarDecl>(D) && !isa<TypeDecl>(D) && !isa<LabelDecl>(D) && !isa<FieldDecl>(D) && !isa<ObjCMethodDecl>(D) && !isFunctionLike(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariableFunctionOrLabel;
    return false;
  }
  return true;
}

static bool checkMicrosoftExtBorlandLangOpts(Sema &S, const AttributeList &Attr) {
  if (S.LangOpts.MicrosoftExt || S.LangOpts.Borland)
    return true;

  S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
  return false;
}

static bool checkVecReturnAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedClass;
    return false;
  }
  return true;
}

static bool checkVecTypeHintAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool checkWarnUnusedAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<RecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedStructOrUnionOrClass : ExpectedStructOrUnion);
    return false;
  }
  return true;
}

static bool checkWarnUnusedResultAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<ObjCMethodDecl>(D) && !isa<CXXRecordDecl>(D) && !isFunctionLike(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionMethodOrClass;
    return false;
  }
  return true;
}

static bool checkWeakAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D) && !isa<FunctionDecl>(D) && !isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << (S.getLangOpts().CPlusPlus ? ExpectedFunctionVariableOrClass : ExpectedVariableOrFunction);
    return false;
  }
  return true;
}

static bool checkWeakRefAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<VarDecl>(D) && !isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariableOrFunction;
    return false;
  }
  return true;
}

static bool checkWorkGroupSizeHintAppertainsTo(Sema &S, const AttributeList &Attr, const Decl *D) {
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunction;
    return false;
  }
  return true;
}

static bool isTargetx86(const llvm::Triple &T) {
  llvm::Triple::ArchType Arch = T.getArch();
  return (Arch == llvm::Triple::x86);
}

static const ParsedAttrInfo AttrInfoMap[AttributeList::UnknownAttribute + 1] = {
  { 0, 1, 1, 1, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, isTargetarmthumbmsp430, defaultSpellingIndexToSemanticSpelling },  // AT_Interrupt
  { 0, 15, 0, 0, 0, 0, checkAcquireCapabilityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, AcquireCapabilityAttrSpellingMap },  // AT_AcquireCapability
  { 0, 15, 0, 0, 0, 0, checkAcquiredAfterAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AcquiredAfter
  { 0, 15, 0, 0, 0, 0, checkAcquiredBeforeAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AcquiredBefore
  { 1, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AddressSpace
  { 1, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Alias
  { 1, 0, 0, 0, 0, 0, checkAlignValueAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AlignValue
  { 0, 1, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, AlignedAttrSpellingMap },  // AT_Aligned
  { 0, 0, 0, 0, 0, 1, checkAlwaysInlineAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, AlwaysInlineAttrSpellingMap },  // AT_AlwaysInline
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AnalyzerNoReturn
  { 1, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Annotate
  { 0, 0, 0, 0, 0, 0, checkArcWeakrefUnavailableAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ArcWeakrefUnavailable
  { 4, 0, 1, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, ArgumentWithTypeTagAttrSpellingMap },  // AT_ArgumentWithTypeTag
  { 1, 0, 0, 0, 0, 0, checkAssertCapabilityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, AssertCapabilityAttrSpellingMap },  // AT_AssertCapability
  { 0, 15, 0, 0, 0, 0, checkAssertExclusiveLockAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AssertExclusiveLock
  { 0, 15, 0, 0, 0, 0, checkAssertSharedLockAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AssertSharedLock
  { 1, 1, 0, 0, 0, 1, checkAssumeAlignedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_AssumeAligned
  { 6, 0, 1, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Availability
  { 1, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Blocks
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CDecl
  { 0, 0, 0, 0, 0, 0, checkCFAuditedTransferAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CFAuditedTransfer
  { 0, 0, 0, 0, 0, 0, checkCFConsumedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CFConsumed
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CFReturnsNotRetained
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CFReturnsRetained
  { 0, 0, 0, 0, 0, 0, checkCFUnknownTransferAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CFUnknownTransfer
  { 0, 0, 0, 0, 0, 0, checkCUDAConstantAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDAConstant
  { 0, 0, 0, 0, 0, 0, checkCUDADeviceAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDADevice
  { 0, 0, 0, 0, 0, 0, checkCUDAGlobalAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDAGlobal
  { 0, 0, 0, 0, 0, 0, checkCUDAHostAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDAHost
  { 0, 0, 0, 0, 0, 0, checkCUDAInvalidTargetAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDAInvalidTarget
  { 1, 1, 0, 0, 0, 0, checkCUDALaunchBoundsAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDALaunchBounds
  { 0, 0, 0, 0, 0, 0, checkCUDASharedAppertainsTo, checkCUDALangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CUDAShared
  { 0, 0, 0, 0, 0, 0, checkCXX11NoReturnAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CXX11NoReturn
  { 0, 15, 0, 0, 0, 0, checkCallableWhenAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CallableWhen
  { 1, 0, 0, 0, 0, 0, checkCapabilityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, CapabilityAttrSpellingMap },  // AT_Capability
  { 0, 0, 0, 0, 0, 0, checkCarriesDependencyAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_CarriesDependency
  { 1, 0, 0, 0, 0, 1, checkCleanupAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Cleanup
  { 0, 0, 0, 0, 0, 1, checkColdAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Cold
  { 0, 0, 0, 0, 0, 1, checkCommonAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Common
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Const
  { 0, 1, 0, 0, 0, 1, checkConstructorAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Constructor
  { 1, 0, 0, 0, 0, 0, checkConsumableAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Consumable
  { 0, 0, 0, 0, 0, 0, checkConsumableAutoCastAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ConsumableAutoCast
  { 0, 0, 0, 0, 0, 0, checkConsumableSetOnReadAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ConsumableSetOnRead
  { 0, 0, 0, 1, 0, 1, checkDLLExportAppertainsTo, defaultDiagnoseLangOpts, isTargetx86x86_64armthumbWin32, defaultSpellingIndexToSemanticSpelling },  // AT_DLLExport
  { 0, 0, 0, 1, 0, 1, checkDLLImportAppertainsTo, defaultDiagnoseLangOpts, isTargetx86x86_64armthumbWin32, defaultSpellingIndexToSemanticSpelling },  // AT_DLLImport
  { 0, 1, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Deprecated
  { 0, 1, 0, 0, 0, 1, checkDestructorAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Destructor
  { 2, 0, 0, 0, 0, 0, checkEnableIfAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_EnableIf
  { 1, 15, 0, 0, 0, 0, checkExclusiveTrylockFunctionAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ExclusiveTrylockFunction
  { 1, 0, 0, 0, 0, 0, checkExtVectorTypeAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ExtVectorType
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_FallThrough
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_FastCall
  { 0, 0, 0, 0, 0, 1, checkFlattenAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Flatten
  { 3, 0, 0, 0, 0, 1, checkFormatAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Format
  { 1, 0, 0, 0, 0, 1, checkFormatArgAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_FormatArg
  { 0, 0, 0, 0, 0, 1, checkGNUInlineAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_GNUInline
  { 1, 0, 0, 0, 0, 0, checkGuardedByAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_GuardedBy
  { 0, 0, 0, 0, 0, 0, checkGuardedVarAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_GuardedVar
  { 0, 0, 0, 0, 0, 1, checkHotAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Hot
  { 0, 0, 0, 0, 0, 0, checkIBActionAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_IBAction
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_IBOutlet
  { 0, 1, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_IBOutletCollection
  { 1, 0, 0, 0, 0, 0, checkInitPriorityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_InitPriority
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_IntelOclBicc
  { 1, 0, 0, 0, 0, 0, checkLockReturnedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_LockReturned
  { 0, 0, 0, 0, 0, 0, checkLockableAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Lockable
  { 0, 15, 0, 0, 0, 0, checkLocksExcludedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_LocksExcluded
  { 3, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, LoopHintAttrSpellingMap },  // AT_LoopHint
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_MSABI
  { 0, 1, 0, 0, 0, 0, defaultAppertainsTo, checkMicrosoftExtLangOpts, defaultTargetRequirements, MSInheritanceAttrSpellingMap },  // AT_MSInheritance
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Malloc
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_MayAlias
  { 0, 0, 0, 0, 0, 0, checkMinSizeAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_MinSize
  { 0, 0, 0, 1, 0, 1, checkMips16AppertainsTo, defaultDiagnoseLangOpts, isTargetmipsmipsel, defaultSpellingIndexToSemanticSpelling },  // AT_Mips16
  { 1, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Mode
  { 0, 0, 0, 0, 0, 1, checkMsStructAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_MsStruct
  { 0, 0, 0, 0, 0, 0, checkNSConsumedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NSConsumed
  { 0, 0, 0, 0, 0, 0, checkNSConsumesSelfAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NSConsumesSelf
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NSReturnsAutoreleased
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NSReturnsNotRetained
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NSReturnsRetained
  { 0, 0, 0, 0, 0, 1, checkNakedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Naked
  { 1, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NeonPolyVectorType
  { 1, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NeonVectorType
  { 0, 0, 0, 0, 0, 1, checkNoCommonAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoCommon
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoDebug
  { 0, 0, 0, 0, 0, 0, checkNoDuplicateAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoDuplicate
  { 0, 0, 0, 0, 0, 1, checkNoInlineAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoInline
  { 0, 0, 0, 0, 0, 1, checkNoInstrumentFunctionAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoInstrumentFunction
  { 0, 0, 0, 1, 0, 1, checkNoMips16AppertainsTo, defaultDiagnoseLangOpts, isTargetmipsmipsel, defaultSpellingIndexToSemanticSpelling },  // AT_NoMips16
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoReturn
  { 0, 0, 0, 0, 0, 1, checkNoSanitizeAddressAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, NoSanitizeAddressAttrSpellingMap },  // AT_NoSanitizeAddress
  { 0, 0, 0, 0, 0, 0, checkNoSanitizeMemoryAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoSanitizeMemory
  { 0, 0, 0, 0, 0, 0, checkNoSanitizeThreadAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoSanitizeThread
  { 0, 0, 0, 0, 0, 1, checkNoSplitStackAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoSplitStack
  { 0, 0, 0, 0, 0, 0, checkNoThreadSafetyAnalysisAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoThreadSafetyAnalysis
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NoThrow
  { 0, 15, 0, 0, 0, 1, checkNonNullAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_NonNull
  { 1, 0, 0, 0, 0, 0, checkObjCBridgeAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCBridge
  { 1, 0, 0, 0, 0, 0, checkObjCBridgeMutableAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCBridgeMutable
  { 3, 0, 1, 0, 0, 0, checkObjCBridgeRelatedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCBridgeRelated
  { 0, 0, 0, 0, 0, 0, checkObjCDesignatedInitializerAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCDesignatedInitializer
  { 0, 0, 0, 0, 0, 0, checkObjCExceptionAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCException
  { 0, 0, 0, 0, 0, 0, checkObjCExplicitProtocolImplAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCExplicitProtocolImpl
  { 1, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCGC
  { 1, 0, 0, 0, 0, 0, checkObjCMethodFamilyAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCMethodFamily
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCNSObject
  { 1, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCOwnership
  { 0, 0, 0, 0, 0, 0, checkObjCPreciseLifetimeAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCPreciseLifetime
  { 0, 0, 0, 0, 0, 0, checkObjCRequiresPropertyDefsAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCRequiresPropertyDefs
  { 0, 0, 0, 0, 0, 0, checkObjCRequiresSuperAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCRequiresSuper
  { 0, 0, 0, 0, 0, 0, checkObjCReturnsInnerPointerAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCReturnsInnerPointer
  { 0, 0, 0, 0, 0, 0, checkObjCRootClassAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCRootClass
  { 1, 0, 0, 0, 0, 0, checkObjCRuntimeNameAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ObjCRuntimeName
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_OpenCLConstantAddressSpace
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_OpenCLGlobalAddressSpace
  { 0, 0, 0, 0, 0, 0, checkOpenCLImageAccessAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, OpenCLImageAccessAttrSpellingMap },  // AT_OpenCLImageAccess
  { 0, 0, 0, 0, 0, 0, checkOpenCLKernelAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_OpenCLKernel
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_OpenCLLocalAddressSpace
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_OpenCLPrivateAddressSpace
  { 0, 0, 0, 0, 0, 0, checkOptimizeNoneAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_OptimizeNone
  { 0, 0, 0, 0, 0, 0, checkOverloadableAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Overloadable
  { 1, 15, 0, 0, 0, 0, checkOwnershipAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, OwnershipAttrSpellingMap },  // AT_Ownership
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Packed
  { 1, 0, 0, 0, 0, 0, checkParamTypestateAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ParamTypestate
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Pascal
  { 1, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Pcs
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_PnaclCall
  { 1, 0, 0, 0, 0, 0, checkPtGuardedByAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_PtGuardedBy
  { 0, 0, 0, 0, 0, 0, checkPtGuardedVarAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_PtGuardedVar
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Ptr32
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Ptr64
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Pure
  { 1, 0, 0, 0, 1, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Regparm
  { 0, 15, 0, 0, 0, 0, checkReleaseCapabilityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, ReleaseCapabilityAttrSpellingMap },  // AT_ReleaseCapability
  { 3, 0, 0, 0, 0, 0, checkReqdWorkGroupSizeAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ReqdWorkGroupSize
  { 0, 15, 0, 0, 0, 0, checkRequiresCapabilityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, RequiresCapabilityAttrSpellingMap },  // AT_RequiresCapability
  { 1, 0, 0, 0, 0, 0, checkReturnTypestateAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ReturnTypestate
  { 0, 0, 0, 0, 0, 1, checkReturnsNonNullAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ReturnsNonNull
  { 0, 0, 0, 0, 0, 1, checkReturnsTwiceAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ReturnsTwice
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_SPtr
  { 0, 0, 0, 0, 0, 0, checkScopedLockableAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ScopedLockable
  { 1, 0, 0, 0, 0, 1, checkSectionAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, SectionAttrSpellingMap },  // AT_Section
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, checkMicrosoftExtLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_SelectAny
  { 0, 2, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Sentinel
  { 1, 0, 0, 0, 0, 0, checkSetTypestateAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_SetTypestate
  { 1, 15, 0, 0, 0, 0, checkSharedTrylockFunctionAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_SharedTrylockFunction
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_StdCall
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_SysVABI
  { 1, 0, 0, 0, 0, 1, checkTLSModelAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_TLSModel
  { 1, 0, 0, 0, 0, 0, checkTestTypestateAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_TestTypestate
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_ThisCall
  { 0, 0, 0, 0, 0, 0, checkThreadAppertainsTo, checkMicrosoftExtLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Thread
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_TransparentUnion
  { 1, 15, 0, 0, 0, 0, checkTryAcquireCapabilityAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, TryAcquireCapabilityAttrSpellingMap },  // AT_TryAcquireCapability
  { 4, 0, 1, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_TypeTagForDatatype
  { 1, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_TypeVisibility
  { 0, 0, 0, 0, 1, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_UPtr
  { 0, 1, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Unavailable
  { 0, 0, 0, 0, 0, 1, checkUnusedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Unused
  { 0, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Used
  { 1, 0, 0, 0, 0, 0, defaultAppertainsTo, checkMicrosoftExtBorlandLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Uuid
  { 0, 0, 0, 0, 0, 0, checkVecReturnAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_VecReturn
  { 1, 0, 0, 0, 0, 0, checkVecTypeHintAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_VecTypeHint
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_VectorCall
  { 1, 0, 0, 0, 1, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_VectorSize
  { 1, 0, 0, 0, 0, 1, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Visibility
  { 0, 0, 0, 0, 0, 0, checkWarnUnusedAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_WarnUnused
  { 0, 0, 0, 0, 0, 1, checkWarnUnusedResultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_WarnUnusedResult
  { 0, 0, 0, 0, 0, 1, checkWeakAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_Weak
  { 0, 0, 0, 0, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_WeakImport
  { 0, 1, 0, 0, 0, 1, checkWeakRefAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_WeakRef
  { 3, 0, 0, 0, 0, 0, checkWorkGroupSizeHintAppertainsTo, defaultDiagnoseLangOpts, defaultTargetRequirements, defaultSpellingIndexToSemanticSpelling },  // AT_WorkGroupSizeHint
  { 0, 0, 0, 1, 0, 0, defaultAppertainsTo, defaultDiagnoseLangOpts, isTargetx86, defaultSpellingIndexToSemanticSpelling }  // AT_X86ForceAlignArgPointer
};

