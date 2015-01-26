/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Attribute classes' member function definitions                              *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

ARMInterruptAttr *ARMInterruptAttr::clone(ASTContext &C) const {
  auto *A = new (C) ARMInterruptAttr(getLocation(), C, interrupt, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ARMInterruptAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((interrupt(\"" << ARMInterruptAttr::ConvertInterruptTypeToStr(getInterrupt()) << "\")))";
    break;
  }
}
}

const char *ARMInterruptAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "interrupt";
  }
}

AcquireCapabilityAttr *AcquireCapabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) AcquireCapabilityAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AcquireCapabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((acquire_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 1 : {
    OS << " [[clang::acquire_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((acquire_shared_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 3 : {
    OS << " [[clang::acquire_shared_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 4 : {
    OS << " __attribute__((exclusive_lock_function(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 5 : {
    OS << " __attribute__((shared_lock_function(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *AcquireCapabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "acquire_capability";
  case 1:
    return "acquire_capability";
  case 2:
    return "acquire_shared_capability";
  case 3:
    return "acquire_shared_capability";
  case 4:
    return "exclusive_lock_function";
  case 5:
    return "shared_lock_function";
  }
}

AcquiredAfterAttr *AcquiredAfterAttr::clone(ASTContext &C) const {
  auto *A = new (C) AcquiredAfterAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AcquiredAfterAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((acquired_after(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *AcquiredAfterAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "acquired_after";
  }
}

AcquiredBeforeAttr *AcquiredBeforeAttr::clone(ASTContext &C) const {
  auto *A = new (C) AcquiredBeforeAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AcquiredBeforeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((acquired_before(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *AcquiredBeforeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "acquired_before";
  }
}

AliasAttr *AliasAttr::clone(ASTContext &C) const {
  auto *A = new (C) AliasAttr(getLocation(), C, getAliasee(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AliasAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((alias(\"" << getAliasee() << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::alias(\"" << getAliasee() << "\")]]";
    break;
  }
}
}

const char *AliasAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "alias";
  case 1:
    return "alias";
  }
}

AlignMac68kAttr *AlignMac68kAttr::clone(ASTContext &C) const {
  auto *A = new (C) AlignMac68kAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AlignMac68kAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
}

const char *AlignMac68kAttr::getSpelling() const {
  return "(No spelling)";
}

AlignValueAttr *AlignValueAttr::clone(ASTContext &C) const {
  auto *A = new (C) AlignValueAttr(getLocation(), C, alignment, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AlignValueAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((align_value(" << getAlignment() << ")))";
    break;
  }
}
}

const char *AlignValueAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "align_value";
  }
}

bool AlignedAttr::isAlignmentDependent() const {
  if (isalignmentExpr)
    return alignmentExpr && (alignmentExpr->isValueDependent() || alignmentExpr->isTypeDependent());
  else
    return alignmentType->getType()->isDependentType();
}
unsigned AlignedAttr::getAlignment(ASTContext &Ctx) const {
  assert(!isAlignmentDependent());
  if (isalignmentExpr)
    return (alignmentExpr ? alignmentExpr->EvaluateKnownConstInt(Ctx).getZExtValue() : 16)* Ctx.getCharWidth();
  else
    return 0; // FIXME
}
AlignedAttr *AlignedAttr::clone(ASTContext &C) const {
  auto *A = new (C) AlignedAttr(getLocation(), C, isalignmentExpr, isalignmentExpr ? static_cast<void*>(alignmentExpr) : alignmentType, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AlignedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((aligned(";
    if (isalignmentExpr && alignmentExpr)
      alignmentExpr->printPretty(OS, 0, Policy);
    OS << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::aligned(";
    if (isalignmentExpr && alignmentExpr)
      alignmentExpr->printPretty(OS, 0, Policy);
    OS << ")]]";
    break;
  }
  case 2 : {
    OS << " __declspec(align(";
    if (isalignmentExpr && alignmentExpr)
      alignmentExpr->printPretty(OS, 0, Policy);
    OS << "))";
    break;
  }
  case 3 : {
    OS << " alignas(";
    if (isalignmentExpr && alignmentExpr)
      alignmentExpr->printPretty(OS, 0, Policy);
    OS << ")";
    break;
  }
  case 4 : {
    OS << " _Alignas(";
    if (isalignmentExpr && alignmentExpr)
      alignmentExpr->printPretty(OS, 0, Policy);
    OS << ")";
    break;
  }
}
}

const char *AlignedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "aligned";
  case 1:
    return "aligned";
  case 2:
    return "align";
  case 3:
    return "alignas";
  case 4:
    return "_Alignas";
  }
}

AlwaysInlineAttr *AlwaysInlineAttr::clone(ASTContext &C) const {
  auto *A = new (C) AlwaysInlineAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AlwaysInlineAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((always_inline))";
    break;
  }
  case 1 : {
    OS << " [[gnu::always_inline]]";
    break;
  }
  case 2 : {
    OS << " __forceinline";
    break;
  }
}
}

const char *AlwaysInlineAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "always_inline";
  case 1:
    return "always_inline";
  case 2:
    return "__forceinline";
  }
}

AnalyzerNoReturnAttr *AnalyzerNoReturnAttr::clone(ASTContext &C) const {
  auto *A = new (C) AnalyzerNoReturnAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AnalyzerNoReturnAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((analyzer_noreturn))";
    break;
  }
}
}

const char *AnalyzerNoReturnAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "analyzer_noreturn";
  }
}

AnnotateAttr *AnnotateAttr::clone(ASTContext &C) const {
  auto *A = new (C) AnnotateAttr(getLocation(), C, getAnnotation(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AnnotateAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((annotate(\"" << getAnnotation() << "\")))";
    break;
  }
}
}

const char *AnnotateAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "annotate";
  }
}

ArcWeakrefUnavailableAttr *ArcWeakrefUnavailableAttr::clone(ASTContext &C) const {
  auto *A = new (C) ArcWeakrefUnavailableAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ArcWeakrefUnavailableAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_arc_weak_reference_unavailable))";
    break;
  }
}
}

const char *ArcWeakrefUnavailableAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_arc_weak_reference_unavailable";
  }
}

ArgumentWithTypeTagAttr *ArgumentWithTypeTagAttr::clone(ASTContext &C) const {
  auto *A = new (C) ArgumentWithTypeTagAttr(getLocation(), C, argumentKind, argumentIdx, typeTagIdx, isPointer, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ArgumentWithTypeTagAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((argument_with_type_tag(" << getArgumentKind()->getName() << ", " << getArgumentIdx() << ", " << getTypeTagIdx() << ", " << getIsPointer() << ")))";
    break;
  }
  case 1 : {
    OS << " __attribute__((pointer_with_type_tag(" << getArgumentKind()->getName() << ", " << getArgumentIdx() << ", " << getTypeTagIdx() << ", " << getIsPointer() << ")))";
    break;
  }
}
}

const char *ArgumentWithTypeTagAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "argument_with_type_tag";
  case 1:
    return "pointer_with_type_tag";
  }
}

AsmLabelAttr *AsmLabelAttr::clone(ASTContext &C) const {
  auto *A = new (C) AsmLabelAttr(getLocation(), C, getLabel(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AsmLabelAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " asm(\"" << getLabel() << "\")";
    break;
  }
  case 1 : {
    OS << " __asm__(\"" << getLabel() << "\")";
    break;
  }
}
}

const char *AsmLabelAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "asm";
  case 1:
    return "__asm__";
  }
}

AssertCapabilityAttr *AssertCapabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) AssertCapabilityAttr(getLocation(), C, expr, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AssertCapabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((assert_capability(" << getExpr() << ")))";
    break;
  }
  case 1 : {
    OS << " [[clang::assert_capability(" << getExpr() << ")]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((assert_shared_capability(" << getExpr() << ")))";
    break;
  }
  case 3 : {
    OS << " [[clang::assert_shared_capability(" << getExpr() << ")]]";
    break;
  }
}
}

const char *AssertCapabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "assert_capability";
  case 1:
    return "assert_capability";
  case 2:
    return "assert_shared_capability";
  case 3:
    return "assert_shared_capability";
  }
}

AssertExclusiveLockAttr *AssertExclusiveLockAttr::clone(ASTContext &C) const {
  auto *A = new (C) AssertExclusiveLockAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AssertExclusiveLockAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((assert_exclusive_lock(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *AssertExclusiveLockAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "assert_exclusive_lock";
  }
}

AssertSharedLockAttr *AssertSharedLockAttr::clone(ASTContext &C) const {
  auto *A = new (C) AssertSharedLockAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AssertSharedLockAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((assert_shared_lock(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *AssertSharedLockAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "assert_shared_lock";
  }
}

AssumeAlignedAttr *AssumeAlignedAttr::clone(ASTContext &C) const {
  auto *A = new (C) AssumeAlignedAttr(getLocation(), C, alignment, offset, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AssumeAlignedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((assume_aligned(" << getAlignment() << ", " << getOffset() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::assume_aligned(" << getAlignment() << ", " << getOffset() << ")]]";
    break;
  }
}
}

const char *AssumeAlignedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "assume_aligned";
  case 1:
    return "assume_aligned";
  }
}

AvailabilityAttr *AvailabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) AvailabilityAttr(getLocation(), C, platform, getIntroduced(), getDeprecated(), getObsoleted(), unavailable, getMessage(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void AvailabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((availability(" << getPlatform()->getName();
  if (!getIntroduced().empty()) OS << ", introduced=" << getIntroduced();
  if (!getDeprecated().empty()) OS << ", deprecated=" << getDeprecated();
  if (!getObsoleted().empty()) OS << ", obsoleted=" << getObsoleted();
  if (getUnavailable()) OS << ", unavailable";
  OS << ")))";
    break;
  }
}
}

const char *AvailabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "availability";
  }
}

BlocksAttr *BlocksAttr::clone(ASTContext &C) const {
  auto *A = new (C) BlocksAttr(getLocation(), C, type, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void BlocksAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((blocks(\"" << BlocksAttr::ConvertBlockTypeToStr(getType()) << "\")))";
    break;
  }
}
}

const char *BlocksAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "blocks";
  }
}

C11NoReturnAttr *C11NoReturnAttr::clone(ASTContext &C) const {
  auto *A = new (C) C11NoReturnAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void C11NoReturnAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " _Noreturn";
    break;
  }
}
}

const char *C11NoReturnAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "_Noreturn";
  }
}

CDeclAttr *CDeclAttr::clone(ASTContext &C) const {
  auto *A = new (C) CDeclAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CDeclAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cdecl))";
    break;
  }
  case 1 : {
    OS << " [[gnu::cdecl]]";
    break;
  }
  case 2 : {
    OS << " __cdecl";
    break;
  }
  case 3 : {
    OS << " _cdecl";
    break;
  }
}
}

const char *CDeclAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cdecl";
  case 1:
    return "cdecl";
  case 2:
    return "__cdecl";
  case 3:
    return "_cdecl";
  }
}

CFAuditedTransferAttr *CFAuditedTransferAttr::clone(ASTContext &C) const {
  auto *A = new (C) CFAuditedTransferAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CFAuditedTransferAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cf_audited_transfer))";
    break;
  }
}
}

const char *CFAuditedTransferAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cf_audited_transfer";
  }
}

CFConsumedAttr *CFConsumedAttr::clone(ASTContext &C) const {
  auto *A = new (C) CFConsumedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CFConsumedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cf_consumed))";
    break;
  }
}
}

const char *CFConsumedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cf_consumed";
  }
}

CFReturnsNotRetainedAttr *CFReturnsNotRetainedAttr::clone(ASTContext &C) const {
  auto *A = new (C) CFReturnsNotRetainedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CFReturnsNotRetainedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cf_returns_not_retained))";
    break;
  }
}
}

const char *CFReturnsNotRetainedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cf_returns_not_retained";
  }
}

CFReturnsRetainedAttr *CFReturnsRetainedAttr::clone(ASTContext &C) const {
  auto *A = new (C) CFReturnsRetainedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CFReturnsRetainedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cf_returns_retained))";
    break;
  }
}
}

const char *CFReturnsRetainedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cf_returns_retained";
  }
}

CFUnknownTransferAttr *CFUnknownTransferAttr::clone(ASTContext &C) const {
  auto *A = new (C) CFUnknownTransferAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CFUnknownTransferAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cf_unknown_transfer))";
    break;
  }
}
}

const char *CFUnknownTransferAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cf_unknown_transfer";
  }
}

CUDAConstantAttr *CUDAConstantAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDAConstantAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDAConstantAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((constant))";
    break;
  }
}
}

const char *CUDAConstantAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "constant";
  }
}

CUDADeviceAttr *CUDADeviceAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDADeviceAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDADeviceAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((device))";
    break;
  }
}
}

const char *CUDADeviceAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "device";
  }
}

CUDAGlobalAttr *CUDAGlobalAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDAGlobalAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDAGlobalAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((global))";
    break;
  }
}
}

const char *CUDAGlobalAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "global";
  }
}

CUDAHostAttr *CUDAHostAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDAHostAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDAHostAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((host))";
    break;
  }
}
}

const char *CUDAHostAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "host";
  }
}

CUDAInvalidTargetAttr *CUDAInvalidTargetAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDAInvalidTargetAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDAInvalidTargetAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
}

const char *CUDAInvalidTargetAttr::getSpelling() const {
  return "(No spelling)";
}

CUDALaunchBoundsAttr *CUDALaunchBoundsAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDALaunchBoundsAttr(getLocation(), C, maxThreads, minBlocks, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDALaunchBoundsAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((launch_bounds(" << getMaxThreads() << ", " << getMinBlocks() << ")))";
    break;
  }
}
}

const char *CUDALaunchBoundsAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "launch_bounds";
  }
}

CUDASharedAttr *CUDASharedAttr::clone(ASTContext &C) const {
  auto *A = new (C) CUDASharedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CUDASharedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((shared))";
    break;
  }
}
}

const char *CUDASharedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "shared";
  }
}

CXX11NoReturnAttr *CXX11NoReturnAttr::clone(ASTContext &C) const {
  auto *A = new (C) CXX11NoReturnAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CXX11NoReturnAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " [[noreturn]]";
    break;
  }
}
}

const char *CXX11NoReturnAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "noreturn";
  }
}

CallableWhenAttr *CallableWhenAttr::clone(ASTContext &C) const {
  auto *A = new (C) CallableWhenAttr(getLocation(), C, callableStates_, callableStates_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CallableWhenAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((callable_when(";
  bool isFirst = true;
  for (const auto &Val : callableStates()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << "\"" << CallableWhenAttr::ConvertConsumedStateToStr(Val)<< "\"";
  }
  OS << ")))";
    break;
  }
}
}

const char *CallableWhenAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "callable_when";
  }
}

CapabilityAttr *CapabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) CapabilityAttr(getLocation(), C, getName(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CapabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((capability(\"" << getName() << "\")))";
    break;
  }
  case 1 : {
    OS << " [[clang::capability(\"" << getName() << "\")]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((shared_capability(\"" << getName() << "\")))";
    break;
  }
  case 3 : {
    OS << " [[clang::shared_capability(\"" << getName() << "\")]]";
    break;
  }
}
}

const char *CapabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "capability";
  case 1:
    return "capability";
  case 2:
    return "shared_capability";
  case 3:
    return "shared_capability";
  }
}

CapturedRecordAttr *CapturedRecordAttr::clone(ASTContext &C) const {
  auto *A = new (C) CapturedRecordAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CapturedRecordAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
}

const char *CapturedRecordAttr::getSpelling() const {
  return "(No spelling)";
}

CarriesDependencyAttr *CarriesDependencyAttr::clone(ASTContext &C) const {
  auto *A = new (C) CarriesDependencyAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CarriesDependencyAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((carries_dependency))";
    break;
  }
  case 1 : {
    OS << " [[carries_dependency]]";
    break;
  }
}
}

const char *CarriesDependencyAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "carries_dependency";
  case 1:
    return "carries_dependency";
  }
}

CleanupAttr *CleanupAttr::clone(ASTContext &C) const {
  auto *A = new (C) CleanupAttr(getLocation(), C, functionDecl, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CleanupAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cleanup(" << getFunctionDecl()->getNameInfo().getAsString() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::cleanup(" << getFunctionDecl()->getNameInfo().getAsString() << ")]]";
    break;
  }
}
}

const char *CleanupAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cleanup";
  case 1:
    return "cleanup";
  }
}

ColdAttr *ColdAttr::clone(ASTContext &C) const {
  auto *A = new (C) ColdAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ColdAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((cold))";
    break;
  }
  case 1 : {
    OS << " [[gnu::cold]]";
    break;
  }
}
}

const char *ColdAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "cold";
  case 1:
    return "cold";
  }
}

CommonAttr *CommonAttr::clone(ASTContext &C) const {
  auto *A = new (C) CommonAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void CommonAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((common))";
    break;
  }
  case 1 : {
    OS << " [[gnu::common]]";
    break;
  }
}
}

const char *CommonAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "common";
  case 1:
    return "common";
  }
}

ConstAttr *ConstAttr::clone(ASTContext &C) const {
  auto *A = new (C) ConstAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ConstAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((const))";
    break;
  }
  case 1 : {
    OS << " [[gnu::const]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((__const))";
    break;
  }
  case 3 : {
    OS << " [[gnu::__const]]";
    break;
  }
}
}

const char *ConstAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "const";
  case 1:
    return "const";
  case 2:
    return "__const";
  case 3:
    return "__const";
  }
}

ConstructorAttr *ConstructorAttr::clone(ASTContext &C) const {
  auto *A = new (C) ConstructorAttr(getLocation(), C, priority, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ConstructorAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((constructor(" << getPriority() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::constructor(" << getPriority() << ")]]";
    break;
  }
}
}

const char *ConstructorAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "constructor";
  case 1:
    return "constructor";
  }
}

ConsumableAttr *ConsumableAttr::clone(ASTContext &C) const {
  auto *A = new (C) ConsumableAttr(getLocation(), C, defaultState, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ConsumableAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((consumable(\"" << ConsumableAttr::ConvertConsumedStateToStr(getDefaultState()) << "\")))";
    break;
  }
}
}

const char *ConsumableAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "consumable";
  }
}

ConsumableAutoCastAttr *ConsumableAutoCastAttr::clone(ASTContext &C) const {
  auto *A = new (C) ConsumableAutoCastAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ConsumableAutoCastAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((consumable_auto_cast_state))";
    break;
  }
}
}

const char *ConsumableAutoCastAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "consumable_auto_cast_state";
  }
}

ConsumableSetOnReadAttr *ConsumableSetOnReadAttr::clone(ASTContext &C) const {
  auto *A = new (C) ConsumableSetOnReadAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ConsumableSetOnReadAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((consumable_set_state_on_read))";
    break;
  }
}
}

const char *ConsumableSetOnReadAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "consumable_set_state_on_read";
  }
}

DLLExportAttr *DLLExportAttr::clone(ASTContext &C) const {
  auto *A = new (C) DLLExportAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void DLLExportAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __declspec(dllexport)";
    break;
  }
  case 1 : {
    OS << " __attribute__((dllexport))";
    break;
  }
  case 2 : {
    OS << " [[gnu::dllexport]]";
    break;
  }
}
}

const char *DLLExportAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "dllexport";
  case 1:
    return "dllexport";
  case 2:
    return "dllexport";
  }
}

DLLImportAttr *DLLImportAttr::clone(ASTContext &C) const {
  auto *A = new (C) DLLImportAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void DLLImportAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __declspec(dllimport)";
    break;
  }
  case 1 : {
    OS << " __attribute__((dllimport))";
    break;
  }
  case 2 : {
    OS << " [[gnu::dllimport]]";
    break;
  }
}
}

const char *DLLImportAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "dllimport";
  case 1:
    return "dllimport";
  case 2:
    return "dllimport";
  }
}

DeprecatedAttr *DeprecatedAttr::clone(ASTContext &C) const {
  auto *A = new (C) DeprecatedAttr(getLocation(), C, getMessage(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void DeprecatedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((deprecated(\"" << getMessage() << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::deprecated(\"" << getMessage() << "\")]]";
    break;
  }
  case 2 : {
    OS << " __declspec(deprecated(\"" << getMessage() << "\"))";
    break;
  }
  case 3 : {
    OS << " [[deprecated(\"" << getMessage() << "\")]]";
    break;
  }
}
}

const char *DeprecatedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "deprecated";
  case 1:
    return "deprecated";
  case 2:
    return "deprecated";
  case 3:
    return "deprecated";
  }
}

DestructorAttr *DestructorAttr::clone(ASTContext &C) const {
  auto *A = new (C) DestructorAttr(getLocation(), C, priority, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void DestructorAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((destructor(" << getPriority() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::destructor(" << getPriority() << ")]]";
    break;
  }
}
}

const char *DestructorAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "destructor";
  case 1:
    return "destructor";
  }
}

EnableIfAttr *EnableIfAttr::clone(ASTContext &C) const {
  auto *A = new (C) EnableIfAttr(getLocation(), C, cond, getMessage(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void EnableIfAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((enable_if(" << getCond() << ", \"" << getMessage() << "\")))";
    break;
  }
}
}

const char *EnableIfAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "enable_if";
  }
}

ExclusiveTrylockFunctionAttr *ExclusiveTrylockFunctionAttr::clone(ASTContext &C) const {
  auto *A = new (C) ExclusiveTrylockFunctionAttr(getLocation(), C, successValue, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ExclusiveTrylockFunctionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((exclusive_trylock_function(" << getSuccessValue() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *ExclusiveTrylockFunctionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "exclusive_trylock_function";
  }
}

FallThroughAttr *FallThroughAttr::clone(ASTContext &C) const {
  auto *A = new (C) FallThroughAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void FallThroughAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " [[clang::fallthrough]]";
    break;
  }
}
}

const char *FallThroughAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "fallthrough";
  }
}

FastCallAttr *FastCallAttr::clone(ASTContext &C) const {
  auto *A = new (C) FastCallAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void FastCallAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((fastcall))";
    break;
  }
  case 1 : {
    OS << " [[gnu::fastcall]]";
    break;
  }
  case 2 : {
    OS << " __fastcall";
    break;
  }
  case 3 : {
    OS << " _fastcall";
    break;
  }
}
}

const char *FastCallAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "fastcall";
  case 1:
    return "fastcall";
  case 2:
    return "__fastcall";
  case 3:
    return "_fastcall";
  }
}

FinalAttr *FinalAttr::clone(ASTContext &C) const {
  auto *A = new (C) FinalAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void FinalAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " final";
    break;
  }
  case 1 : {
    OS << " sealed";
    break;
  }
}
}

const char *FinalAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "final";
  case 1:
    return "sealed";
  }
}

FlattenAttr *FlattenAttr::clone(ASTContext &C) const {
  auto *A = new (C) FlattenAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void FlattenAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((flatten))";
    break;
  }
  case 1 : {
    OS << " [[gnu::flatten]]";
    break;
  }
}
}

const char *FlattenAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "flatten";
  case 1:
    return "flatten";
  }
}

FormatAttr *FormatAttr::clone(ASTContext &C) const {
  auto *A = new (C) FormatAttr(getLocation(), C, type, formatIdx, firstArg, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void FormatAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((format(" << getType()->getName() << ", " << getFormatIdx() << ", " << getFirstArg() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::format(" << getType()->getName() << ", " << getFormatIdx() << ", " << getFirstArg() << ")]]";
    break;
  }
}
}

const char *FormatAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "format";
  case 1:
    return "format";
  }
}

FormatArgAttr *FormatArgAttr::clone(ASTContext &C) const {
  auto *A = new (C) FormatArgAttr(getLocation(), C, formatIdx, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void FormatArgAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((format_arg(" << getFormatIdx() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::format_arg(" << getFormatIdx() << ")]]";
    break;
  }
}
}

const char *FormatArgAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "format_arg";
  case 1:
    return "format_arg";
  }
}

GNUInlineAttr *GNUInlineAttr::clone(ASTContext &C) const {
  auto *A = new (C) GNUInlineAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void GNUInlineAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((gnu_inline))";
    break;
  }
  case 1 : {
    OS << " [[gnu::gnu_inline]]";
    break;
  }
}
}

const char *GNUInlineAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "gnu_inline";
  case 1:
    return "gnu_inline";
  }
}

GuardedByAttr *GuardedByAttr::clone(ASTContext &C) const {
  auto *A = new (C) GuardedByAttr(getLocation(), C, arg, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void GuardedByAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((guarded_by(" << getArg() << ")))";
    break;
  }
}
}

const char *GuardedByAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "guarded_by";
  }
}

GuardedVarAttr *GuardedVarAttr::clone(ASTContext &C) const {
  auto *A = new (C) GuardedVarAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void GuardedVarAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((guarded_var))";
    break;
  }
}
}

const char *GuardedVarAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "guarded_var";
  }
}

HotAttr *HotAttr::clone(ASTContext &C) const {
  auto *A = new (C) HotAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void HotAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((hot))";
    break;
  }
  case 1 : {
    OS << " [[gnu::hot]]";
    break;
  }
}
}

const char *HotAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "hot";
  case 1:
    return "hot";
  }
}

IBActionAttr *IBActionAttr::clone(ASTContext &C) const {
  auto *A = new (C) IBActionAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void IBActionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ibaction))";
    break;
  }
}
}

const char *IBActionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ibaction";
  }
}

IBOutletAttr *IBOutletAttr::clone(ASTContext &C) const {
  auto *A = new (C) IBOutletAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void IBOutletAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((iboutlet))";
    break;
  }
}
}

const char *IBOutletAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "iboutlet";
  }
}

IBOutletCollectionAttr *IBOutletCollectionAttr::clone(ASTContext &C) const {
  auto *A = new (C) IBOutletCollectionAttr(getLocation(), C, interface, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void IBOutletCollectionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((iboutletcollection(" << getInterface().getAsString() << ")))";
    break;
  }
}
}

const char *IBOutletCollectionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "iboutletcollection";
  }
}

InitPriorityAttr *InitPriorityAttr::clone(ASTContext &C) const {
  auto *A = new (C) InitPriorityAttr(getLocation(), C, priority, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void InitPriorityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((init_priority(" << getPriority() << ")))";
    break;
  }
}
}

const char *InitPriorityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "init_priority";
  }
}

InitSegAttr *InitSegAttr::clone(ASTContext &C) const {
  auto *A = new (C) InitSegAttr(getLocation(), C, getSection(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void InitSegAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << "#pragma init_seg ";
    printPrettyPragma(OS, Policy);
    break;
  }
}
}

const char *InitSegAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "init_seg";
  }
}

IntelOclBiccAttr *IntelOclBiccAttr::clone(ASTContext &C) const {
  auto *A = new (C) IntelOclBiccAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void IntelOclBiccAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((intel_ocl_bicc))";
    break;
  }
}
}

const char *IntelOclBiccAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "intel_ocl_bicc";
  }
}

LockReturnedAttr *LockReturnedAttr::clone(ASTContext &C) const {
  auto *A = new (C) LockReturnedAttr(getLocation(), C, arg, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void LockReturnedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((lock_returned(" << getArg() << ")))";
    break;
  }
}
}

const char *LockReturnedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "lock_returned";
  }
}

LocksExcludedAttr *LocksExcludedAttr::clone(ASTContext &C) const {
  auto *A = new (C) LocksExcludedAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void LocksExcludedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((locks_excluded(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *LocksExcludedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "locks_excluded";
  }
}

LoopHintAttr *LoopHintAttr::clone(ASTContext &C) const {
  auto *A = new (C) LoopHintAttr(getLocation(), C, option, state, value, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void LoopHintAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << "#pragma clang loop ";
    printPrettyPragma(OS, Policy);
    break;
  }
  case 1 : {
    OS << "#pragma unroll ";
    printPrettyPragma(OS, Policy);
    break;
  }
  case 2 : {
    OS << "#pragma nounroll ";
    printPrettyPragma(OS, Policy);
    break;
  }
}
}

const char *LoopHintAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "loop";
  case 1:
    return "unroll";
  case 2:
    return "nounroll";
  }
}

MSABIAttr *MSABIAttr::clone(ASTContext &C) const {
  auto *A = new (C) MSABIAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MSABIAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ms_abi))";
    break;
  }
  case 1 : {
    OS << " [[gnu::ms_abi]]";
    break;
  }
}
}

const char *MSABIAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ms_abi";
  case 1:
    return "ms_abi";
  }
}

MSInheritanceAttr *MSInheritanceAttr::clone(ASTContext &C) const {
  auto *A = new (C) MSInheritanceAttr(getLocation(), C, bestCase, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MSInheritanceAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __single_inheritance(" << getBestCase() << ")";
    break;
  }
  case 1 : {
    OS << " __multiple_inheritance(" << getBestCase() << ")";
    break;
  }
  case 2 : {
    OS << " __virtual_inheritance(" << getBestCase() << ")";
    break;
  }
  case 3 : {
    OS << " __unspecified_inheritance(" << getBestCase() << ")";
    break;
  }
}
}

const char *MSInheritanceAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "__single_inheritance";
  case 1:
    return "__multiple_inheritance";
  case 2:
    return "__virtual_inheritance";
  case 3:
    return "__unspecified_inheritance";
  }
}

MSP430InterruptAttr *MSP430InterruptAttr::clone(ASTContext &C) const {
  auto *A = new (C) MSP430InterruptAttr(getLocation(), C, number, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MSP430InterruptAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((interrupt(" << getNumber() << ")))";
    break;
  }
}
}

const char *MSP430InterruptAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "interrupt";
  }
}

MSVtorDispAttr *MSVtorDispAttr::clone(ASTContext &C) const {
  auto *A = new (C) MSVtorDispAttr(getLocation(), C, vdm, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MSVtorDispAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
}

const char *MSVtorDispAttr::getSpelling() const {
  return "(No spelling)";
}

MallocAttr *MallocAttr::clone(ASTContext &C) const {
  auto *A = new (C) MallocAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MallocAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((malloc))";
    break;
  }
  case 1 : {
    OS << " [[gnu::malloc]]";
    break;
  }
}
}

const char *MallocAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "malloc";
  case 1:
    return "malloc";
  }
}

MaxFieldAlignmentAttr *MaxFieldAlignmentAttr::clone(ASTContext &C) const {
  auto *A = new (C) MaxFieldAlignmentAttr(getLocation(), C, alignment, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MaxFieldAlignmentAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
}

const char *MaxFieldAlignmentAttr::getSpelling() const {
  return "(No spelling)";
}

MayAliasAttr *MayAliasAttr::clone(ASTContext &C) const {
  auto *A = new (C) MayAliasAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MayAliasAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((may_alias))";
    break;
  }
  case 1 : {
    OS << " [[gnu::may_alias]]";
    break;
  }
}
}

const char *MayAliasAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "may_alias";
  case 1:
    return "may_alias";
  }
}

MinSizeAttr *MinSizeAttr::clone(ASTContext &C) const {
  auto *A = new (C) MinSizeAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MinSizeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((minsize))";
    break;
  }
}
}

const char *MinSizeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "minsize";
  }
}

Mips16Attr *Mips16Attr::clone(ASTContext &C) const {
  auto *A = new (C) Mips16Attr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void Mips16Attr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((mips16))";
    break;
  }
  case 1 : {
    OS << " [[gnu::mips16]]";
    break;
  }
}
}

const char *Mips16Attr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "mips16";
  case 1:
    return "mips16";
  }
}

ModeAttr *ModeAttr::clone(ASTContext &C) const {
  auto *A = new (C) ModeAttr(getLocation(), C, mode, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ModeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((mode(" << getMode()->getName() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::mode(" << getMode()->getName() << ")]]";
    break;
  }
}
}

const char *ModeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "mode";
  case 1:
    return "mode";
  }
}

MsStructAttr *MsStructAttr::clone(ASTContext &C) const {
  auto *A = new (C) MsStructAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void MsStructAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ms_struct))";
    break;
  }
  case 1 : {
    OS << " [[gnu::ms_struct]]";
    break;
  }
}
}

const char *MsStructAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ms_struct";
  case 1:
    return "ms_struct";
  }
}

NSConsumedAttr *NSConsumedAttr::clone(ASTContext &C) const {
  auto *A = new (C) NSConsumedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NSConsumedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ns_consumed))";
    break;
  }
}
}

const char *NSConsumedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ns_consumed";
  }
}

NSConsumesSelfAttr *NSConsumesSelfAttr::clone(ASTContext &C) const {
  auto *A = new (C) NSConsumesSelfAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NSConsumesSelfAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ns_consumes_self))";
    break;
  }
}
}

const char *NSConsumesSelfAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ns_consumes_self";
  }
}

NSReturnsAutoreleasedAttr *NSReturnsAutoreleasedAttr::clone(ASTContext &C) const {
  auto *A = new (C) NSReturnsAutoreleasedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NSReturnsAutoreleasedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ns_returns_autoreleased))";
    break;
  }
}
}

const char *NSReturnsAutoreleasedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ns_returns_autoreleased";
  }
}

NSReturnsNotRetainedAttr *NSReturnsNotRetainedAttr::clone(ASTContext &C) const {
  auto *A = new (C) NSReturnsNotRetainedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NSReturnsNotRetainedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ns_returns_not_retained))";
    break;
  }
}
}

const char *NSReturnsNotRetainedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ns_returns_not_retained";
  }
}

NSReturnsRetainedAttr *NSReturnsRetainedAttr::clone(ASTContext &C) const {
  auto *A = new (C) NSReturnsRetainedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NSReturnsRetainedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ns_returns_retained))";
    break;
  }
}
}

const char *NSReturnsRetainedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ns_returns_retained";
  }
}

NakedAttr *NakedAttr::clone(ASTContext &C) const {
  auto *A = new (C) NakedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NakedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((naked))";
    break;
  }
  case 1 : {
    OS << " [[gnu::naked]]";
    break;
  }
  case 2 : {
    OS << " __declspec(naked)";
    break;
  }
}
}

const char *NakedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "naked";
  case 1:
    return "naked";
  case 2:
    return "naked";
  }
}

NoCommonAttr *NoCommonAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoCommonAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoCommonAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((nocommon))";
    break;
  }
  case 1 : {
    OS << " [[gnu::nocommon]]";
    break;
  }
}
}

const char *NoCommonAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "nocommon";
  case 1:
    return "nocommon";
  }
}

NoDebugAttr *NoDebugAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoDebugAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoDebugAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((nodebug))";
    break;
  }
  case 1 : {
    OS << " [[gnu::nodebug]]";
    break;
  }
}
}

const char *NoDebugAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "nodebug";
  case 1:
    return "nodebug";
  }
}

NoDuplicateAttr *NoDuplicateAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoDuplicateAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoDuplicateAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((noduplicate))";
    break;
  }
  case 1 : {
    OS << " [[clang::noduplicate]]";
    break;
  }
}
}

const char *NoDuplicateAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "noduplicate";
  case 1:
    return "noduplicate";
  }
}

NoInlineAttr *NoInlineAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoInlineAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoInlineAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((noinline))";
    break;
  }
  case 1 : {
    OS << " [[gnu::noinline]]";
    break;
  }
  case 2 : {
    OS << " __declspec(noinline)";
    break;
  }
}
}

const char *NoInlineAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "noinline";
  case 1:
    return "noinline";
  case 2:
    return "noinline";
  }
}

NoInstrumentFunctionAttr *NoInstrumentFunctionAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoInstrumentFunctionAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoInstrumentFunctionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((no_instrument_function))";
    break;
  }
  case 1 : {
    OS << " [[gnu::no_instrument_function]]";
    break;
  }
}
}

const char *NoInstrumentFunctionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "no_instrument_function";
  case 1:
    return "no_instrument_function";
  }
}

NoMips16Attr *NoMips16Attr::clone(ASTContext &C) const {
  auto *A = new (C) NoMips16Attr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoMips16Attr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((nomips16))";
    break;
  }
  case 1 : {
    OS << " [[gnu::nomips16]]";
    break;
  }
}
}

const char *NoMips16Attr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "nomips16";
  case 1:
    return "nomips16";
  }
}

NoReturnAttr *NoReturnAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoReturnAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoReturnAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((noreturn))";
    break;
  }
  case 1 : {
    OS << " [[gnu::noreturn]]";
    break;
  }
  case 2 : {
    OS << " __declspec(noreturn)";
    break;
  }
}
}

const char *NoReturnAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "noreturn";
  case 1:
    return "noreturn";
  case 2:
    return "noreturn";
  }
}

NoSanitizeAddressAttr *NoSanitizeAddressAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoSanitizeAddressAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoSanitizeAddressAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((no_address_safety_analysis))";
    break;
  }
  case 1 : {
    OS << " [[gnu::no_address_safety_analysis]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((no_sanitize_address))";
    break;
  }
  case 3 : {
    OS << " [[gnu::no_sanitize_address]]";
    break;
  }
}
}

const char *NoSanitizeAddressAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "no_address_safety_analysis";
  case 1:
    return "no_address_safety_analysis";
  case 2:
    return "no_sanitize_address";
  case 3:
    return "no_sanitize_address";
  }
}

NoSanitizeMemoryAttr *NoSanitizeMemoryAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoSanitizeMemoryAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoSanitizeMemoryAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((no_sanitize_memory))";
    break;
  }
}
}

const char *NoSanitizeMemoryAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "no_sanitize_memory";
  }
}

NoSanitizeThreadAttr *NoSanitizeThreadAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoSanitizeThreadAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoSanitizeThreadAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((no_sanitize_thread))";
    break;
  }
}
}

const char *NoSanitizeThreadAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "no_sanitize_thread";
  }
}

NoSplitStackAttr *NoSplitStackAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoSplitStackAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoSplitStackAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((no_split_stack))";
    break;
  }
  case 1 : {
    OS << " [[gnu::no_split_stack]]";
    break;
  }
}
}

const char *NoSplitStackAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "no_split_stack";
  case 1:
    return "no_split_stack";
  }
}

NoThreadSafetyAnalysisAttr *NoThreadSafetyAnalysisAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoThreadSafetyAnalysisAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoThreadSafetyAnalysisAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((no_thread_safety_analysis))";
    break;
  }
}
}

const char *NoThreadSafetyAnalysisAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "no_thread_safety_analysis";
  }
}

NoThrowAttr *NoThrowAttr::clone(ASTContext &C) const {
  auto *A = new (C) NoThrowAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NoThrowAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((nothrow))";
    break;
  }
  case 1 : {
    OS << " [[gnu::nothrow]]";
    break;
  }
  case 2 : {
    OS << " __declspec(nothrow)";
    break;
  }
}
}

const char *NoThrowAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "nothrow";
  case 1:
    return "nothrow";
  case 2:
    return "nothrow";
  }
}

NonNullAttr *NonNullAttr::clone(ASTContext &C) const {
  auto *A = new (C) NonNullAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void NonNullAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((nonnull(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::nonnull(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
}
}

const char *NonNullAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "nonnull";
  case 1:
    return "nonnull";
  }
}

OMPThreadPrivateDeclAttr *OMPThreadPrivateDeclAttr::clone(ASTContext &C) const {
  auto *A = new (C) OMPThreadPrivateDeclAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OMPThreadPrivateDeclAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
}

const char *OMPThreadPrivateDeclAttr::getSpelling() const {
  return "(No spelling)";
}

ObjCBridgeAttr *ObjCBridgeAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCBridgeAttr(getLocation(), C, bridgedType, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCBridgeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_bridge(" << getBridgedType()->getName() << ")))";
    break;
  }
}
}

const char *ObjCBridgeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_bridge";
  }
}

ObjCBridgeMutableAttr *ObjCBridgeMutableAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCBridgeMutableAttr(getLocation(), C, bridgedType, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCBridgeMutableAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_bridge_mutable(" << getBridgedType()->getName() << ")))";
    break;
  }
}
}

const char *ObjCBridgeMutableAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_bridge_mutable";
  }
}

ObjCBridgeRelatedAttr *ObjCBridgeRelatedAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCBridgeRelatedAttr(getLocation(), C, relatedClass, classMethod, instanceMethod, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCBridgeRelatedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_bridge_related(" << getRelatedClass()->getName() << ", " << getClassMethod()->getName() << ", " << getInstanceMethod()->getName() << ")))";
    break;
  }
}
}

const char *ObjCBridgeRelatedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_bridge_related";
  }
}

ObjCDesignatedInitializerAttr *ObjCDesignatedInitializerAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCDesignatedInitializerAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCDesignatedInitializerAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_designated_initializer))";
    break;
  }
}
}

const char *ObjCDesignatedInitializerAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_designated_initializer";
  }
}

ObjCExceptionAttr *ObjCExceptionAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCExceptionAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCExceptionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_exception))";
    break;
  }
}
}

const char *ObjCExceptionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_exception";
  }
}

ObjCExplicitProtocolImplAttr *ObjCExplicitProtocolImplAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCExplicitProtocolImplAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCExplicitProtocolImplAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_protocol_requires_explicit_implementation))";
    break;
  }
}
}

const char *ObjCExplicitProtocolImplAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_protocol_requires_explicit_implementation";
  }
}

ObjCMethodFamilyAttr *ObjCMethodFamilyAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCMethodFamilyAttr(getLocation(), C, family, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCMethodFamilyAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_method_family(\"" << ObjCMethodFamilyAttr::ConvertFamilyKindToStr(getFamily()) << "\")))";
    break;
  }
}
}

const char *ObjCMethodFamilyAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_method_family";
  }
}

ObjCNSObjectAttr *ObjCNSObjectAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCNSObjectAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCNSObjectAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((NSObject))";
    break;
  }
}
}

const char *ObjCNSObjectAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "NSObject";
  }
}

ObjCPreciseLifetimeAttr *ObjCPreciseLifetimeAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCPreciseLifetimeAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCPreciseLifetimeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_precise_lifetime))";
    break;
  }
}
}

const char *ObjCPreciseLifetimeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_precise_lifetime";
  }
}

ObjCRequiresPropertyDefsAttr *ObjCRequiresPropertyDefsAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCRequiresPropertyDefsAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCRequiresPropertyDefsAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_requires_property_definitions))";
    break;
  }
}
}

const char *ObjCRequiresPropertyDefsAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_requires_property_definitions";
  }
}

ObjCRequiresSuperAttr *ObjCRequiresSuperAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCRequiresSuperAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCRequiresSuperAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_requires_super))";
    break;
  }
}
}

const char *ObjCRequiresSuperAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_requires_super";
  }
}

ObjCReturnsInnerPointerAttr *ObjCReturnsInnerPointerAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCReturnsInnerPointerAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCReturnsInnerPointerAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_returns_inner_pointer))";
    break;
  }
}
}

const char *ObjCReturnsInnerPointerAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_returns_inner_pointer";
  }
}

ObjCRootClassAttr *ObjCRootClassAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCRootClassAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCRootClassAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_root_class))";
    break;
  }
}
}

const char *ObjCRootClassAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_root_class";
  }
}

ObjCRuntimeNameAttr *ObjCRuntimeNameAttr::clone(ASTContext &C) const {
  auto *A = new (C) ObjCRuntimeNameAttr(getLocation(), C, getMetadataName(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ObjCRuntimeNameAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((objc_runtime_name(\"" << getMetadataName() << "\")))";
    break;
  }
}
}

const char *ObjCRuntimeNameAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "objc_runtime_name";
  }
}

OpenCLImageAccessAttr *OpenCLImageAccessAttr::clone(ASTContext &C) const {
  auto *A = new (C) OpenCLImageAccessAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OpenCLImageAccessAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __read_only";
    break;
  }
  case 1 : {
    OS << " read_only";
    break;
  }
  case 2 : {
    OS << " __write_only";
    break;
  }
  case 3 : {
    OS << " write_only";
    break;
  }
  case 4 : {
    OS << " __read_write";
    break;
  }
  case 5 : {
    OS << " read_write";
    break;
  }
}
}

const char *OpenCLImageAccessAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "__read_only";
  case 1:
    return "read_only";
  case 2:
    return "__write_only";
  case 3:
    return "write_only";
  case 4:
    return "__read_write";
  case 5:
    return "read_write";
  }
}

OpenCLKernelAttr *OpenCLKernelAttr::clone(ASTContext &C) const {
  auto *A = new (C) OpenCLKernelAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OpenCLKernelAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __kernel";
    break;
  }
  case 1 : {
    OS << " kernel";
    break;
  }
}
}

const char *OpenCLKernelAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "__kernel";
  case 1:
    return "kernel";
  }
}

OptimizeNoneAttr *OptimizeNoneAttr::clone(ASTContext &C) const {
  auto *A = new (C) OptimizeNoneAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OptimizeNoneAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((optnone))";
    break;
  }
  case 1 : {
    OS << " [[clang::optnone]]";
    break;
  }
}
}

const char *OptimizeNoneAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "optnone";
  case 1:
    return "optnone";
  }
}

OverloadableAttr *OverloadableAttr::clone(ASTContext &C) const {
  auto *A = new (C) OverloadableAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OverloadableAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((overloadable))";
    break;
  }
}
}

const char *OverloadableAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "overloadable";
  }
}

OverrideAttr *OverrideAttr::clone(ASTContext &C) const {
  auto *A = new (C) OverrideAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OverrideAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " override";
    break;
  }
}
}

const char *OverrideAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "override";
  }
}

OwnershipAttr *OwnershipAttr::clone(ASTContext &C) const {
  auto *A = new (C) OwnershipAttr(getLocation(), C, module, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void OwnershipAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((ownership_holds(" << getModule()->getName() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 1 : {
    OS << " __attribute__((ownership_returns(" << getModule()->getName() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 2 : {
    OS << " __attribute__((ownership_takes(" << getModule()->getName() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *OwnershipAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "ownership_holds";
  case 1:
    return "ownership_returns";
  case 2:
    return "ownership_takes";
  }
}

PackedAttr *PackedAttr::clone(ASTContext &C) const {
  auto *A = new (C) PackedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PackedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((packed))";
    break;
  }
  case 1 : {
    OS << " [[gnu::packed]]";
    break;
  }
}
}

const char *PackedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "packed";
  case 1:
    return "packed";
  }
}

ParamTypestateAttr *ParamTypestateAttr::clone(ASTContext &C) const {
  auto *A = new (C) ParamTypestateAttr(getLocation(), C, paramState, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ParamTypestateAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((param_typestate(\"" << ParamTypestateAttr::ConvertConsumedStateToStr(getParamState()) << "\")))";
    break;
  }
}
}

const char *ParamTypestateAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "param_typestate";
  }
}

PascalAttr *PascalAttr::clone(ASTContext &C) const {
  auto *A = new (C) PascalAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PascalAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((pascal))";
    break;
  }
  case 1 : {
    OS << " __pascal";
    break;
  }
  case 2 : {
    OS << " _pascal";
    break;
  }
}
}

const char *PascalAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "pascal";
  case 1:
    return "__pascal";
  case 2:
    return "_pascal";
  }
}

PcsAttr *PcsAttr::clone(ASTContext &C) const {
  auto *A = new (C) PcsAttr(getLocation(), C, pCS, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PcsAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((pcs(\"" << PcsAttr::ConvertPCSTypeToStr(getPCS()) << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::pcs(\"" << PcsAttr::ConvertPCSTypeToStr(getPCS()) << "\")]]";
    break;
  }
}
}

const char *PcsAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "pcs";
  case 1:
    return "pcs";
  }
}

PnaclCallAttr *PnaclCallAttr::clone(ASTContext &C) const {
  auto *A = new (C) PnaclCallAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PnaclCallAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((pnaclcall))";
    break;
  }
}
}

const char *PnaclCallAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "pnaclcall";
  }
}

PtGuardedByAttr *PtGuardedByAttr::clone(ASTContext &C) const {
  auto *A = new (C) PtGuardedByAttr(getLocation(), C, arg, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PtGuardedByAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((pt_guarded_by(" << getArg() << ")))";
    break;
  }
}
}

const char *PtGuardedByAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "pt_guarded_by";
  }
}

PtGuardedVarAttr *PtGuardedVarAttr::clone(ASTContext &C) const {
  auto *A = new (C) PtGuardedVarAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PtGuardedVarAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((pt_guarded_var))";
    break;
  }
}
}

const char *PtGuardedVarAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "pt_guarded_var";
  }
}

PureAttr *PureAttr::clone(ASTContext &C) const {
  auto *A = new (C) PureAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void PureAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((pure))";
    break;
  }
  case 1 : {
    OS << " [[gnu::pure]]";
    break;
  }
}
}

const char *PureAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "pure";
  case 1:
    return "pure";
  }
}

ReleaseCapabilityAttr *ReleaseCapabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) ReleaseCapabilityAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ReleaseCapabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((release_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 1 : {
    OS << " [[clang::release_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((release_shared_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 3 : {
    OS << " [[clang::release_shared_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 4 : {
    OS << " __attribute__((release_generic_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 5 : {
    OS << " [[clang::release_generic_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 6 : {
    OS << " __attribute__((unlock_function(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *ReleaseCapabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "release_capability";
  case 1:
    return "release_capability";
  case 2:
    return "release_shared_capability";
  case 3:
    return "release_shared_capability";
  case 4:
    return "release_generic_capability";
  case 5:
    return "release_generic_capability";
  case 6:
    return "unlock_function";
  }
}

ReqdWorkGroupSizeAttr *ReqdWorkGroupSizeAttr::clone(ASTContext &C) const {
  auto *A = new (C) ReqdWorkGroupSizeAttr(getLocation(), C, xDim, yDim, zDim, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ReqdWorkGroupSizeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((reqd_work_group_size(" << getXDim() << ", " << getYDim() << ", " << getZDim() << ")))";
    break;
  }
}
}

const char *ReqdWorkGroupSizeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "reqd_work_group_size";
  }
}

RequiresCapabilityAttr *RequiresCapabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) RequiresCapabilityAttr(getLocation(), C, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void RequiresCapabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((requires_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 1 : {
    OS << " [[clang::requires_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((exclusive_locks_required(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 3 : {
    OS << " __attribute__((requires_shared_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 4 : {
    OS << " [[clang::requires_shared_capability(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 5 : {
    OS << " __attribute__((shared_locks_required(";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *RequiresCapabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "requires_capability";
  case 1:
    return "requires_capability";
  case 2:
    return "exclusive_locks_required";
  case 3:
    return "requires_shared_capability";
  case 4:
    return "requires_shared_capability";
  case 5:
    return "shared_locks_required";
  }
}

ReturnTypestateAttr *ReturnTypestateAttr::clone(ASTContext &C) const {
  auto *A = new (C) ReturnTypestateAttr(getLocation(), C, state, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ReturnTypestateAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((return_typestate(\"" << ReturnTypestateAttr::ConvertConsumedStateToStr(getState()) << "\")))";
    break;
  }
}
}

const char *ReturnTypestateAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "return_typestate";
  }
}

ReturnsNonNullAttr *ReturnsNonNullAttr::clone(ASTContext &C) const {
  auto *A = new (C) ReturnsNonNullAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ReturnsNonNullAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((returns_nonnull))";
    break;
  }
  case 1 : {
    OS << " [[gnu::returns_nonnull]]";
    break;
  }
}
}

const char *ReturnsNonNullAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "returns_nonnull";
  case 1:
    return "returns_nonnull";
  }
}

ReturnsTwiceAttr *ReturnsTwiceAttr::clone(ASTContext &C) const {
  auto *A = new (C) ReturnsTwiceAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ReturnsTwiceAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((returns_twice))";
    break;
  }
  case 1 : {
    OS << " [[gnu::returns_twice]]";
    break;
  }
}
}

const char *ReturnsTwiceAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "returns_twice";
  case 1:
    return "returns_twice";
  }
}

ScopedLockableAttr *ScopedLockableAttr::clone(ASTContext &C) const {
  auto *A = new (C) ScopedLockableAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ScopedLockableAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((scoped_lockable))";
    break;
  }
}
}

const char *ScopedLockableAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "scoped_lockable";
  }
}

SectionAttr *SectionAttr::clone(ASTContext &C) const {
  auto *A = new (C) SectionAttr(getLocation(), C, getName(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void SectionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((section(\"" << getName() << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::section(\"" << getName() << "\")]]";
    break;
  }
  case 2 : {
    OS << " __declspec(allocate(\"" << getName() << "\"))";
    break;
  }
}
}

const char *SectionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "section";
  case 1:
    return "section";
  case 2:
    return "allocate";
  }
}

SelectAnyAttr *SelectAnyAttr::clone(ASTContext &C) const {
  auto *A = new (C) SelectAnyAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void SelectAnyAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __declspec(selectany)";
    break;
  }
}
}

const char *SelectAnyAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "selectany";
  }
}

SentinelAttr *SentinelAttr::clone(ASTContext &C) const {
  auto *A = new (C) SentinelAttr(getLocation(), C, sentinel, nullPos, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void SentinelAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((sentinel(" << getSentinel() << ", " << getNullPos() << ")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::sentinel(" << getSentinel() << ", " << getNullPos() << ")]]";
    break;
  }
}
}

const char *SentinelAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "sentinel";
  case 1:
    return "sentinel";
  }
}

SetTypestateAttr *SetTypestateAttr::clone(ASTContext &C) const {
  auto *A = new (C) SetTypestateAttr(getLocation(), C, newState, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void SetTypestateAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((set_typestate(\"" << SetTypestateAttr::ConvertConsumedStateToStr(getNewState()) << "\")))";
    break;
  }
}
}

const char *SetTypestateAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "set_typestate";
  }
}

SharedTrylockFunctionAttr *SharedTrylockFunctionAttr::clone(ASTContext &C) const {
  auto *A = new (C) SharedTrylockFunctionAttr(getLocation(), C, successValue, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void SharedTrylockFunctionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((shared_trylock_function(" << getSuccessValue() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
}
}

const char *SharedTrylockFunctionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "shared_trylock_function";
  }
}

StdCallAttr *StdCallAttr::clone(ASTContext &C) const {
  auto *A = new (C) StdCallAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void StdCallAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((stdcall))";
    break;
  }
  case 1 : {
    OS << " [[gnu::stdcall]]";
    break;
  }
  case 2 : {
    OS << " __stdcall";
    break;
  }
  case 3 : {
    OS << " _stdcall";
    break;
  }
}
}

const char *StdCallAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "stdcall";
  case 1:
    return "stdcall";
  case 2:
    return "__stdcall";
  case 3:
    return "_stdcall";
  }
}

SysVABIAttr *SysVABIAttr::clone(ASTContext &C) const {
  auto *A = new (C) SysVABIAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void SysVABIAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((sysv_abi))";
    break;
  }
  case 1 : {
    OS << " [[gnu::sysv_abi]]";
    break;
  }
}
}

const char *SysVABIAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "sysv_abi";
  case 1:
    return "sysv_abi";
  }
}

TLSModelAttr *TLSModelAttr::clone(ASTContext &C) const {
  auto *A = new (C) TLSModelAttr(getLocation(), C, getModel(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void TLSModelAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((tls_model(\"" << getModel() << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::tls_model(\"" << getModel() << "\")]]";
    break;
  }
}
}

const char *TLSModelAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "tls_model";
  case 1:
    return "tls_model";
  }
}

TestTypestateAttr *TestTypestateAttr::clone(ASTContext &C) const {
  auto *A = new (C) TestTypestateAttr(getLocation(), C, testState, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void TestTypestateAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((test_typestate(\"" << TestTypestateAttr::ConvertConsumedStateToStr(getTestState()) << "\")))";
    break;
  }
}
}

const char *TestTypestateAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "test_typestate";
  }
}

ThisCallAttr *ThisCallAttr::clone(ASTContext &C) const {
  auto *A = new (C) ThisCallAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ThisCallAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((thiscall))";
    break;
  }
  case 1 : {
    OS << " [[gnu::thiscall]]";
    break;
  }
  case 2 : {
    OS << " __thiscall";
    break;
  }
  case 3 : {
    OS << " _thiscall";
    break;
  }
}
}

const char *ThisCallAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "thiscall";
  case 1:
    return "thiscall";
  case 2:
    return "__thiscall";
  case 3:
    return "_thiscall";
  }
}

ThreadAttr *ThreadAttr::clone(ASTContext &C) const {
  auto *A = new (C) ThreadAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void ThreadAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __declspec(thread)";
    break;
  }
}
}

const char *ThreadAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "thread";
  }
}

TransparentUnionAttr *TransparentUnionAttr::clone(ASTContext &C) const {
  auto *A = new (C) TransparentUnionAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void TransparentUnionAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((transparent_union))";
    break;
  }
  case 1 : {
    OS << " [[gnu::transparent_union]]";
    break;
  }
}
}

const char *TransparentUnionAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "transparent_union";
  case 1:
    return "transparent_union";
  }
}

TryAcquireCapabilityAttr *TryAcquireCapabilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) TryAcquireCapabilityAttr(getLocation(), C, successValue, args_, args_Size, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void TryAcquireCapabilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((try_acquire_capability(" << getSuccessValue() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 1 : {
    OS << " [[clang::try_acquire_capability(" << getSuccessValue() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
  case 2 : {
    OS << " __attribute__((try_acquire_shared_capability(" << getSuccessValue() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")))";
    break;
  }
  case 3 : {
    OS << " [[clang::try_acquire_shared_capability(" << getSuccessValue() << ", ";
  bool isFirst = true;
  for (const auto &Val : args()) {
    if (isFirst) isFirst = false;
    else OS << ", ";
    OS << Val;
  }
  OS << ")]]";
    break;
  }
}
}

const char *TryAcquireCapabilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "try_acquire_capability";
  case 1:
    return "try_acquire_capability";
  case 2:
    return "try_acquire_shared_capability";
  case 3:
    return "try_acquire_shared_capability";
  }
}

TypeTagForDatatypeAttr *TypeTagForDatatypeAttr::clone(ASTContext &C) const {
  auto *A = new (C) TypeTagForDatatypeAttr(getLocation(), C, argumentKind, matchingCType, layoutCompatible, mustBeNull, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void TypeTagForDatatypeAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((type_tag_for_datatype(" << getArgumentKind()->getName() << ", " << getMatchingCType().getAsString() << ", " << getLayoutCompatible() << ", " << getMustBeNull() << ")))";
    break;
  }
}
}

const char *TypeTagForDatatypeAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "type_tag_for_datatype";
  }
}

TypeVisibilityAttr *TypeVisibilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) TypeVisibilityAttr(getLocation(), C, visibility, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void TypeVisibilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((type_visibility(\"" << TypeVisibilityAttr::ConvertVisibilityTypeToStr(getVisibility()) << "\")))";
    break;
  }
  case 1 : {
    OS << " [[clang::type_visibility(\"" << TypeVisibilityAttr::ConvertVisibilityTypeToStr(getVisibility()) << "\")]]";
    break;
  }
}
}

const char *TypeVisibilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "type_visibility";
  case 1:
    return "type_visibility";
  }
}

UnavailableAttr *UnavailableAttr::clone(ASTContext &C) const {
  auto *A = new (C) UnavailableAttr(getLocation(), C, getMessage(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void UnavailableAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((unavailable(\"" << getMessage() << "\")))";
    break;
  }
}
}

const char *UnavailableAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "unavailable";
  }
}

UnusedAttr *UnusedAttr::clone(ASTContext &C) const {
  auto *A = new (C) UnusedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void UnusedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((unused))";
    break;
  }
  case 1 : {
    OS << " [[gnu::unused]]";
    break;
  }
}
}

const char *UnusedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "unused";
  case 1:
    return "unused";
  }
}

UsedAttr *UsedAttr::clone(ASTContext &C) const {
  auto *A = new (C) UsedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void UsedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((used))";
    break;
  }
  case 1 : {
    OS << " [[gnu::used]]";
    break;
  }
}
}

const char *UsedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "used";
  case 1:
    return "used";
  }
}

UuidAttr *UuidAttr::clone(ASTContext &C) const {
  auto *A = new (C) UuidAttr(getLocation(), C, getGuid(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void UuidAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __declspec(uuid(\"" << getGuid() << "\"))";
    break;
  }
}
}

const char *UuidAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "uuid";
  }
}

VecReturnAttr *VecReturnAttr::clone(ASTContext &C) const {
  auto *A = new (C) VecReturnAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void VecReturnAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((vecreturn))";
    break;
  }
}
}

const char *VecReturnAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "vecreturn";
  }
}

VecTypeHintAttr *VecTypeHintAttr::clone(ASTContext &C) const {
  auto *A = new (C) VecTypeHintAttr(getLocation(), C, typeHint, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void VecTypeHintAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((vec_type_hint(" << getTypeHint().getAsString() << ")))";
    break;
  }
}
}

const char *VecTypeHintAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "vec_type_hint";
  }
}

VectorCallAttr *VectorCallAttr::clone(ASTContext &C) const {
  auto *A = new (C) VectorCallAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void VectorCallAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((vectorcall))";
    break;
  }
  case 1 : {
    OS << " __vectorcall";
    break;
  }
  case 2 : {
    OS << " _vectorcall";
    break;
  }
}
}

const char *VectorCallAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "vectorcall";
  case 1:
    return "__vectorcall";
  case 2:
    return "_vectorcall";
  }
}

VisibilityAttr *VisibilityAttr::clone(ASTContext &C) const {
  auto *A = new (C) VisibilityAttr(getLocation(), C, visibility, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void VisibilityAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((visibility(\"" << VisibilityAttr::ConvertVisibilityTypeToStr(getVisibility()) << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::visibility(\"" << VisibilityAttr::ConvertVisibilityTypeToStr(getVisibility()) << "\")]]";
    break;
  }
}
}

const char *VisibilityAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "visibility";
  case 1:
    return "visibility";
  }
}

WarnUnusedAttr *WarnUnusedAttr::clone(ASTContext &C) const {
  auto *A = new (C) WarnUnusedAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void WarnUnusedAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((warn_unused))";
    break;
  }
}
}

const char *WarnUnusedAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "warn_unused";
  }
}

WarnUnusedResultAttr *WarnUnusedResultAttr::clone(ASTContext &C) const {
  auto *A = new (C) WarnUnusedResultAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void WarnUnusedResultAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((warn_unused_result))";
    break;
  }
  case 1 : {
    OS << " [[gnu::warn_unused_result]]";
    break;
  }
  case 2 : {
    OS << " [[clang::warn_unused_result]]";
    break;
  }
}
}

const char *WarnUnusedResultAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "warn_unused_result";
  case 1:
    return "warn_unused_result";
  case 2:
    return "warn_unused_result";
  }
}

WeakAttr *WeakAttr::clone(ASTContext &C) const {
  auto *A = new (C) WeakAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void WeakAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((weak))";
    break;
  }
  case 1 : {
    OS << " [[gnu::weak]]";
    break;
  }
}
}

const char *WeakAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "weak";
  case 1:
    return "weak";
  }
}

WeakImportAttr *WeakImportAttr::clone(ASTContext &C) const {
  auto *A = new (C) WeakImportAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void WeakImportAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((weak_import))";
    break;
  }
}
}

const char *WeakImportAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "weak_import";
  }
}

WeakRefAttr *WeakRefAttr::clone(ASTContext &C) const {
  auto *A = new (C) WeakRefAttr(getLocation(), C, getAliasee(), getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void WeakRefAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((weakref(\"" << getAliasee() << "\")))";
    break;
  }
  case 1 : {
    OS << " [[gnu::weakref(\"" << getAliasee() << "\")]]";
    break;
  }
}
}

const char *WeakRefAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "weakref";
  case 1:
    return "weakref";
  }
}

WorkGroupSizeHintAttr *WorkGroupSizeHintAttr::clone(ASTContext &C) const {
  auto *A = new (C) WorkGroupSizeHintAttr(getLocation(), C, xDim, yDim, zDim, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void WorkGroupSizeHintAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((work_group_size_hint(" << getXDim() << ", " << getYDim() << ", " << getZDim() << ")))";
    break;
  }
}
}

const char *WorkGroupSizeHintAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "work_group_size_hint";
  }
}

X86ForceAlignArgPointerAttr *X86ForceAlignArgPointerAttr::clone(ASTContext &C) const {
  auto *A = new (C) X86ForceAlignArgPointerAttr(getLocation(), C, getSpellingListIndex());
  A->Inherited = Inherited;
  A->IsPackExpansion = IsPackExpansion;
  A->Implicit = Implicit;
  return A;
}

void X86ForceAlignArgPointerAttr::printPretty(raw_ostream &OS, const PrintingPolicy &Policy) const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    break;
  case 0 : {
    OS << " __attribute__((force_align_arg_pointer))";
    break;
  }
}
}

const char *X86ForceAlignArgPointerAttr::getSpelling() const {
  switch (SpellingListIndex) {
  default:
    llvm_unreachable("Unknown attribute spelling!");
    return "(No spelling)";
  case 0:
    return "force_align_arg_pointer";
  }
}

