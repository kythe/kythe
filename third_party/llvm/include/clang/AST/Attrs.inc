/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Attribute classes' definitions                                              *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_CLANG_ATTR_CLASSES_INC
#define LLVM_CLANG_ATTR_CLASSES_INC

class ARMInterruptAttr : public InheritableAttr {
public:
  enum InterruptType {
    IRQ,
    FIQ,
    SWI,
    ABORT,
    UNDEF,
    Generic
  };
private:
  InterruptType interrupt;


public:
  static ARMInterruptAttr *CreateImplicit(ASTContext &Ctx, InterruptType Interrupt, SourceRange Loc = SourceRange()) {
    ARMInterruptAttr *A = new (Ctx) ARMInterruptAttr(Loc, Ctx, Interrupt, 0);
    A->setImplicit(true);
    return A;
  }

  ARMInterruptAttr(SourceRange R, ASTContext &Ctx
              , InterruptType Interrupt
              , unsigned SI
             )
    : InheritableAttr(attr::ARMInterrupt, R, SI)
              , interrupt(Interrupt)
  {

  }

  ARMInterruptAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ARMInterrupt, R, SI)
              , interrupt(InterruptType(0))
  {
  }

  ARMInterruptAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  InterruptType getInterrupt() const {
    return interrupt;
  }

  static bool ConvertStrToInterruptType(StringRef Val, InterruptType &Out) {
    Optional<InterruptType> R = llvm::StringSwitch<Optional<InterruptType>>(Val)
      .Case("IRQ", ARMInterruptAttr::IRQ)
      .Case("FIQ", ARMInterruptAttr::FIQ)
      .Case("SWI", ARMInterruptAttr::SWI)
      .Case("ABORT", ARMInterruptAttr::ABORT)
      .Case("UNDEF", ARMInterruptAttr::UNDEF)
      .Case("", ARMInterruptAttr::Generic)
      .Default(Optional<InterruptType>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertInterruptTypeToStr(InterruptType Val) {
    switch(Val) {
    case ARMInterruptAttr::IRQ: return "IRQ";
    case ARMInterruptAttr::FIQ: return "FIQ";
    case ARMInterruptAttr::SWI: return "SWI";
    case ARMInterruptAttr::ABORT: return "ABORT";
    case ARMInterruptAttr::UNDEF: return "UNDEF";
    case ARMInterruptAttr::Generic: return "";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::ARMInterrupt; }
  bool isLateParsed() const override { return 0; }
};

class AcquireCapabilityAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  enum Spelling {
    GNU_acquire_capability,
    CXX11_clang_acquire_capability,
    GNU_acquire_shared_capability,
    CXX11_clang_acquire_shared_capability,
    GNU_exclusive_lock_function,
    GNU_shared_lock_function
  };

  static AcquireCapabilityAttr *CreateImplicit(ASTContext &Ctx, Spelling S, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    AcquireCapabilityAttr *A = new (Ctx) AcquireCapabilityAttr(Loc, Ctx, Args, ArgsSize, S);
    A->setImplicit(true);
    return A;
  }

  AcquireCapabilityAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::AcquireCapability, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  AcquireCapabilityAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AcquireCapability, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  AcquireCapabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_acquire_capability;
    case 1: return CXX11_clang_acquire_capability;
    case 2: return GNU_acquire_shared_capability;
    case 3: return CXX11_clang_acquire_shared_capability;
    case 4: return GNU_exclusive_lock_function;
    case 5: return GNU_shared_lock_function;
  }
  }
  bool isShared() const { return SpellingListIndex == 2 ||
    SpellingListIndex == 3 ||
    SpellingListIndex == 5; }
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::AcquireCapability; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class AcquiredAfterAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  static AcquiredAfterAttr *CreateImplicit(ASTContext &Ctx, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    AcquiredAfterAttr *A = new (Ctx) AcquiredAfterAttr(Loc, Ctx, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  AcquiredAfterAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::AcquiredAfter, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  AcquiredAfterAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AcquiredAfter, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  AcquiredAfterAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::AcquiredAfter; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class AcquiredBeforeAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  static AcquiredBeforeAttr *CreateImplicit(ASTContext &Ctx, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    AcquiredBeforeAttr *A = new (Ctx) AcquiredBeforeAttr(Loc, Ctx, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  AcquiredBeforeAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::AcquiredBefore, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  AcquiredBeforeAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AcquiredBefore, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  AcquiredBeforeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::AcquiredBefore; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class AliasAttr : public Attr {
unsigned aliaseeLength;
char *aliasee;


public:
  static AliasAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Aliasee, SourceRange Loc = SourceRange()) {
    AliasAttr *A = new (Ctx) AliasAttr(Loc, Ctx, Aliasee, 0);
    A->setImplicit(true);
    return A;
  }

  AliasAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Aliasee
              , unsigned SI
             )
    : Attr(attr::Alias, R, SI)
              , aliaseeLength(Aliasee.size()),aliasee(new (Ctx, 1) char[aliaseeLength])
  {
      std::memcpy(aliasee, Aliasee.data(), aliaseeLength);
  }

  AliasAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getAliasee() const {
    return llvm::StringRef(aliasee, aliaseeLength);
  }
  unsigned getAliaseeLength() const {
    return aliaseeLength;
  }
  void setAliasee(ASTContext &C, llvm::StringRef S) {
    aliaseeLength = S.size();
    this->aliasee = new (C, 1) char [aliaseeLength];
    std::memcpy(this->aliasee, S.data(), aliaseeLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Alias; }
  bool isLateParsed() const override { return 0; }
};

class AlignMac68kAttr : public InheritableAttr {

public:
  static AlignMac68kAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    AlignMac68kAttr *A = new (Ctx) AlignMac68kAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  AlignMac68kAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AlignMac68k, R, SI)
  {
  }

  AlignMac68kAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::AlignMac68k; }
  bool isLateParsed() const override { return 0; }
};

class AlignValueAttr : public Attr {
Expr * alignment;


public:
  static AlignValueAttr *CreateImplicit(ASTContext &Ctx, Expr * Alignment, SourceRange Loc = SourceRange()) {
    AlignValueAttr *A = new (Ctx) AlignValueAttr(Loc, Ctx, Alignment, 0);
    A->setImplicit(true);
    return A;
  }

  AlignValueAttr(SourceRange R, ASTContext &Ctx
              , Expr * Alignment
              , unsigned SI
             )
    : Attr(attr::AlignValue, R, SI)
              , alignment(Alignment)
  {

  }

  AlignValueAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getAlignment() const {
    return alignment;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::AlignValue; }
  bool isLateParsed() const override { return 0; }
};

class AlignedAttr : public InheritableAttr {
bool isalignmentExpr;
union {
Expr *alignmentExpr;
TypeSourceInfo *alignmentType;
};


public:
  enum Spelling {
    GNU_aligned,
    CXX11_gnu_aligned,
    Declspec_align,
    Keyword_alignas,
    Keyword_Alignas
  };

  static AlignedAttr *CreateImplicit(ASTContext &Ctx, Spelling S, bool IsAlignmentExpr, void *Alignment, SourceRange Loc = SourceRange()) {
    AlignedAttr *A = new (Ctx) AlignedAttr(Loc, Ctx, IsAlignmentExpr, Alignment, S);
    A->setImplicit(true);
    return A;
  }

  AlignedAttr(SourceRange R, ASTContext &Ctx
              , bool IsAlignmentExpr, void *Alignment
              , unsigned SI
             )
    : InheritableAttr(attr::Aligned, R, SI)
              , isalignmentExpr(IsAlignmentExpr)
  {
    if (isalignmentExpr)
       alignmentExpr = reinterpret_cast<Expr *>(Alignment);
    else
       alignmentType = reinterpret_cast<TypeSourceInfo *>(Alignment);
  }

  AlignedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Aligned, R, SI)
              , isalignmentExpr(false)
  {
  }

  AlignedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_aligned;
    case 1: return CXX11_gnu_aligned;
    case 2: return Declspec_align;
    case 3: return Keyword_alignas;
    case 4: return Keyword_Alignas;
  }
  }
  bool isGNU() const { return SpellingListIndex == 0 ||
    SpellingListIndex == 1; }
  bool isC11() const { return SpellingListIndex == 4; }
  bool isAlignas() const { return SpellingListIndex == 3 ||
    SpellingListIndex == 4; }
  bool isDeclspec() const { return SpellingListIndex == 2; }
  bool isAlignmentDependent() const;
  unsigned getAlignment(ASTContext &Ctx) const;
  bool isAlignmentExpr() const {
    return isalignmentExpr;
  }
  Expr *getAlignmentExpr() const {
    assert(isalignmentExpr);
    return alignmentExpr;
  }
  TypeSourceInfo *getAlignmentType() const {
    assert(!isalignmentExpr);
    return alignmentType;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Aligned; }
  bool isLateParsed() const override { return 0; }
};

class AlwaysInlineAttr : public InheritableAttr {

public:
  enum Spelling {
    GNU_always_inline,
    CXX11_gnu_always_inline,
    Keyword_forceinline
  };

  static AlwaysInlineAttr *CreateImplicit(ASTContext &Ctx, Spelling S, SourceRange Loc = SourceRange()) {
    AlwaysInlineAttr *A = new (Ctx) AlwaysInlineAttr(Loc, Ctx, S);
    A->setImplicit(true);
    return A;
  }

  AlwaysInlineAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AlwaysInline, R, SI)
  {
  }

  AlwaysInlineAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_always_inline;
    case 1: return CXX11_gnu_always_inline;
    case 2: return Keyword_forceinline;
  }
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::AlwaysInline; }
  bool isLateParsed() const override { return 0; }
};

class AnalyzerNoReturnAttr : public InheritableAttr {

public:
  static AnalyzerNoReturnAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    AnalyzerNoReturnAttr *A = new (Ctx) AnalyzerNoReturnAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  AnalyzerNoReturnAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AnalyzerNoReturn, R, SI)
  {
  }

  AnalyzerNoReturnAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::AnalyzerNoReturn; }
  bool isLateParsed() const override { return 0; }
};

class AnnotateAttr : public InheritableParamAttr {
unsigned annotationLength;
char *annotation;


public:
  static AnnotateAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Annotation, SourceRange Loc = SourceRange()) {
    AnnotateAttr *A = new (Ctx) AnnotateAttr(Loc, Ctx, Annotation, 0);
    A->setImplicit(true);
    return A;
  }

  AnnotateAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Annotation
              , unsigned SI
             )
    : InheritableParamAttr(attr::Annotate, R, SI)
              , annotationLength(Annotation.size()),annotation(new (Ctx, 1) char[annotationLength])
  {
      std::memcpy(annotation, Annotation.data(), annotationLength);
  }

  AnnotateAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getAnnotation() const {
    return llvm::StringRef(annotation, annotationLength);
  }
  unsigned getAnnotationLength() const {
    return annotationLength;
  }
  void setAnnotation(ASTContext &C, llvm::StringRef S) {
    annotationLength = S.size();
    this->annotation = new (C, 1) char [annotationLength];
    std::memcpy(this->annotation, S.data(), annotationLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Annotate; }
  bool isLateParsed() const override { return 0; }
};

class ArcWeakrefUnavailableAttr : public InheritableAttr {

public:
  static ArcWeakrefUnavailableAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ArcWeakrefUnavailableAttr *A = new (Ctx) ArcWeakrefUnavailableAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ArcWeakrefUnavailableAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ArcWeakrefUnavailable, R, SI)
  {
  }

  ArcWeakrefUnavailableAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ArcWeakrefUnavailable; }
  bool isLateParsed() const override { return 0; }
};

class ArgumentWithTypeTagAttr : public InheritableAttr {
IdentifierInfo * argumentKind;

unsigned argumentIdx;

unsigned typeTagIdx;

bool isPointer;


public:
  enum Spelling {
    GNU_argument_with_type_tag,
    GNU_pointer_with_type_tag
  };

  static ArgumentWithTypeTagAttr *CreateImplicit(ASTContext &Ctx, Spelling S, IdentifierInfo * ArgumentKind, unsigned ArgumentIdx, unsigned TypeTagIdx, bool IsPointer, SourceRange Loc = SourceRange()) {
    ArgumentWithTypeTagAttr *A = new (Ctx) ArgumentWithTypeTagAttr(Loc, Ctx, ArgumentKind, ArgumentIdx, TypeTagIdx, IsPointer, S);
    A->setImplicit(true);
    return A;
  }

  ArgumentWithTypeTagAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * ArgumentKind
              , unsigned ArgumentIdx
              , unsigned TypeTagIdx
              , bool IsPointer
              , unsigned SI
             )
    : InheritableAttr(attr::ArgumentWithTypeTag, R, SI)
              , argumentKind(ArgumentKind)
              , argumentIdx(ArgumentIdx)
              , typeTagIdx(TypeTagIdx)
              , isPointer(IsPointer)
  {




  }

  ArgumentWithTypeTagAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_argument_with_type_tag;
    case 1: return GNU_pointer_with_type_tag;
  }
  }
  IdentifierInfo * getArgumentKind() const {
    return argumentKind;
  }

  unsigned getArgumentIdx() const {
    return argumentIdx;
  }

  unsigned getTypeTagIdx() const {
    return typeTagIdx;
  }

  bool getIsPointer() const {
    return isPointer;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::ArgumentWithTypeTag; }
  bool isLateParsed() const override { return 0; }
};

class AsmLabelAttr : public InheritableAttr {
unsigned labelLength;
char *label;


public:
  static AsmLabelAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Label, SourceRange Loc = SourceRange()) {
    AsmLabelAttr *A = new (Ctx) AsmLabelAttr(Loc, Ctx, Label, 0);
    A->setImplicit(true);
    return A;
  }

  AsmLabelAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Label
              , unsigned SI
             )
    : InheritableAttr(attr::AsmLabel, R, SI)
              , labelLength(Label.size()),label(new (Ctx, 1) char[labelLength])
  {
      std::memcpy(label, Label.data(), labelLength);
  }

  AsmLabelAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getLabel() const {
    return llvm::StringRef(label, labelLength);
  }
  unsigned getLabelLength() const {
    return labelLength;
  }
  void setLabel(ASTContext &C, llvm::StringRef S) {
    labelLength = S.size();
    this->label = new (C, 1) char [labelLength];
    std::memcpy(this->label, S.data(), labelLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::AsmLabel; }
  bool isLateParsed() const override { return 0; }
};

class AssertCapabilityAttr : public InheritableAttr {
Expr * expr;


public:
  enum Spelling {
    GNU_assert_capability,
    CXX11_clang_assert_capability,
    GNU_assert_shared_capability,
    CXX11_clang_assert_shared_capability
  };

  static AssertCapabilityAttr *CreateImplicit(ASTContext &Ctx, Spelling S, Expr * Expr, SourceRange Loc = SourceRange()) {
    AssertCapabilityAttr *A = new (Ctx) AssertCapabilityAttr(Loc, Ctx, Expr, S);
    A->setImplicit(true);
    return A;
  }

  AssertCapabilityAttr(SourceRange R, ASTContext &Ctx
              , Expr * Expr
              , unsigned SI
             )
    : InheritableAttr(attr::AssertCapability, R, SI)
              , expr(Expr)
  {

  }

  AssertCapabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_assert_capability;
    case 1: return CXX11_clang_assert_capability;
    case 2: return GNU_assert_shared_capability;
    case 3: return CXX11_clang_assert_shared_capability;
  }
  }
  bool isShared() const { return SpellingListIndex == 2 ||
    SpellingListIndex == 3; }
  Expr * getExpr() const {
    return expr;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::AssertCapability; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class AssertExclusiveLockAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  static AssertExclusiveLockAttr *CreateImplicit(ASTContext &Ctx, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    AssertExclusiveLockAttr *A = new (Ctx) AssertExclusiveLockAttr(Loc, Ctx, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  AssertExclusiveLockAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::AssertExclusiveLock, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  AssertExclusiveLockAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AssertExclusiveLock, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  AssertExclusiveLockAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::AssertExclusiveLock; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class AssertSharedLockAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  static AssertSharedLockAttr *CreateImplicit(ASTContext &Ctx, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    AssertSharedLockAttr *A = new (Ctx) AssertSharedLockAttr(Loc, Ctx, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  AssertSharedLockAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::AssertSharedLock, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  AssertSharedLockAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::AssertSharedLock, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  AssertSharedLockAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::AssertSharedLock; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class AssumeAlignedAttr : public InheritableAttr {
Expr * alignment;

Expr * offset;


public:
  static AssumeAlignedAttr *CreateImplicit(ASTContext &Ctx, Expr * Alignment, Expr * Offset, SourceRange Loc = SourceRange()) {
    AssumeAlignedAttr *A = new (Ctx) AssumeAlignedAttr(Loc, Ctx, Alignment, Offset, 0);
    A->setImplicit(true);
    return A;
  }

  AssumeAlignedAttr(SourceRange R, ASTContext &Ctx
              , Expr * Alignment
              , Expr * Offset
              , unsigned SI
             )
    : InheritableAttr(attr::AssumeAligned, R, SI)
              , alignment(Alignment)
              , offset(Offset)
  {


  }

  AssumeAlignedAttr(SourceRange R, ASTContext &Ctx
              , Expr * Alignment
              , unsigned SI
             )
    : InheritableAttr(attr::AssumeAligned, R, SI)
              , alignment()
              , offset()
  {

  }

  AssumeAlignedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getAlignment() const {
    return alignment;
  }

  Expr * getOffset() const {
    return offset;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::AssumeAligned; }
  bool isLateParsed() const override { return 0; }
};

class AvailabilityAttr : public InheritableAttr {
IdentifierInfo * platform;

VersionTuple introduced;


VersionTuple deprecated;


VersionTuple obsoleted;


bool unavailable;

unsigned messageLength;
char *message;


public:
  static AvailabilityAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * Platform, VersionTuple Introduced, VersionTuple Deprecated, VersionTuple Obsoleted, bool Unavailable, llvm::StringRef Message, SourceRange Loc = SourceRange()) {
    AvailabilityAttr *A = new (Ctx) AvailabilityAttr(Loc, Ctx, Platform, Introduced, Deprecated, Obsoleted, Unavailable, Message, 0);
    A->setImplicit(true);
    return A;
  }

  AvailabilityAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * Platform
              , VersionTuple Introduced
              , VersionTuple Deprecated
              , VersionTuple Obsoleted
              , bool Unavailable
              , llvm::StringRef Message
              , unsigned SI
             )
    : InheritableAttr(attr::Availability, R, SI)
              , platform(Platform)
              , introduced(Introduced)
              , deprecated(Deprecated)
              , obsoleted(Obsoleted)
              , unavailable(Unavailable)
              , messageLength(Message.size()),message(new (Ctx, 1) char[messageLength])
  {





      std::memcpy(message, Message.data(), messageLength);
  }

  AvailabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getPlatform() const {
    return platform;
  }

  VersionTuple getIntroduced() const {
    return introduced;
  }
  void setIntroduced(ASTContext &C, VersionTuple V) {
    introduced = V;
  }

  VersionTuple getDeprecated() const {
    return deprecated;
  }
  void setDeprecated(ASTContext &C, VersionTuple V) {
    deprecated = V;
  }

  VersionTuple getObsoleted() const {
    return obsoleted;
  }
  void setObsoleted(ASTContext &C, VersionTuple V) {
    obsoleted = V;
  }

  bool getUnavailable() const {
    return unavailable;
  }

  llvm::StringRef getMessage() const {
    return llvm::StringRef(message, messageLength);
  }
  unsigned getMessageLength() const {
    return messageLength;
  }
  void setMessage(ASTContext &C, llvm::StringRef S) {
    messageLength = S.size();
    this->message = new (C, 1) char [messageLength];
    std::memcpy(this->message, S.data(), messageLength);
  }

static llvm::StringRef getPrettyPlatformName(llvm::StringRef Platform) {
    return llvm::StringSwitch<llvm::StringRef>(Platform)
             .Case("ios", "iOS")
             .Case("macosx", "OS X")
             .Default(llvm::StringRef());
} 

  static bool classof(const Attr *A) { return A->getKind() == attr::Availability; }
  bool isLateParsed() const override { return 0; }
  bool duplicatesAllowed() const override { return true; }

};

class BlocksAttr : public InheritableAttr {
public:
  enum BlockType {
    ByRef
  };
private:
  BlockType type;


public:
  static BlocksAttr *CreateImplicit(ASTContext &Ctx, BlockType Type, SourceRange Loc = SourceRange()) {
    BlocksAttr *A = new (Ctx) BlocksAttr(Loc, Ctx, Type, 0);
    A->setImplicit(true);
    return A;
  }

  BlocksAttr(SourceRange R, ASTContext &Ctx
              , BlockType Type
              , unsigned SI
             )
    : InheritableAttr(attr::Blocks, R, SI)
              , type(Type)
  {

  }

  BlocksAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  BlockType getType() const {
    return type;
  }

  static bool ConvertStrToBlockType(StringRef Val, BlockType &Out) {
    Optional<BlockType> R = llvm::StringSwitch<Optional<BlockType>>(Val)
      .Case("byref", BlocksAttr::ByRef)
      .Default(Optional<BlockType>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertBlockTypeToStr(BlockType Val) {
    switch(Val) {
    case BlocksAttr::ByRef: return "byref";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::Blocks; }
  bool isLateParsed() const override { return 0; }
};

class C11NoReturnAttr : public InheritableAttr {

public:
  static C11NoReturnAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    C11NoReturnAttr *A = new (Ctx) C11NoReturnAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  C11NoReturnAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::C11NoReturn, R, SI)
  {
  }

  C11NoReturnAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::C11NoReturn; }
  bool isLateParsed() const override { return 0; }
};

class CDeclAttr : public InheritableAttr {

public:
  static CDeclAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CDeclAttr *A = new (Ctx) CDeclAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CDeclAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CDecl, R, SI)
  {
  }

  CDeclAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CDecl; }
  bool isLateParsed() const override { return 0; }
};

class CFAuditedTransferAttr : public InheritableAttr {

public:
  static CFAuditedTransferAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CFAuditedTransferAttr *A = new (Ctx) CFAuditedTransferAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CFAuditedTransferAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CFAuditedTransfer, R, SI)
  {
  }

  CFAuditedTransferAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CFAuditedTransfer; }
  bool isLateParsed() const override { return 0; }
};

class CFConsumedAttr : public InheritableParamAttr {

public:
  static CFConsumedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CFConsumedAttr *A = new (Ctx) CFConsumedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CFConsumedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableParamAttr(attr::CFConsumed, R, SI)
  {
  }

  CFConsumedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CFConsumed; }
  bool isLateParsed() const override { return 0; }
};

class CFReturnsNotRetainedAttr : public InheritableAttr {

public:
  static CFReturnsNotRetainedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CFReturnsNotRetainedAttr *A = new (Ctx) CFReturnsNotRetainedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CFReturnsNotRetainedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CFReturnsNotRetained, R, SI)
  {
  }

  CFReturnsNotRetainedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CFReturnsNotRetained; }
  bool isLateParsed() const override { return 0; }
};

class CFReturnsRetainedAttr : public InheritableAttr {

public:
  static CFReturnsRetainedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CFReturnsRetainedAttr *A = new (Ctx) CFReturnsRetainedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CFReturnsRetainedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CFReturnsRetained, R, SI)
  {
  }

  CFReturnsRetainedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CFReturnsRetained; }
  bool isLateParsed() const override { return 0; }
};

class CFUnknownTransferAttr : public InheritableAttr {

public:
  static CFUnknownTransferAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CFUnknownTransferAttr *A = new (Ctx) CFUnknownTransferAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CFUnknownTransferAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CFUnknownTransfer, R, SI)
  {
  }

  CFUnknownTransferAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CFUnknownTransfer; }
  bool isLateParsed() const override { return 0; }
};

class CUDAConstantAttr : public InheritableAttr {

public:
  static CUDAConstantAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CUDAConstantAttr *A = new (Ctx) CUDAConstantAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CUDAConstantAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CUDAConstant, R, SI)
  {
  }

  CUDAConstantAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CUDAConstant; }
  bool isLateParsed() const override { return 0; }
};

class CUDADeviceAttr : public InheritableAttr {

public:
  static CUDADeviceAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CUDADeviceAttr *A = new (Ctx) CUDADeviceAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CUDADeviceAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CUDADevice, R, SI)
  {
  }

  CUDADeviceAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CUDADevice; }
  bool isLateParsed() const override { return 0; }
};

class CUDAGlobalAttr : public InheritableAttr {

public:
  static CUDAGlobalAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CUDAGlobalAttr *A = new (Ctx) CUDAGlobalAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CUDAGlobalAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CUDAGlobal, R, SI)
  {
  }

  CUDAGlobalAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CUDAGlobal; }
  bool isLateParsed() const override { return 0; }
};

class CUDAHostAttr : public InheritableAttr {

public:
  static CUDAHostAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CUDAHostAttr *A = new (Ctx) CUDAHostAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CUDAHostAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CUDAHost, R, SI)
  {
  }

  CUDAHostAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CUDAHost; }
  bool isLateParsed() const override { return 0; }
};

class CUDAInvalidTargetAttr : public InheritableAttr {

public:
  static CUDAInvalidTargetAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CUDAInvalidTargetAttr *A = new (Ctx) CUDAInvalidTargetAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CUDAInvalidTargetAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CUDAInvalidTarget, R, SI)
  {
  }

  CUDAInvalidTargetAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CUDAInvalidTarget; }
  bool isLateParsed() const override { return 0; }
};

class CUDALaunchBoundsAttr : public InheritableAttr {
int maxThreads;

int minBlocks;


public:
  static CUDALaunchBoundsAttr *CreateImplicit(ASTContext &Ctx, int MaxThreads, int MinBlocks, SourceRange Loc = SourceRange()) {
    CUDALaunchBoundsAttr *A = new (Ctx) CUDALaunchBoundsAttr(Loc, Ctx, MaxThreads, MinBlocks, 0);
    A->setImplicit(true);
    return A;
  }

  CUDALaunchBoundsAttr(SourceRange R, ASTContext &Ctx
              , int MaxThreads
              , int MinBlocks
              , unsigned SI
             )
    : InheritableAttr(attr::CUDALaunchBounds, R, SI)
              , maxThreads(MaxThreads)
              , minBlocks(MinBlocks)
  {


  }

  CUDALaunchBoundsAttr(SourceRange R, ASTContext &Ctx
              , int MaxThreads
              , unsigned SI
             )
    : InheritableAttr(attr::CUDALaunchBounds, R, SI)
              , maxThreads()
              , minBlocks()
  {

  }

  CUDALaunchBoundsAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  int getMaxThreads() const {
    return maxThreads;
  }

  int getMinBlocks() const {
    return minBlocks;
  }

  static const int DefaultMinBlocks = 0;



  static bool classof(const Attr *A) { return A->getKind() == attr::CUDALaunchBounds; }
  bool isLateParsed() const override { return 0; }
};

class CUDASharedAttr : public InheritableAttr {

public:
  static CUDASharedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CUDASharedAttr *A = new (Ctx) CUDASharedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CUDASharedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CUDAShared, R, SI)
  {
  }

  CUDASharedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CUDAShared; }
  bool isLateParsed() const override { return 0; }
};

class CXX11NoReturnAttr : public InheritableAttr {

public:
  static CXX11NoReturnAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CXX11NoReturnAttr *A = new (Ctx) CXX11NoReturnAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CXX11NoReturnAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CXX11NoReturn, R, SI)
  {
  }

  CXX11NoReturnAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CXX11NoReturn; }
  bool isLateParsed() const override { return 0; }
};

class CallableWhenAttr : public InheritableAttr {
public:
  enum ConsumedState {
    Unknown,
    Consumed,
    Unconsumed
  };
private:
  unsigned callableStates_Size;
  ConsumedState *callableStates_;


public:
  static CallableWhenAttr *CreateImplicit(ASTContext &Ctx, ConsumedState *CallableStates, unsigned CallableStatesSize, SourceRange Loc = SourceRange()) {
    CallableWhenAttr *A = new (Ctx) CallableWhenAttr(Loc, Ctx, CallableStates, CallableStatesSize, 0);
    A->setImplicit(true);
    return A;
  }

  CallableWhenAttr(SourceRange R, ASTContext &Ctx
              , ConsumedState *CallableStates, unsigned CallableStatesSize
              , unsigned SI
             )
    : InheritableAttr(attr::CallableWhen, R, SI)
              , callableStates_Size(CallableStatesSize), callableStates_(new (Ctx, 16) ConsumedState[callableStates_Size])
  {
    std::copy(CallableStates, CallableStates + callableStates_Size, callableStates_);
  }

  CallableWhenAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CallableWhen, R, SI)
              , callableStates_Size(0), callableStates_(nullptr)
  {
  }

  CallableWhenAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef ConsumedState* callableStates_iterator;
  callableStates_iterator callableStates_begin() const { return callableStates_; }
  callableStates_iterator callableStates_end() const { return callableStates_ + callableStates_Size; }
  unsigned callableStates_size() const { return callableStates_Size; }
  llvm::iterator_range<callableStates_iterator> callableStates() const { return llvm::make_range(callableStates_begin(), callableStates_end()); }


  static bool ConvertStrToConsumedState(StringRef Val, ConsumedState &Out) {
    Optional<ConsumedState> R = llvm::StringSwitch<Optional<ConsumedState>>(Val)
      .Case("unknown", CallableWhenAttr::Unknown)
      .Case("consumed", CallableWhenAttr::Consumed)
      .Case("unconsumed", CallableWhenAttr::Unconsumed)
      .Default(Optional<ConsumedState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertConsumedStateToStr(ConsumedState Val) {
    switch(Val) {
    case CallableWhenAttr::Unknown: return "unknown";
    case CallableWhenAttr::Consumed: return "consumed";
    case CallableWhenAttr::Unconsumed: return "unconsumed";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::CallableWhen; }
  bool isLateParsed() const override { return 0; }
};

class CapabilityAttr : public InheritableAttr {
unsigned nameLength;
char *name;


public:
  enum Spelling {
    GNU_capability,
    CXX11_clang_capability,
    GNU_shared_capability,
    CXX11_clang_shared_capability
  };

  static CapabilityAttr *CreateImplicit(ASTContext &Ctx, Spelling S, llvm::StringRef Name, SourceRange Loc = SourceRange()) {
    CapabilityAttr *A = new (Ctx) CapabilityAttr(Loc, Ctx, Name, S);
    A->setImplicit(true);
    return A;
  }

  CapabilityAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Name
              , unsigned SI
             )
    : InheritableAttr(attr::Capability, R, SI)
              , nameLength(Name.size()),name(new (Ctx, 1) char[nameLength])
  {
      std::memcpy(name, Name.data(), nameLength);
  }

  CapabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_capability;
    case 1: return CXX11_clang_capability;
    case 2: return GNU_shared_capability;
    case 3: return CXX11_clang_shared_capability;
  }
  }
  bool isShared() const { return SpellingListIndex == 2 ||
    SpellingListIndex == 3; }
  llvm::StringRef getName() const {
    return llvm::StringRef(name, nameLength);
  }
  unsigned getNameLength() const {
    return nameLength;
  }
  void setName(ASTContext &C, llvm::StringRef S) {
    nameLength = S.size();
    this->name = new (C, 1) char [nameLength];
    std::memcpy(this->name, S.data(), nameLength);
  }


    bool isMutex() const { return getName().equals_lower("mutex"); }
    bool isRole() const { return getName().equals_lower("role"); }
  

  static bool classof(const Attr *A) { return A->getKind() == attr::Capability; }
  bool isLateParsed() const override { return 0; }
};

class CapturedRecordAttr : public InheritableAttr {

public:
  static CapturedRecordAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CapturedRecordAttr *A = new (Ctx) CapturedRecordAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CapturedRecordAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::CapturedRecord, R, SI)
  {
  }

  CapturedRecordAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CapturedRecord; }
  bool isLateParsed() const override { return 0; }
};

class CarriesDependencyAttr : public InheritableParamAttr {

public:
  static CarriesDependencyAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CarriesDependencyAttr *A = new (Ctx) CarriesDependencyAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CarriesDependencyAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableParamAttr(attr::CarriesDependency, R, SI)
  {
  }

  CarriesDependencyAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::CarriesDependency; }
  bool isLateParsed() const override { return 0; }
};

class CleanupAttr : public InheritableAttr {
FunctionDecl * functionDecl;


public:
  static CleanupAttr *CreateImplicit(ASTContext &Ctx, FunctionDecl * FunctionDecl, SourceRange Loc = SourceRange()) {
    CleanupAttr *A = new (Ctx) CleanupAttr(Loc, Ctx, FunctionDecl, 0);
    A->setImplicit(true);
    return A;
  }

  CleanupAttr(SourceRange R, ASTContext &Ctx
              , FunctionDecl * FunctionDecl
              , unsigned SI
             )
    : InheritableAttr(attr::Cleanup, R, SI)
              , functionDecl(FunctionDecl)
  {

  }

  CleanupAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  FunctionDecl * getFunctionDecl() const {
    return functionDecl;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Cleanup; }
  bool isLateParsed() const override { return 0; }
};

class ColdAttr : public InheritableAttr {

public:
  static ColdAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ColdAttr *A = new (Ctx) ColdAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ColdAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Cold, R, SI)
  {
  }

  ColdAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Cold; }
  bool isLateParsed() const override { return 0; }
};

class CommonAttr : public InheritableAttr {

public:
  static CommonAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    CommonAttr *A = new (Ctx) CommonAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  CommonAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Common, R, SI)
  {
  }

  CommonAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Common; }
  bool isLateParsed() const override { return 0; }
};

class ConstAttr : public InheritableAttr {

public:
  static ConstAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ConstAttr *A = new (Ctx) ConstAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ConstAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Const, R, SI)
  {
  }

  ConstAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Const; }
  bool isLateParsed() const override { return 0; }
};

class ConstructorAttr : public InheritableAttr {
int priority;


public:
  static ConstructorAttr *CreateImplicit(ASTContext &Ctx, int Priority, SourceRange Loc = SourceRange()) {
    ConstructorAttr *A = new (Ctx) ConstructorAttr(Loc, Ctx, Priority, 0);
    A->setImplicit(true);
    return A;
  }

  ConstructorAttr(SourceRange R, ASTContext &Ctx
              , int Priority
              , unsigned SI
             )
    : InheritableAttr(attr::Constructor, R, SI)
              , priority(Priority)
  {

  }

  ConstructorAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Constructor, R, SI)
              , priority()
  {
  }

  ConstructorAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  int getPriority() const {
    return priority;
  }

  static const int DefaultPriority = 65535;



  static bool classof(const Attr *A) { return A->getKind() == attr::Constructor; }
  bool isLateParsed() const override { return 0; }
};

class ConsumableAttr : public InheritableAttr {
public:
  enum ConsumedState {
    Unknown,
    Consumed,
    Unconsumed
  };
private:
  ConsumedState defaultState;


public:
  static ConsumableAttr *CreateImplicit(ASTContext &Ctx, ConsumedState DefaultState, SourceRange Loc = SourceRange()) {
    ConsumableAttr *A = new (Ctx) ConsumableAttr(Loc, Ctx, DefaultState, 0);
    A->setImplicit(true);
    return A;
  }

  ConsumableAttr(SourceRange R, ASTContext &Ctx
              , ConsumedState DefaultState
              , unsigned SI
             )
    : InheritableAttr(attr::Consumable, R, SI)
              , defaultState(DefaultState)
  {

  }

  ConsumableAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  ConsumedState getDefaultState() const {
    return defaultState;
  }

  static bool ConvertStrToConsumedState(StringRef Val, ConsumedState &Out) {
    Optional<ConsumedState> R = llvm::StringSwitch<Optional<ConsumedState>>(Val)
      .Case("unknown", ConsumableAttr::Unknown)
      .Case("consumed", ConsumableAttr::Consumed)
      .Case("unconsumed", ConsumableAttr::Unconsumed)
      .Default(Optional<ConsumedState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertConsumedStateToStr(ConsumedState Val) {
    switch(Val) {
    case ConsumableAttr::Unknown: return "unknown";
    case ConsumableAttr::Consumed: return "consumed";
    case ConsumableAttr::Unconsumed: return "unconsumed";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::Consumable; }
  bool isLateParsed() const override { return 0; }
};

class ConsumableAutoCastAttr : public InheritableAttr {

public:
  static ConsumableAutoCastAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ConsumableAutoCastAttr *A = new (Ctx) ConsumableAutoCastAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ConsumableAutoCastAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ConsumableAutoCast, R, SI)
  {
  }

  ConsumableAutoCastAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ConsumableAutoCast; }
  bool isLateParsed() const override { return 0; }
};

class ConsumableSetOnReadAttr : public InheritableAttr {

public:
  static ConsumableSetOnReadAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ConsumableSetOnReadAttr *A = new (Ctx) ConsumableSetOnReadAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ConsumableSetOnReadAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ConsumableSetOnRead, R, SI)
  {
  }

  ConsumableSetOnReadAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ConsumableSetOnRead; }
  bool isLateParsed() const override { return 0; }
};

class DLLExportAttr : public InheritableAttr {

public:
  static DLLExportAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    DLLExportAttr *A = new (Ctx) DLLExportAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  DLLExportAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::DLLExport, R, SI)
  {
  }

  DLLExportAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::DLLExport; }
  bool isLateParsed() const override { return 0; }
};

class DLLImportAttr : public InheritableAttr {

public:
  static DLLImportAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    DLLImportAttr *A = new (Ctx) DLLImportAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  DLLImportAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::DLLImport, R, SI)
  {
  }

  DLLImportAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::DLLImport; }
  bool isLateParsed() const override { return 0; }
};

class DeprecatedAttr : public InheritableAttr {
unsigned messageLength;
char *message;


public:
  static DeprecatedAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Message, SourceRange Loc = SourceRange()) {
    DeprecatedAttr *A = new (Ctx) DeprecatedAttr(Loc, Ctx, Message, 0);
    A->setImplicit(true);
    return A;
  }

  DeprecatedAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Message
              , unsigned SI
             )
    : InheritableAttr(attr::Deprecated, R, SI)
              , messageLength(Message.size()),message(new (Ctx, 1) char[messageLength])
  {
      std::memcpy(message, Message.data(), messageLength);
  }

  DeprecatedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Deprecated, R, SI)
              , messageLength(0),message(0)
  {
  }

  DeprecatedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getMessage() const {
    return llvm::StringRef(message, messageLength);
  }
  unsigned getMessageLength() const {
    return messageLength;
  }
  void setMessage(ASTContext &C, llvm::StringRef S) {
    messageLength = S.size();
    this->message = new (C, 1) char [messageLength];
    std::memcpy(this->message, S.data(), messageLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Deprecated; }
  bool isLateParsed() const override { return 0; }
};

class DestructorAttr : public InheritableAttr {
int priority;


public:
  static DestructorAttr *CreateImplicit(ASTContext &Ctx, int Priority, SourceRange Loc = SourceRange()) {
    DestructorAttr *A = new (Ctx) DestructorAttr(Loc, Ctx, Priority, 0);
    A->setImplicit(true);
    return A;
  }

  DestructorAttr(SourceRange R, ASTContext &Ctx
              , int Priority
              , unsigned SI
             )
    : InheritableAttr(attr::Destructor, R, SI)
              , priority(Priority)
  {

  }

  DestructorAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Destructor, R, SI)
              , priority()
  {
  }

  DestructorAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  int getPriority() const {
    return priority;
  }

  static const int DefaultPriority = 65535;



  static bool classof(const Attr *A) { return A->getKind() == attr::Destructor; }
  bool isLateParsed() const override { return 0; }
};

class EnableIfAttr : public InheritableAttr {
Expr * cond;

unsigned messageLength;
char *message;


public:
  static EnableIfAttr *CreateImplicit(ASTContext &Ctx, Expr * Cond, llvm::StringRef Message, SourceRange Loc = SourceRange()) {
    EnableIfAttr *A = new (Ctx) EnableIfAttr(Loc, Ctx, Cond, Message, 0);
    A->setImplicit(true);
    return A;
  }

  EnableIfAttr(SourceRange R, ASTContext &Ctx
              , Expr * Cond
              , llvm::StringRef Message
              , unsigned SI
             )
    : InheritableAttr(attr::EnableIf, R, SI)
              , cond(Cond)
              , messageLength(Message.size()),message(new (Ctx, 1) char[messageLength])
  {

      std::memcpy(message, Message.data(), messageLength);
  }

  EnableIfAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getCond() const {
    return cond;
  }

  llvm::StringRef getMessage() const {
    return llvm::StringRef(message, messageLength);
  }
  unsigned getMessageLength() const {
    return messageLength;
  }
  void setMessage(ASTContext &C, llvm::StringRef S) {
    messageLength = S.size();
    this->message = new (C, 1) char [messageLength];
    std::memcpy(this->message, S.data(), messageLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::EnableIf; }
  bool isLateParsed() const override { return 0; }
};

class ExclusiveTrylockFunctionAttr : public InheritableAttr {
Expr * successValue;

  unsigned args_Size;
  Expr * *args_;


public:
  static ExclusiveTrylockFunctionAttr *CreateImplicit(ASTContext &Ctx, Expr * SuccessValue, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    ExclusiveTrylockFunctionAttr *A = new (Ctx) ExclusiveTrylockFunctionAttr(Loc, Ctx, SuccessValue, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  ExclusiveTrylockFunctionAttr(SourceRange R, ASTContext &Ctx
              , Expr * SuccessValue
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::ExclusiveTrylockFunction, R, SI)
              , successValue(SuccessValue)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {

    std::copy(Args, Args + args_Size, args_);
  }

  ExclusiveTrylockFunctionAttr(SourceRange R, ASTContext &Ctx
              , Expr * SuccessValue
              , unsigned SI
             )
    : InheritableAttr(attr::ExclusiveTrylockFunction, R, SI)
              , successValue()
              , args_Size(0), args_(nullptr)
  {

  }

  ExclusiveTrylockFunctionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getSuccessValue() const {
    return successValue;
  }

  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::ExclusiveTrylockFunction; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class FallThroughAttr : public Attr {

public:
  static FallThroughAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    FallThroughAttr *A = new (Ctx) FallThroughAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  FallThroughAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : Attr(attr::FallThrough, R, SI)
  {
  }

  FallThroughAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::FallThrough; }
  bool isLateParsed() const override { return 0; }
};

class FastCallAttr : public InheritableAttr {

public:
  static FastCallAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    FastCallAttr *A = new (Ctx) FastCallAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  FastCallAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::FastCall, R, SI)
  {
  }

  FastCallAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::FastCall; }
  bool isLateParsed() const override { return 0; }
};

class FinalAttr : public InheritableAttr {

public:
  enum Spelling {
    Keyword_final,
    Keyword_sealed
  };

  static FinalAttr *CreateImplicit(ASTContext &Ctx, Spelling S, SourceRange Loc = SourceRange()) {
    FinalAttr *A = new (Ctx) FinalAttr(Loc, Ctx, S);
    A->setImplicit(true);
    return A;
  }

  FinalAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Final, R, SI)
  {
  }

  FinalAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Keyword_final;
    case 1: return Keyword_sealed;
  }
  }
  bool isSpelledAsSealed() const { return SpellingListIndex == 1; }


  static bool classof(const Attr *A) { return A->getKind() == attr::Final; }
  bool isLateParsed() const override { return 0; }
};

class FlattenAttr : public InheritableAttr {

public:
  static FlattenAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    FlattenAttr *A = new (Ctx) FlattenAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  FlattenAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Flatten, R, SI)
  {
  }

  FlattenAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Flatten; }
  bool isLateParsed() const override { return 0; }
};

class FormatAttr : public InheritableAttr {
IdentifierInfo * type;

int formatIdx;

int firstArg;


public:
  static FormatAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * Type, int FormatIdx, int FirstArg, SourceRange Loc = SourceRange()) {
    FormatAttr *A = new (Ctx) FormatAttr(Loc, Ctx, Type, FormatIdx, FirstArg, 0);
    A->setImplicit(true);
    return A;
  }

  FormatAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * Type
              , int FormatIdx
              , int FirstArg
              , unsigned SI
             )
    : InheritableAttr(attr::Format, R, SI)
              , type(Type)
              , formatIdx(FormatIdx)
              , firstArg(FirstArg)
  {



  }

  FormatAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getType() const {
    return type;
  }

  int getFormatIdx() const {
    return formatIdx;
  }

  int getFirstArg() const {
    return firstArg;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Format; }
  bool isLateParsed() const override { return 0; }
};

class FormatArgAttr : public InheritableAttr {
int formatIdx;


public:
  static FormatArgAttr *CreateImplicit(ASTContext &Ctx, int FormatIdx, SourceRange Loc = SourceRange()) {
    FormatArgAttr *A = new (Ctx) FormatArgAttr(Loc, Ctx, FormatIdx, 0);
    A->setImplicit(true);
    return A;
  }

  FormatArgAttr(SourceRange R, ASTContext &Ctx
              , int FormatIdx
              , unsigned SI
             )
    : InheritableAttr(attr::FormatArg, R, SI)
              , formatIdx(FormatIdx)
  {

  }

  FormatArgAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  int getFormatIdx() const {
    return formatIdx;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::FormatArg; }
  bool isLateParsed() const override { return 0; }
};

class GNUInlineAttr : public InheritableAttr {

public:
  static GNUInlineAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    GNUInlineAttr *A = new (Ctx) GNUInlineAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  GNUInlineAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::GNUInline, R, SI)
  {
  }

  GNUInlineAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::GNUInline; }
  bool isLateParsed() const override { return 0; }
};

class GuardedByAttr : public InheritableAttr {
Expr * arg;


public:
  static GuardedByAttr *CreateImplicit(ASTContext &Ctx, Expr * Arg, SourceRange Loc = SourceRange()) {
    GuardedByAttr *A = new (Ctx) GuardedByAttr(Loc, Ctx, Arg, 0);
    A->setImplicit(true);
    return A;
  }

  GuardedByAttr(SourceRange R, ASTContext &Ctx
              , Expr * Arg
              , unsigned SI
             )
    : InheritableAttr(attr::GuardedBy, R, SI)
              , arg(Arg)
  {

  }

  GuardedByAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getArg() const {
    return arg;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::GuardedBy; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class GuardedVarAttr : public InheritableAttr {

public:
  static GuardedVarAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    GuardedVarAttr *A = new (Ctx) GuardedVarAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  GuardedVarAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::GuardedVar, R, SI)
  {
  }

  GuardedVarAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::GuardedVar; }
  bool isLateParsed() const override { return 0; }
};

class HotAttr : public InheritableAttr {

public:
  static HotAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    HotAttr *A = new (Ctx) HotAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  HotAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Hot, R, SI)
  {
  }

  HotAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Hot; }
  bool isLateParsed() const override { return 0; }
};

class IBActionAttr : public InheritableAttr {

public:
  static IBActionAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    IBActionAttr *A = new (Ctx) IBActionAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  IBActionAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::IBAction, R, SI)
  {
  }

  IBActionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::IBAction; }
  bool isLateParsed() const override { return 0; }
};

class IBOutletAttr : public InheritableAttr {

public:
  static IBOutletAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    IBOutletAttr *A = new (Ctx) IBOutletAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  IBOutletAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::IBOutlet, R, SI)
  {
  }

  IBOutletAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::IBOutlet; }
  bool isLateParsed() const override { return 0; }
};

class IBOutletCollectionAttr : public InheritableAttr {
TypeSourceInfo * interface;


public:
  static IBOutletCollectionAttr *CreateImplicit(ASTContext &Ctx, TypeSourceInfo * Interface, SourceRange Loc = SourceRange()) {
    IBOutletCollectionAttr *A = new (Ctx) IBOutletCollectionAttr(Loc, Ctx, Interface, 0);
    A->setImplicit(true);
    return A;
  }

  IBOutletCollectionAttr(SourceRange R, ASTContext &Ctx
              , TypeSourceInfo * Interface
              , unsigned SI
             )
    : InheritableAttr(attr::IBOutletCollection, R, SI)
              , interface(Interface)
  {

  }

  IBOutletCollectionAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::IBOutletCollection, R, SI)
              , interface()
  {
  }

  IBOutletCollectionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  QualType getInterface() const {
    return interface->getType();
  }  TypeSourceInfo * getInterfaceLoc() const {
    return interface;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::IBOutletCollection; }
  bool isLateParsed() const override { return 0; }
};

class InitPriorityAttr : public InheritableAttr {
unsigned priority;


public:
  static InitPriorityAttr *CreateImplicit(ASTContext &Ctx, unsigned Priority, SourceRange Loc = SourceRange()) {
    InitPriorityAttr *A = new (Ctx) InitPriorityAttr(Loc, Ctx, Priority, 0);
    A->setImplicit(true);
    return A;
  }

  InitPriorityAttr(SourceRange R, ASTContext &Ctx
              , unsigned Priority
              , unsigned SI
             )
    : InheritableAttr(attr::InitPriority, R, SI)
              , priority(Priority)
  {

  }

  InitPriorityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  unsigned getPriority() const {
    return priority;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::InitPriority; }
  bool isLateParsed() const override { return 0; }
};

class InitSegAttr : public Attr {
unsigned sectionLength;
char *section;


public:
  static InitSegAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Section, SourceRange Loc = SourceRange()) {
    InitSegAttr *A = new (Ctx) InitSegAttr(Loc, Ctx, Section, 0);
    A->setImplicit(true);
    return A;
  }

  InitSegAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Section
              , unsigned SI
             )
    : Attr(attr::InitSeg, R, SI)
              , sectionLength(Section.size()),section(new (Ctx, 1) char[sectionLength])
  {
      std::memcpy(section, Section.data(), sectionLength);
  }

  InitSegAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getSection() const {
    return llvm::StringRef(section, sectionLength);
  }
  unsigned getSectionLength() const {
    return sectionLength;
  }
  void setSection(ASTContext &C, llvm::StringRef S) {
    sectionLength = S.size();
    this->section = new (C, 1) char [sectionLength];
    std::memcpy(this->section, S.data(), sectionLength);
  }


  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    OS << '(' << getSection() << ')';
  }
  

  static bool classof(const Attr *A) { return A->getKind() == attr::InitSeg; }
  bool isLateParsed() const override { return 0; }
};

class IntelOclBiccAttr : public InheritableAttr {

public:
  static IntelOclBiccAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    IntelOclBiccAttr *A = new (Ctx) IntelOclBiccAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  IntelOclBiccAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::IntelOclBicc, R, SI)
  {
  }

  IntelOclBiccAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::IntelOclBicc; }
  bool isLateParsed() const override { return 0; }
};

class LockReturnedAttr : public InheritableAttr {
Expr * arg;


public:
  static LockReturnedAttr *CreateImplicit(ASTContext &Ctx, Expr * Arg, SourceRange Loc = SourceRange()) {
    LockReturnedAttr *A = new (Ctx) LockReturnedAttr(Loc, Ctx, Arg, 0);
    A->setImplicit(true);
    return A;
  }

  LockReturnedAttr(SourceRange R, ASTContext &Ctx
              , Expr * Arg
              , unsigned SI
             )
    : InheritableAttr(attr::LockReturned, R, SI)
              , arg(Arg)
  {

  }

  LockReturnedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getArg() const {
    return arg;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::LockReturned; }
  bool isLateParsed() const override { return 1; }
};

class LocksExcludedAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  static LocksExcludedAttr *CreateImplicit(ASTContext &Ctx, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    LocksExcludedAttr *A = new (Ctx) LocksExcludedAttr(Loc, Ctx, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  LocksExcludedAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::LocksExcluded, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  LocksExcludedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::LocksExcluded, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  LocksExcludedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::LocksExcluded; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class LoopHintAttr : public Attr {
public:
  enum OptionType {
    Vectorize,
    VectorizeWidth,
    Interleave,
    InterleaveCount,
    Unroll,
    UnrollCount
  };
private:
  OptionType option;

public:
  enum LoopHintState {
    Default,
    Enable,
    Disable
  };
private:
  LoopHintState state;

Expr * value;


public:
  enum Spelling {
    Pragma_clang_loop,
    Pragma_unroll,
    Pragma_nounroll
  };

  static LoopHintAttr *CreateImplicit(ASTContext &Ctx, Spelling S, OptionType Option, LoopHintState State, Expr * Value, SourceRange Loc = SourceRange()) {
    LoopHintAttr *A = new (Ctx) LoopHintAttr(Loc, Ctx, Option, State, Value, S);
    A->setImplicit(true);
    return A;
  }

  LoopHintAttr(SourceRange R, ASTContext &Ctx
              , OptionType Option
              , LoopHintState State
              , Expr * Value
              , unsigned SI
             )
    : Attr(attr::LoopHint, R, SI)
              , option(Option)
              , state(State)
              , value(Value)
  {



  }

  LoopHintAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Pragma_clang_loop;
    case 1: return Pragma_unroll;
    case 2: return Pragma_nounroll;
  }
  }
  OptionType getOption() const {
    return option;
  }

  static bool ConvertStrToOptionType(StringRef Val, OptionType &Out) {
    Optional<OptionType> R = llvm::StringSwitch<Optional<OptionType>>(Val)
      .Case("vectorize", LoopHintAttr::Vectorize)
      .Case("vectorize_width", LoopHintAttr::VectorizeWidth)
      .Case("interleave", LoopHintAttr::Interleave)
      .Case("interleave_count", LoopHintAttr::InterleaveCount)
      .Case("unroll", LoopHintAttr::Unroll)
      .Case("unroll_count", LoopHintAttr::UnrollCount)
      .Default(Optional<OptionType>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertOptionTypeToStr(OptionType Val) {
    switch(Val) {
    case LoopHintAttr::Vectorize: return "vectorize";
    case LoopHintAttr::VectorizeWidth: return "vectorize_width";
    case LoopHintAttr::Interleave: return "interleave";
    case LoopHintAttr::InterleaveCount: return "interleave_count";
    case LoopHintAttr::Unroll: return "unroll";
    case LoopHintAttr::UnrollCount: return "unroll_count";
    }
    llvm_unreachable("No enumerator with that value");
  }
  LoopHintState getState() const {
    return state;
  }

  static bool ConvertStrToLoopHintState(StringRef Val, LoopHintState &Out) {
    Optional<LoopHintState> R = llvm::StringSwitch<Optional<LoopHintState>>(Val)
      .Case("default", LoopHintAttr::Default)
      .Case("enable", LoopHintAttr::Enable)
      .Case("disable", LoopHintAttr::Disable)
      .Default(Optional<LoopHintState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertLoopHintStateToStr(LoopHintState Val) {
    switch(Val) {
    case LoopHintAttr::Default: return "default";
    case LoopHintAttr::Enable: return "enable";
    case LoopHintAttr::Disable: return "disable";
    }
    llvm_unreachable("No enumerator with that value");
  }
  Expr * getValue() const {
    return value;
  }


  static const char *getOptionName(int Option) {
    switch(Option) {
    case Vectorize: return "vectorize";
    case VectorizeWidth: return "vectorize_width";
    case Interleave: return "interleave";
    case InterleaveCount: return "interleave_count";
    case Unroll: return "unroll";
    case UnrollCount: return "unroll_count";
    }
    llvm_unreachable("Unhandled LoopHint option.");
  }

  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    // For "#pragma unroll" and "#pragma nounroll" the string "unroll" or
    // "nounroll" is already emitted as the pragma name.
    if (SpellingIndex == Pragma_nounroll) {
      OS << "\n";
      return;
    }
    else if (SpellingIndex == Pragma_unroll) {
      OS << getValueString(Policy) << "\n";
      return;
    }

    assert(SpellingIndex == Pragma_clang_loop && "Unexpected spelling");
    OS << getOptionName(option) << getValueString(Policy) << "\n";
  }

  // Return a string containing the loop hint argument including the
  // enclosing parentheses.
  std::string getValueString(const PrintingPolicy &Policy) const {
    std::string ValueName;
    llvm::raw_string_ostream OS(ValueName);
    OS << "(";
    if (option == VectorizeWidth || option == InterleaveCount ||
        option == UnrollCount)
      value->printPretty(OS, nullptr, Policy);
    else if (state == Default)
      return "";
    else if (state == Enable)
      OS << (option == Unroll ? "full" : "enable");
    else
      OS << "disable";
    OS << ")";
    return OS.str();
  }

  // Return a string suitable for identifying this attribute in diagnostics.
  std::string getDiagnosticName(const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    if (SpellingIndex == Pragma_nounroll)
      return "#pragma nounroll";
    else if (SpellingIndex == Pragma_unroll)
      return "#pragma unroll" + getValueString(Policy);

    assert(SpellingIndex == Pragma_clang_loop && "Unexpected spelling");
    return getOptionName(option) + getValueString(Policy);
  }
  

  static bool classof(const Attr *A) { return A->getKind() == attr::LoopHint; }
  bool isLateParsed() const override { return 0; }
};

class MSABIAttr : public InheritableAttr {

public:
  static MSABIAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    MSABIAttr *A = new (Ctx) MSABIAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  MSABIAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::MSABI, R, SI)
  {
  }

  MSABIAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::MSABI; }
  bool isLateParsed() const override { return 0; }
};

class MSInheritanceAttr : public InheritableAttr {
bool bestCase;


public:
  enum Spelling {
    Keyword_single_inheritance,
    Keyword_multiple_inheritance,
    Keyword_virtual_inheritance,
    Keyword_unspecified_inheritance
  };

  static MSInheritanceAttr *CreateImplicit(ASTContext &Ctx, Spelling S, bool BestCase, SourceRange Loc = SourceRange()) {
    MSInheritanceAttr *A = new (Ctx) MSInheritanceAttr(Loc, Ctx, BestCase, S);
    A->setImplicit(true);
    return A;
  }

  MSInheritanceAttr(SourceRange R, ASTContext &Ctx
              , bool BestCase
              , unsigned SI
             )
    : InheritableAttr(attr::MSInheritance, R, SI)
              , bestCase(BestCase)
  {

  }

  MSInheritanceAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::MSInheritance, R, SI)
              , bestCase()
  {
  }

  MSInheritanceAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Keyword_single_inheritance;
    case 1: return Keyword_multiple_inheritance;
    case 2: return Keyword_virtual_inheritance;
    case 3: return Keyword_unspecified_inheritance;
  }
  }
  bool getBestCase() const {
    return bestCase;
  }

  static const bool DefaultBestCase = 1;


  static bool hasVBPtrOffsetField(Spelling Inheritance) {
    return Inheritance == Keyword_unspecified_inheritance;
  }

  // Only member pointers to functions need a this adjustment, since it can be
  // combined with the field offset for data pointers.
  static bool hasNVOffsetField(bool IsMemberFunction, Spelling Inheritance) {
    return IsMemberFunction && Inheritance >= Keyword_multiple_inheritance;
  }

  static bool hasVBTableOffsetField(Spelling Inheritance) {
    return Inheritance >= Keyword_virtual_inheritance;
  }

  static bool hasOnlyOneField(bool IsMemberFunction,
                              Spelling Inheritance) {
    if (IsMemberFunction)
      return Inheritance <= Keyword_single_inheritance;
    return Inheritance <= Keyword_multiple_inheritance;
  }
  

  static bool classof(const Attr *A) { return A->getKind() == attr::MSInheritance; }
  bool isLateParsed() const override { return 0; }
};

class MSP430InterruptAttr : public InheritableAttr {
unsigned number;


public:
  static MSP430InterruptAttr *CreateImplicit(ASTContext &Ctx, unsigned Number, SourceRange Loc = SourceRange()) {
    MSP430InterruptAttr *A = new (Ctx) MSP430InterruptAttr(Loc, Ctx, Number, 0);
    A->setImplicit(true);
    return A;
  }

  MSP430InterruptAttr(SourceRange R, ASTContext &Ctx
              , unsigned Number
              , unsigned SI
             )
    : InheritableAttr(attr::MSP430Interrupt, R, SI)
              , number(Number)
  {

  }

  MSP430InterruptAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  unsigned getNumber() const {
    return number;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::MSP430Interrupt; }
  bool isLateParsed() const override { return 0; }
};

class MSVtorDispAttr : public InheritableAttr {
unsigned vdm;


public:
  static MSVtorDispAttr *CreateImplicit(ASTContext &Ctx, unsigned Vdm, SourceRange Loc = SourceRange()) {
    MSVtorDispAttr *A = new (Ctx) MSVtorDispAttr(Loc, Ctx, Vdm, 0);
    A->setImplicit(true);
    return A;
  }

  MSVtorDispAttr(SourceRange R, ASTContext &Ctx
              , unsigned Vdm
              , unsigned SI
             )
    : InheritableAttr(attr::MSVtorDisp, R, SI)
              , vdm(Vdm)
  {

  }

  MSVtorDispAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  unsigned getVdm() const {
    return vdm;
  }


  enum Mode {
    Never,
    ForVBaseOverride,
    ForVFTable
  };

  Mode getVtorDispMode() const { return Mode(vdm); }
  

  static bool classof(const Attr *A) { return A->getKind() == attr::MSVtorDisp; }
  bool isLateParsed() const override { return 0; }
};

class MallocAttr : public InheritableAttr {

public:
  static MallocAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    MallocAttr *A = new (Ctx) MallocAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  MallocAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Malloc, R, SI)
  {
  }

  MallocAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Malloc; }
  bool isLateParsed() const override { return 0; }
};

class MaxFieldAlignmentAttr : public InheritableAttr {
unsigned alignment;


public:
  static MaxFieldAlignmentAttr *CreateImplicit(ASTContext &Ctx, unsigned Alignment, SourceRange Loc = SourceRange()) {
    MaxFieldAlignmentAttr *A = new (Ctx) MaxFieldAlignmentAttr(Loc, Ctx, Alignment, 0);
    A->setImplicit(true);
    return A;
  }

  MaxFieldAlignmentAttr(SourceRange R, ASTContext &Ctx
              , unsigned Alignment
              , unsigned SI
             )
    : InheritableAttr(attr::MaxFieldAlignment, R, SI)
              , alignment(Alignment)
  {

  }

  MaxFieldAlignmentAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  unsigned getAlignment() const {
    return alignment;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::MaxFieldAlignment; }
  bool isLateParsed() const override { return 0; }
};

class MayAliasAttr : public InheritableAttr {

public:
  static MayAliasAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    MayAliasAttr *A = new (Ctx) MayAliasAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  MayAliasAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::MayAlias, R, SI)
  {
  }

  MayAliasAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::MayAlias; }
  bool isLateParsed() const override { return 0; }
};

class MinSizeAttr : public InheritableAttr {

public:
  static MinSizeAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    MinSizeAttr *A = new (Ctx) MinSizeAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  MinSizeAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::MinSize, R, SI)
  {
  }

  MinSizeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::MinSize; }
  bool isLateParsed() const override { return 0; }
};

class Mips16Attr : public InheritableAttr {

public:
  static Mips16Attr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    Mips16Attr *A = new (Ctx) Mips16Attr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  Mips16Attr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Mips16, R, SI)
  {
  }

  Mips16Attr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Mips16; }
  bool isLateParsed() const override { return 0; }
};

class ModeAttr : public Attr {
IdentifierInfo * mode;


public:
  static ModeAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * Mode, SourceRange Loc = SourceRange()) {
    ModeAttr *A = new (Ctx) ModeAttr(Loc, Ctx, Mode, 0);
    A->setImplicit(true);
    return A;
  }

  ModeAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * Mode
              , unsigned SI
             )
    : Attr(attr::Mode, R, SI)
              , mode(Mode)
  {

  }

  ModeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getMode() const {
    return mode;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Mode; }
  bool isLateParsed() const override { return 0; }
};

class MsStructAttr : public InheritableAttr {

public:
  static MsStructAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    MsStructAttr *A = new (Ctx) MsStructAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  MsStructAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::MsStruct, R, SI)
  {
  }

  MsStructAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::MsStruct; }
  bool isLateParsed() const override { return 0; }
};

class NSConsumedAttr : public InheritableParamAttr {

public:
  static NSConsumedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NSConsumedAttr *A = new (Ctx) NSConsumedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NSConsumedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableParamAttr(attr::NSConsumed, R, SI)
  {
  }

  NSConsumedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NSConsumed; }
  bool isLateParsed() const override { return 0; }
};

class NSConsumesSelfAttr : public InheritableAttr {

public:
  static NSConsumesSelfAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NSConsumesSelfAttr *A = new (Ctx) NSConsumesSelfAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NSConsumesSelfAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NSConsumesSelf, R, SI)
  {
  }

  NSConsumesSelfAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NSConsumesSelf; }
  bool isLateParsed() const override { return 0; }
};

class NSReturnsAutoreleasedAttr : public InheritableAttr {

public:
  static NSReturnsAutoreleasedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NSReturnsAutoreleasedAttr *A = new (Ctx) NSReturnsAutoreleasedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NSReturnsAutoreleasedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NSReturnsAutoreleased, R, SI)
  {
  }

  NSReturnsAutoreleasedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NSReturnsAutoreleased; }
  bool isLateParsed() const override { return 0; }
};

class NSReturnsNotRetainedAttr : public InheritableAttr {

public:
  static NSReturnsNotRetainedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NSReturnsNotRetainedAttr *A = new (Ctx) NSReturnsNotRetainedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NSReturnsNotRetainedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NSReturnsNotRetained, R, SI)
  {
  }

  NSReturnsNotRetainedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NSReturnsNotRetained; }
  bool isLateParsed() const override { return 0; }
};

class NSReturnsRetainedAttr : public InheritableAttr {

public:
  static NSReturnsRetainedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NSReturnsRetainedAttr *A = new (Ctx) NSReturnsRetainedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NSReturnsRetainedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NSReturnsRetained, R, SI)
  {
  }

  NSReturnsRetainedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NSReturnsRetained; }
  bool isLateParsed() const override { return 0; }
};

class NakedAttr : public InheritableAttr {

public:
  static NakedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NakedAttr *A = new (Ctx) NakedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NakedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Naked, R, SI)
  {
  }

  NakedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Naked; }
  bool isLateParsed() const override { return 0; }
};

class NoCommonAttr : public InheritableAttr {

public:
  static NoCommonAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoCommonAttr *A = new (Ctx) NoCommonAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoCommonAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoCommon, R, SI)
  {
  }

  NoCommonAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoCommon; }
  bool isLateParsed() const override { return 0; }
};

class NoDebugAttr : public InheritableAttr {

public:
  static NoDebugAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoDebugAttr *A = new (Ctx) NoDebugAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoDebugAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoDebug, R, SI)
  {
  }

  NoDebugAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoDebug; }
  bool isLateParsed() const override { return 0; }
};

class NoDuplicateAttr : public InheritableAttr {

public:
  static NoDuplicateAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoDuplicateAttr *A = new (Ctx) NoDuplicateAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoDuplicateAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoDuplicate, R, SI)
  {
  }

  NoDuplicateAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoDuplicate; }
  bool isLateParsed() const override { return 0; }
};

class NoInlineAttr : public InheritableAttr {

public:
  static NoInlineAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoInlineAttr *A = new (Ctx) NoInlineAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoInlineAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoInline, R, SI)
  {
  }

  NoInlineAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoInline; }
  bool isLateParsed() const override { return 0; }
};

class NoInstrumentFunctionAttr : public InheritableAttr {

public:
  static NoInstrumentFunctionAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoInstrumentFunctionAttr *A = new (Ctx) NoInstrumentFunctionAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoInstrumentFunctionAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoInstrumentFunction, R, SI)
  {
  }

  NoInstrumentFunctionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoInstrumentFunction; }
  bool isLateParsed() const override { return 0; }
};

class NoMips16Attr : public InheritableAttr {

public:
  static NoMips16Attr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoMips16Attr *A = new (Ctx) NoMips16Attr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoMips16Attr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoMips16, R, SI)
  {
  }

  NoMips16Attr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoMips16; }
  bool isLateParsed() const override { return 0; }
};

class NoReturnAttr : public InheritableAttr {

public:
  static NoReturnAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoReturnAttr *A = new (Ctx) NoReturnAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoReturnAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoReturn, R, SI)
  {
  }

  NoReturnAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoReturn; }
  bool isLateParsed() const override { return 0; }
};

class NoSanitizeAddressAttr : public InheritableAttr {

public:
  enum Spelling {
    GNU_no_address_safety_analysis,
    CXX11_gnu_no_address_safety_analysis,
    GNU_no_sanitize_address,
    CXX11_gnu_no_sanitize_address
  };

  static NoSanitizeAddressAttr *CreateImplicit(ASTContext &Ctx, Spelling S, SourceRange Loc = SourceRange()) {
    NoSanitizeAddressAttr *A = new (Ctx) NoSanitizeAddressAttr(Loc, Ctx, S);
    A->setImplicit(true);
    return A;
  }

  NoSanitizeAddressAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoSanitizeAddress, R, SI)
  {
  }

  NoSanitizeAddressAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_no_address_safety_analysis;
    case 1: return CXX11_gnu_no_address_safety_analysis;
    case 2: return GNU_no_sanitize_address;
    case 3: return CXX11_gnu_no_sanitize_address;
  }
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::NoSanitizeAddress; }
  bool isLateParsed() const override { return 0; }
};

class NoSanitizeMemoryAttr : public InheritableAttr {

public:
  static NoSanitizeMemoryAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoSanitizeMemoryAttr *A = new (Ctx) NoSanitizeMemoryAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoSanitizeMemoryAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoSanitizeMemory, R, SI)
  {
  }

  NoSanitizeMemoryAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoSanitizeMemory; }
  bool isLateParsed() const override { return 0; }
};

class NoSanitizeThreadAttr : public InheritableAttr {

public:
  static NoSanitizeThreadAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoSanitizeThreadAttr *A = new (Ctx) NoSanitizeThreadAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoSanitizeThreadAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoSanitizeThread, R, SI)
  {
  }

  NoSanitizeThreadAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoSanitizeThread; }
  bool isLateParsed() const override { return 0; }
};

class NoSplitStackAttr : public InheritableAttr {

public:
  static NoSplitStackAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoSplitStackAttr *A = new (Ctx) NoSplitStackAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoSplitStackAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoSplitStack, R, SI)
  {
  }

  NoSplitStackAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoSplitStack; }
  bool isLateParsed() const override { return 0; }
};

class NoThreadSafetyAnalysisAttr : public InheritableAttr {

public:
  static NoThreadSafetyAnalysisAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoThreadSafetyAnalysisAttr *A = new (Ctx) NoThreadSafetyAnalysisAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoThreadSafetyAnalysisAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoThreadSafetyAnalysis, R, SI)
  {
  }

  NoThreadSafetyAnalysisAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoThreadSafetyAnalysis; }
  bool isLateParsed() const override { return 0; }
};

class NoThrowAttr : public InheritableAttr {

public:
  static NoThrowAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    NoThrowAttr *A = new (Ctx) NoThrowAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  NoThrowAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NoThrow, R, SI)
  {
  }

  NoThrowAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::NoThrow; }
  bool isLateParsed() const override { return 0; }
};

class NonNullAttr : public InheritableAttr {
  unsigned args_Size;
  unsigned *args_;


public:
  static NonNullAttr *CreateImplicit(ASTContext &Ctx, unsigned *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    NonNullAttr *A = new (Ctx) NonNullAttr(Loc, Ctx, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  NonNullAttr(SourceRange R, ASTContext &Ctx
              , unsigned *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::NonNull, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) unsigned[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  NonNullAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::NonNull, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  NonNullAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  typedef unsigned* args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }


bool isNonNull(unsigned idx) const {
    if (!args_size())
      return true;
    for (const auto &V : args())
      if (V == idx)
        return true;
    return false;
  } 

  static bool classof(const Attr *A) { return A->getKind() == attr::NonNull; }
  bool isLateParsed() const override { return 0; }
  bool duplicatesAllowed() const override { return true; }

};

class OMPThreadPrivateDeclAttr : public InheritableAttr {

public:
  static OMPThreadPrivateDeclAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    OMPThreadPrivateDeclAttr *A = new (Ctx) OMPThreadPrivateDeclAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  OMPThreadPrivateDeclAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::OMPThreadPrivateDecl, R, SI)
  {
  }

  OMPThreadPrivateDeclAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::OMPThreadPrivateDecl; }
  bool isLateParsed() const override { return 0; }
};

class ObjCBridgeAttr : public InheritableAttr {
IdentifierInfo * bridgedType;


public:
  static ObjCBridgeAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * BridgedType, SourceRange Loc = SourceRange()) {
    ObjCBridgeAttr *A = new (Ctx) ObjCBridgeAttr(Loc, Ctx, BridgedType, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCBridgeAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * BridgedType
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCBridge, R, SI)
              , bridgedType(BridgedType)
  {

  }

  ObjCBridgeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getBridgedType() const {
    return bridgedType;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCBridge; }
  bool isLateParsed() const override { return 0; }
};

class ObjCBridgeMutableAttr : public InheritableAttr {
IdentifierInfo * bridgedType;


public:
  static ObjCBridgeMutableAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * BridgedType, SourceRange Loc = SourceRange()) {
    ObjCBridgeMutableAttr *A = new (Ctx) ObjCBridgeMutableAttr(Loc, Ctx, BridgedType, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCBridgeMutableAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * BridgedType
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCBridgeMutable, R, SI)
              , bridgedType(BridgedType)
  {

  }

  ObjCBridgeMutableAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getBridgedType() const {
    return bridgedType;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCBridgeMutable; }
  bool isLateParsed() const override { return 0; }
};

class ObjCBridgeRelatedAttr : public InheritableAttr {
IdentifierInfo * relatedClass;

IdentifierInfo * classMethod;

IdentifierInfo * instanceMethod;


public:
  static ObjCBridgeRelatedAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * RelatedClass, IdentifierInfo * ClassMethod, IdentifierInfo * InstanceMethod, SourceRange Loc = SourceRange()) {
    ObjCBridgeRelatedAttr *A = new (Ctx) ObjCBridgeRelatedAttr(Loc, Ctx, RelatedClass, ClassMethod, InstanceMethod, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCBridgeRelatedAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * RelatedClass
              , IdentifierInfo * ClassMethod
              , IdentifierInfo * InstanceMethod
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCBridgeRelated, R, SI)
              , relatedClass(RelatedClass)
              , classMethod(ClassMethod)
              , instanceMethod(InstanceMethod)
  {



  }

  ObjCBridgeRelatedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getRelatedClass() const {
    return relatedClass;
  }

  IdentifierInfo * getClassMethod() const {
    return classMethod;
  }

  IdentifierInfo * getInstanceMethod() const {
    return instanceMethod;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCBridgeRelated; }
  bool isLateParsed() const override { return 0; }
};

class ObjCDesignatedInitializerAttr : public Attr {

public:
  static ObjCDesignatedInitializerAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCDesignatedInitializerAttr *A = new (Ctx) ObjCDesignatedInitializerAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCDesignatedInitializerAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : Attr(attr::ObjCDesignatedInitializer, R, SI)
  {
  }

  ObjCDesignatedInitializerAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCDesignatedInitializer; }
  bool isLateParsed() const override { return 0; }
};

class ObjCExceptionAttr : public InheritableAttr {

public:
  static ObjCExceptionAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCExceptionAttr *A = new (Ctx) ObjCExceptionAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCExceptionAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCException, R, SI)
  {
  }

  ObjCExceptionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCException; }
  bool isLateParsed() const override { return 0; }
};

class ObjCExplicitProtocolImplAttr : public InheritableAttr {

public:
  static ObjCExplicitProtocolImplAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCExplicitProtocolImplAttr *A = new (Ctx) ObjCExplicitProtocolImplAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCExplicitProtocolImplAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCExplicitProtocolImpl, R, SI)
  {
  }

  ObjCExplicitProtocolImplAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCExplicitProtocolImpl; }
  bool isLateParsed() const override { return 0; }
};

class ObjCMethodFamilyAttr : public InheritableAttr {
public:
  enum FamilyKind {
    OMF_None,
    OMF_alloc,
    OMF_copy,
    OMF_init,
    OMF_mutableCopy,
    OMF_new
  };
private:
  FamilyKind family;


public:
  static ObjCMethodFamilyAttr *CreateImplicit(ASTContext &Ctx, FamilyKind Family, SourceRange Loc = SourceRange()) {
    ObjCMethodFamilyAttr *A = new (Ctx) ObjCMethodFamilyAttr(Loc, Ctx, Family, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCMethodFamilyAttr(SourceRange R, ASTContext &Ctx
              , FamilyKind Family
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCMethodFamily, R, SI)
              , family(Family)
  {

  }

  ObjCMethodFamilyAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  FamilyKind getFamily() const {
    return family;
  }

  static bool ConvertStrToFamilyKind(StringRef Val, FamilyKind &Out) {
    Optional<FamilyKind> R = llvm::StringSwitch<Optional<FamilyKind>>(Val)
      .Case("none", ObjCMethodFamilyAttr::OMF_None)
      .Case("alloc", ObjCMethodFamilyAttr::OMF_alloc)
      .Case("copy", ObjCMethodFamilyAttr::OMF_copy)
      .Case("init", ObjCMethodFamilyAttr::OMF_init)
      .Case("mutableCopy", ObjCMethodFamilyAttr::OMF_mutableCopy)
      .Case("new", ObjCMethodFamilyAttr::OMF_new)
      .Default(Optional<FamilyKind>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertFamilyKindToStr(FamilyKind Val) {
    switch(Val) {
    case ObjCMethodFamilyAttr::OMF_None: return "none";
    case ObjCMethodFamilyAttr::OMF_alloc: return "alloc";
    case ObjCMethodFamilyAttr::OMF_copy: return "copy";
    case ObjCMethodFamilyAttr::OMF_init: return "init";
    case ObjCMethodFamilyAttr::OMF_mutableCopy: return "mutableCopy";
    case ObjCMethodFamilyAttr::OMF_new: return "new";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCMethodFamily; }
  bool isLateParsed() const override { return 0; }
};

class ObjCNSObjectAttr : public InheritableAttr {

public:
  static ObjCNSObjectAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCNSObjectAttr *A = new (Ctx) ObjCNSObjectAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCNSObjectAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCNSObject, R, SI)
  {
  }

  ObjCNSObjectAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCNSObject; }
  bool isLateParsed() const override { return 0; }
};

class ObjCPreciseLifetimeAttr : public InheritableAttr {

public:
  static ObjCPreciseLifetimeAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCPreciseLifetimeAttr *A = new (Ctx) ObjCPreciseLifetimeAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCPreciseLifetimeAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCPreciseLifetime, R, SI)
  {
  }

  ObjCPreciseLifetimeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCPreciseLifetime; }
  bool isLateParsed() const override { return 0; }
};

class ObjCRequiresPropertyDefsAttr : public InheritableAttr {

public:
  static ObjCRequiresPropertyDefsAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCRequiresPropertyDefsAttr *A = new (Ctx) ObjCRequiresPropertyDefsAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCRequiresPropertyDefsAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCRequiresPropertyDefs, R, SI)
  {
  }

  ObjCRequiresPropertyDefsAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCRequiresPropertyDefs; }
  bool isLateParsed() const override { return 0; }
};

class ObjCRequiresSuperAttr : public InheritableAttr {

public:
  static ObjCRequiresSuperAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCRequiresSuperAttr *A = new (Ctx) ObjCRequiresSuperAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCRequiresSuperAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCRequiresSuper, R, SI)
  {
  }

  ObjCRequiresSuperAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCRequiresSuper; }
  bool isLateParsed() const override { return 0; }
};

class ObjCReturnsInnerPointerAttr : public InheritableAttr {

public:
  static ObjCReturnsInnerPointerAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCReturnsInnerPointerAttr *A = new (Ctx) ObjCReturnsInnerPointerAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCReturnsInnerPointerAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCReturnsInnerPointer, R, SI)
  {
  }

  ObjCReturnsInnerPointerAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCReturnsInnerPointer; }
  bool isLateParsed() const override { return 0; }
};

class ObjCRootClassAttr : public InheritableAttr {

public:
  static ObjCRootClassAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ObjCRootClassAttr *A = new (Ctx) ObjCRootClassAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCRootClassAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ObjCRootClass, R, SI)
  {
  }

  ObjCRootClassAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCRootClass; }
  bool isLateParsed() const override { return 0; }
};

class ObjCRuntimeNameAttr : public Attr {
unsigned metadataNameLength;
char *metadataName;


public:
  static ObjCRuntimeNameAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef MetadataName, SourceRange Loc = SourceRange()) {
    ObjCRuntimeNameAttr *A = new (Ctx) ObjCRuntimeNameAttr(Loc, Ctx, MetadataName, 0);
    A->setImplicit(true);
    return A;
  }

  ObjCRuntimeNameAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef MetadataName
              , unsigned SI
             )
    : Attr(attr::ObjCRuntimeName, R, SI)
              , metadataNameLength(MetadataName.size()),metadataName(new (Ctx, 1) char[metadataNameLength])
  {
      std::memcpy(metadataName, MetadataName.data(), metadataNameLength);
  }

  ObjCRuntimeNameAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getMetadataName() const {
    return llvm::StringRef(metadataName, metadataNameLength);
  }
  unsigned getMetadataNameLength() const {
    return metadataNameLength;
  }
  void setMetadataName(ASTContext &C, llvm::StringRef S) {
    metadataNameLength = S.size();
    this->metadataName = new (C, 1) char [metadataNameLength];
    std::memcpy(this->metadataName, S.data(), metadataNameLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::ObjCRuntimeName; }
  bool isLateParsed() const override { return 0; }
};

class OpenCLImageAccessAttr : public Attr {

public:
  enum Spelling {
    Keyword_read_only,
    Keyword_write_only,
    Keyword_read_write
  };

  static OpenCLImageAccessAttr *CreateImplicit(ASTContext &Ctx, Spelling S, SourceRange Loc = SourceRange()) {
    OpenCLImageAccessAttr *A = new (Ctx) OpenCLImageAccessAttr(Loc, Ctx, S);
    A->setImplicit(true);
    return A;
  }

  OpenCLImageAccessAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : Attr(attr::OpenCLImageAccess, R, SI)
  {
  }

  OpenCLImageAccessAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return Keyword_read_only;
    case 1: return Keyword_read_only;
    case 2: return Keyword_write_only;
    case 3: return Keyword_write_only;
    case 4: return Keyword_read_write;
    case 5: return Keyword_read_write;
  }
  }
  bool isReadOnly() const { return SpellingListIndex == 0 ||
    SpellingListIndex == 1; }
  bool isReadWrite() const { return SpellingListIndex == 4 ||
    SpellingListIndex == 5; }
  bool isWriteOnly() const { return SpellingListIndex == 2 ||
    SpellingListIndex == 3; }


  static bool classof(const Attr *A) { return A->getKind() == attr::OpenCLImageAccess; }
  bool isLateParsed() const override { return 0; }
};

class OpenCLKernelAttr : public InheritableAttr {

public:
  static OpenCLKernelAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    OpenCLKernelAttr *A = new (Ctx) OpenCLKernelAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  OpenCLKernelAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::OpenCLKernel, R, SI)
  {
  }

  OpenCLKernelAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::OpenCLKernel; }
  bool isLateParsed() const override { return 0; }
};

class OptimizeNoneAttr : public InheritableAttr {

public:
  static OptimizeNoneAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    OptimizeNoneAttr *A = new (Ctx) OptimizeNoneAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  OptimizeNoneAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::OptimizeNone, R, SI)
  {
  }

  OptimizeNoneAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::OptimizeNone; }
  bool isLateParsed() const override { return 0; }
};

class OverloadableAttr : public Attr {

public:
  static OverloadableAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    OverloadableAttr *A = new (Ctx) OverloadableAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  OverloadableAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : Attr(attr::Overloadable, R, SI)
  {
  }

  OverloadableAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Overloadable; }
  bool isLateParsed() const override { return 0; }
};

class OverrideAttr : public InheritableAttr {

public:
  static OverrideAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    OverrideAttr *A = new (Ctx) OverrideAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  OverrideAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Override, R, SI)
  {
  }

  OverrideAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Override; }
  bool isLateParsed() const override { return 0; }
};

class OwnershipAttr : public InheritableAttr {
IdentifierInfo * module;

  unsigned args_Size;
  unsigned *args_;


public:
  enum Spelling {
    GNU_ownership_holds,
    GNU_ownership_returns,
    GNU_ownership_takes
  };

  static OwnershipAttr *CreateImplicit(ASTContext &Ctx, Spelling S, IdentifierInfo * Module, unsigned *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    OwnershipAttr *A = new (Ctx) OwnershipAttr(Loc, Ctx, Module, Args, ArgsSize, S);
    A->setImplicit(true);
    return A;
  }

  OwnershipAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * Module
              , unsigned *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::Ownership, R, SI)
              , module(Module)
              , args_Size(ArgsSize), args_(new (Ctx, 16) unsigned[args_Size])
  {

    std::copy(Args, Args + args_Size, args_);
  }

  OwnershipAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * Module
              , unsigned SI
             )
    : InheritableAttr(attr::Ownership, R, SI)
              , module()
              , args_Size(0), args_(nullptr)
  {

  }

  OwnershipAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_ownership_holds;
    case 1: return GNU_ownership_returns;
    case 2: return GNU_ownership_takes;
  }
  }
  bool isHolds() const { return SpellingListIndex == 0; }
  bool isReturns() const { return SpellingListIndex == 1; }
  bool isTakes() const { return SpellingListIndex == 2; }
  IdentifierInfo * getModule() const {
    return module;
  }

  typedef unsigned* args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }



    enum OwnershipKind { Holds, Returns, Takes };
    OwnershipKind getOwnKind() const {
      return isHolds() ? Holds :
             isTakes() ? Takes :
             Returns;
    }
  

  static bool classof(const Attr *A) { return A->getKind() == attr::Ownership; }
  bool isLateParsed() const override { return 0; }
};

class PackedAttr : public InheritableAttr {

public:
  static PackedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    PackedAttr *A = new (Ctx) PackedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  PackedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Packed, R, SI)
  {
  }

  PackedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Packed; }
  bool isLateParsed() const override { return 0; }
};

class ParamTypestateAttr : public InheritableAttr {
public:
  enum ConsumedState {
    Unknown,
    Consumed,
    Unconsumed
  };
private:
  ConsumedState paramState;


public:
  static ParamTypestateAttr *CreateImplicit(ASTContext &Ctx, ConsumedState ParamState, SourceRange Loc = SourceRange()) {
    ParamTypestateAttr *A = new (Ctx) ParamTypestateAttr(Loc, Ctx, ParamState, 0);
    A->setImplicit(true);
    return A;
  }

  ParamTypestateAttr(SourceRange R, ASTContext &Ctx
              , ConsumedState ParamState
              , unsigned SI
             )
    : InheritableAttr(attr::ParamTypestate, R, SI)
              , paramState(ParamState)
  {

  }

  ParamTypestateAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  ConsumedState getParamState() const {
    return paramState;
  }

  static bool ConvertStrToConsumedState(StringRef Val, ConsumedState &Out) {
    Optional<ConsumedState> R = llvm::StringSwitch<Optional<ConsumedState>>(Val)
      .Case("unknown", ParamTypestateAttr::Unknown)
      .Case("consumed", ParamTypestateAttr::Consumed)
      .Case("unconsumed", ParamTypestateAttr::Unconsumed)
      .Default(Optional<ConsumedState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertConsumedStateToStr(ConsumedState Val) {
    switch(Val) {
    case ParamTypestateAttr::Unknown: return "unknown";
    case ParamTypestateAttr::Consumed: return "consumed";
    case ParamTypestateAttr::Unconsumed: return "unconsumed";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::ParamTypestate; }
  bool isLateParsed() const override { return 0; }
};

class PascalAttr : public InheritableAttr {

public:
  static PascalAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    PascalAttr *A = new (Ctx) PascalAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  PascalAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Pascal, R, SI)
  {
  }

  PascalAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Pascal; }
  bool isLateParsed() const override { return 0; }
};

class PcsAttr : public InheritableAttr {
public:
  enum PCSType {
    AAPCS,
    AAPCS_VFP
  };
private:
  PCSType pCS;


public:
  static PcsAttr *CreateImplicit(ASTContext &Ctx, PCSType PCS, SourceRange Loc = SourceRange()) {
    PcsAttr *A = new (Ctx) PcsAttr(Loc, Ctx, PCS, 0);
    A->setImplicit(true);
    return A;
  }

  PcsAttr(SourceRange R, ASTContext &Ctx
              , PCSType PCS
              , unsigned SI
             )
    : InheritableAttr(attr::Pcs, R, SI)
              , pCS(PCS)
  {

  }

  PcsAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  PCSType getPCS() const {
    return pCS;
  }

  static bool ConvertStrToPCSType(StringRef Val, PCSType &Out) {
    Optional<PCSType> R = llvm::StringSwitch<Optional<PCSType>>(Val)
      .Case("aapcs", PcsAttr::AAPCS)
      .Case("aapcs-vfp", PcsAttr::AAPCS_VFP)
      .Default(Optional<PCSType>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertPCSTypeToStr(PCSType Val) {
    switch(Val) {
    case PcsAttr::AAPCS: return "aapcs";
    case PcsAttr::AAPCS_VFP: return "aapcs-vfp";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::Pcs; }
  bool isLateParsed() const override { return 0; }
};

class PnaclCallAttr : public InheritableAttr {

public:
  static PnaclCallAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    PnaclCallAttr *A = new (Ctx) PnaclCallAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  PnaclCallAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::PnaclCall, R, SI)
  {
  }

  PnaclCallAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::PnaclCall; }
  bool isLateParsed() const override { return 0; }
};

class PtGuardedByAttr : public InheritableAttr {
Expr * arg;


public:
  static PtGuardedByAttr *CreateImplicit(ASTContext &Ctx, Expr * Arg, SourceRange Loc = SourceRange()) {
    PtGuardedByAttr *A = new (Ctx) PtGuardedByAttr(Loc, Ctx, Arg, 0);
    A->setImplicit(true);
    return A;
  }

  PtGuardedByAttr(SourceRange R, ASTContext &Ctx
              , Expr * Arg
              , unsigned SI
             )
    : InheritableAttr(attr::PtGuardedBy, R, SI)
              , arg(Arg)
  {

  }

  PtGuardedByAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getArg() const {
    return arg;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::PtGuardedBy; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class PtGuardedVarAttr : public InheritableAttr {

public:
  static PtGuardedVarAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    PtGuardedVarAttr *A = new (Ctx) PtGuardedVarAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  PtGuardedVarAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::PtGuardedVar, R, SI)
  {
  }

  PtGuardedVarAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::PtGuardedVar; }
  bool isLateParsed() const override { return 0; }
};

class PureAttr : public InheritableAttr {

public:
  static PureAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    PureAttr *A = new (Ctx) PureAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  PureAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Pure, R, SI)
  {
  }

  PureAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Pure; }
  bool isLateParsed() const override { return 0; }
};

class ReleaseCapabilityAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  enum Spelling {
    GNU_release_capability,
    CXX11_clang_release_capability,
    GNU_release_shared_capability,
    CXX11_clang_release_shared_capability,
    GNU_release_generic_capability,
    CXX11_clang_release_generic_capability,
    GNU_unlock_function
  };

  static ReleaseCapabilityAttr *CreateImplicit(ASTContext &Ctx, Spelling S, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    ReleaseCapabilityAttr *A = new (Ctx) ReleaseCapabilityAttr(Loc, Ctx, Args, ArgsSize, S);
    A->setImplicit(true);
    return A;
  }

  ReleaseCapabilityAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::ReleaseCapability, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  ReleaseCapabilityAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ReleaseCapability, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  ReleaseCapabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
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
  bool isShared() const { return SpellingListIndex == 2 ||
    SpellingListIndex == 3; }
  bool isGeneric() const { return SpellingListIndex == 4 ||
    SpellingListIndex == 5 ||
    SpellingListIndex == 6; }
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::ReleaseCapability; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class ReqdWorkGroupSizeAttr : public InheritableAttr {
unsigned xDim;

unsigned yDim;

unsigned zDim;


public:
  static ReqdWorkGroupSizeAttr *CreateImplicit(ASTContext &Ctx, unsigned XDim, unsigned YDim, unsigned ZDim, SourceRange Loc = SourceRange()) {
    ReqdWorkGroupSizeAttr *A = new (Ctx) ReqdWorkGroupSizeAttr(Loc, Ctx, XDim, YDim, ZDim, 0);
    A->setImplicit(true);
    return A;
  }

  ReqdWorkGroupSizeAttr(SourceRange R, ASTContext &Ctx
              , unsigned XDim
              , unsigned YDim
              , unsigned ZDim
              , unsigned SI
             )
    : InheritableAttr(attr::ReqdWorkGroupSize, R, SI)
              , xDim(XDim)
              , yDim(YDim)
              , zDim(ZDim)
  {



  }

  ReqdWorkGroupSizeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  unsigned getXDim() const {
    return xDim;
  }

  unsigned getYDim() const {
    return yDim;
  }

  unsigned getZDim() const {
    return zDim;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::ReqdWorkGroupSize; }
  bool isLateParsed() const override { return 0; }
};

class RequiresCapabilityAttr : public InheritableAttr {
  unsigned args_Size;
  Expr * *args_;


public:
  enum Spelling {
    GNU_requires_capability,
    CXX11_clang_requires_capability,
    GNU_exclusive_locks_required,
    GNU_requires_shared_capability,
    CXX11_clang_requires_shared_capability,
    GNU_shared_locks_required
  };

  static RequiresCapabilityAttr *CreateImplicit(ASTContext &Ctx, Spelling S, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    RequiresCapabilityAttr *A = new (Ctx) RequiresCapabilityAttr(Loc, Ctx, Args, ArgsSize, S);
    A->setImplicit(true);
    return A;
  }

  RequiresCapabilityAttr(SourceRange R, ASTContext &Ctx
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::RequiresCapability, R, SI)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {
    std::copy(Args, Args + args_Size, args_);
  }

  RequiresCapabilityAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::RequiresCapability, R, SI)
              , args_Size(0), args_(nullptr)
  {
  }

  RequiresCapabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_requires_capability;
    case 1: return CXX11_clang_requires_capability;
    case 2: return GNU_exclusive_locks_required;
    case 3: return GNU_requires_shared_capability;
    case 4: return CXX11_clang_requires_shared_capability;
    case 5: return GNU_shared_locks_required;
  }
  }
  bool isShared() const { return SpellingListIndex == 3 ||
    SpellingListIndex == 5 ||
    SpellingListIndex == 4; }
  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::RequiresCapability; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class ReturnTypestateAttr : public InheritableAttr {
public:
  enum ConsumedState {
    Unknown,
    Consumed,
    Unconsumed
  };
private:
  ConsumedState state;


public:
  static ReturnTypestateAttr *CreateImplicit(ASTContext &Ctx, ConsumedState State, SourceRange Loc = SourceRange()) {
    ReturnTypestateAttr *A = new (Ctx) ReturnTypestateAttr(Loc, Ctx, State, 0);
    A->setImplicit(true);
    return A;
  }

  ReturnTypestateAttr(SourceRange R, ASTContext &Ctx
              , ConsumedState State
              , unsigned SI
             )
    : InheritableAttr(attr::ReturnTypestate, R, SI)
              , state(State)
  {

  }

  ReturnTypestateAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  ConsumedState getState() const {
    return state;
  }

  static bool ConvertStrToConsumedState(StringRef Val, ConsumedState &Out) {
    Optional<ConsumedState> R = llvm::StringSwitch<Optional<ConsumedState>>(Val)
      .Case("unknown", ReturnTypestateAttr::Unknown)
      .Case("consumed", ReturnTypestateAttr::Consumed)
      .Case("unconsumed", ReturnTypestateAttr::Unconsumed)
      .Default(Optional<ConsumedState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertConsumedStateToStr(ConsumedState Val) {
    switch(Val) {
    case ReturnTypestateAttr::Unknown: return "unknown";
    case ReturnTypestateAttr::Consumed: return "consumed";
    case ReturnTypestateAttr::Unconsumed: return "unconsumed";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::ReturnTypestate; }
  bool isLateParsed() const override { return 0; }
};

class ReturnsNonNullAttr : public InheritableAttr {

public:
  static ReturnsNonNullAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ReturnsNonNullAttr *A = new (Ctx) ReturnsNonNullAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ReturnsNonNullAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ReturnsNonNull, R, SI)
  {
  }

  ReturnsNonNullAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ReturnsNonNull; }
  bool isLateParsed() const override { return 0; }
};

class ReturnsTwiceAttr : public InheritableAttr {

public:
  static ReturnsTwiceAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ReturnsTwiceAttr *A = new (Ctx) ReturnsTwiceAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ReturnsTwiceAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ReturnsTwice, R, SI)
  {
  }

  ReturnsTwiceAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ReturnsTwice; }
  bool isLateParsed() const override { return 0; }
};

class ScopedLockableAttr : public InheritableAttr {

public:
  static ScopedLockableAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ScopedLockableAttr *A = new (Ctx) ScopedLockableAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ScopedLockableAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ScopedLockable, R, SI)
  {
  }

  ScopedLockableAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ScopedLockable; }
  bool isLateParsed() const override { return 0; }
};

class SectionAttr : public InheritableAttr {
unsigned nameLength;
char *name;


public:
  enum Spelling {
    GNU_section,
    CXX11_gnu_section,
    Declspec_allocate
  };

  static SectionAttr *CreateImplicit(ASTContext &Ctx, Spelling S, llvm::StringRef Name, SourceRange Loc = SourceRange()) {
    SectionAttr *A = new (Ctx) SectionAttr(Loc, Ctx, Name, S);
    A->setImplicit(true);
    return A;
  }

  SectionAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Name
              , unsigned SI
             )
    : InheritableAttr(attr::Section, R, SI)
              , nameLength(Name.size()),name(new (Ctx, 1) char[nameLength])
  {
      std::memcpy(name, Name.data(), nameLength);
  }

  SectionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_section;
    case 1: return CXX11_gnu_section;
    case 2: return Declspec_allocate;
  }
  }
  llvm::StringRef getName() const {
    return llvm::StringRef(name, nameLength);
  }
  unsigned getNameLength() const {
    return nameLength;
  }
  void setName(ASTContext &C, llvm::StringRef S) {
    nameLength = S.size();
    this->name = new (C, 1) char [nameLength];
    std::memcpy(this->name, S.data(), nameLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Section; }
  bool isLateParsed() const override { return 0; }
};

class SelectAnyAttr : public InheritableAttr {

public:
  static SelectAnyAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    SelectAnyAttr *A = new (Ctx) SelectAnyAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  SelectAnyAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::SelectAny, R, SI)
  {
  }

  SelectAnyAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::SelectAny; }
  bool isLateParsed() const override { return 0; }
};

class SentinelAttr : public InheritableAttr {
int sentinel;

int nullPos;


public:
  static SentinelAttr *CreateImplicit(ASTContext &Ctx, int Sentinel, int NullPos, SourceRange Loc = SourceRange()) {
    SentinelAttr *A = new (Ctx) SentinelAttr(Loc, Ctx, Sentinel, NullPos, 0);
    A->setImplicit(true);
    return A;
  }

  SentinelAttr(SourceRange R, ASTContext &Ctx
              , int Sentinel
              , int NullPos
              , unsigned SI
             )
    : InheritableAttr(attr::Sentinel, R, SI)
              , sentinel(Sentinel)
              , nullPos(NullPos)
  {


  }

  SentinelAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Sentinel, R, SI)
              , sentinel()
              , nullPos()
  {
  }

  SentinelAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  int getSentinel() const {
    return sentinel;
  }

  static const int DefaultSentinel = 0;

  int getNullPos() const {
    return nullPos;
  }

  static const int DefaultNullPos = 0;



  static bool classof(const Attr *A) { return A->getKind() == attr::Sentinel; }
  bool isLateParsed() const override { return 0; }
};

class SetTypestateAttr : public InheritableAttr {
public:
  enum ConsumedState {
    Unknown,
    Consumed,
    Unconsumed
  };
private:
  ConsumedState newState;


public:
  static SetTypestateAttr *CreateImplicit(ASTContext &Ctx, ConsumedState NewState, SourceRange Loc = SourceRange()) {
    SetTypestateAttr *A = new (Ctx) SetTypestateAttr(Loc, Ctx, NewState, 0);
    A->setImplicit(true);
    return A;
  }

  SetTypestateAttr(SourceRange R, ASTContext &Ctx
              , ConsumedState NewState
              , unsigned SI
             )
    : InheritableAttr(attr::SetTypestate, R, SI)
              , newState(NewState)
  {

  }

  SetTypestateAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  ConsumedState getNewState() const {
    return newState;
  }

  static bool ConvertStrToConsumedState(StringRef Val, ConsumedState &Out) {
    Optional<ConsumedState> R = llvm::StringSwitch<Optional<ConsumedState>>(Val)
      .Case("unknown", SetTypestateAttr::Unknown)
      .Case("consumed", SetTypestateAttr::Consumed)
      .Case("unconsumed", SetTypestateAttr::Unconsumed)
      .Default(Optional<ConsumedState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertConsumedStateToStr(ConsumedState Val) {
    switch(Val) {
    case SetTypestateAttr::Unknown: return "unknown";
    case SetTypestateAttr::Consumed: return "consumed";
    case SetTypestateAttr::Unconsumed: return "unconsumed";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::SetTypestate; }
  bool isLateParsed() const override { return 0; }
};

class SharedTrylockFunctionAttr : public InheritableAttr {
Expr * successValue;

  unsigned args_Size;
  Expr * *args_;


public:
  static SharedTrylockFunctionAttr *CreateImplicit(ASTContext &Ctx, Expr * SuccessValue, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    SharedTrylockFunctionAttr *A = new (Ctx) SharedTrylockFunctionAttr(Loc, Ctx, SuccessValue, Args, ArgsSize, 0);
    A->setImplicit(true);
    return A;
  }

  SharedTrylockFunctionAttr(SourceRange R, ASTContext &Ctx
              , Expr * SuccessValue
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::SharedTrylockFunction, R, SI)
              , successValue(SuccessValue)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {

    std::copy(Args, Args + args_Size, args_);
  }

  SharedTrylockFunctionAttr(SourceRange R, ASTContext &Ctx
              , Expr * SuccessValue
              , unsigned SI
             )
    : InheritableAttr(attr::SharedTrylockFunction, R, SI)
              , successValue()
              , args_Size(0), args_(nullptr)
  {

  }

  SharedTrylockFunctionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Expr * getSuccessValue() const {
    return successValue;
  }

  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::SharedTrylockFunction; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class StdCallAttr : public InheritableAttr {

public:
  static StdCallAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    StdCallAttr *A = new (Ctx) StdCallAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  StdCallAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::StdCall, R, SI)
  {
  }

  StdCallAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::StdCall; }
  bool isLateParsed() const override { return 0; }
};

class SysVABIAttr : public InheritableAttr {

public:
  static SysVABIAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    SysVABIAttr *A = new (Ctx) SysVABIAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  SysVABIAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::SysVABI, R, SI)
  {
  }

  SysVABIAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::SysVABI; }
  bool isLateParsed() const override { return 0; }
};

class TLSModelAttr : public InheritableAttr {
unsigned modelLength;
char *model;


public:
  static TLSModelAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Model, SourceRange Loc = SourceRange()) {
    TLSModelAttr *A = new (Ctx) TLSModelAttr(Loc, Ctx, Model, 0);
    A->setImplicit(true);
    return A;
  }

  TLSModelAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Model
              , unsigned SI
             )
    : InheritableAttr(attr::TLSModel, R, SI)
              , modelLength(Model.size()),model(new (Ctx, 1) char[modelLength])
  {
      std::memcpy(model, Model.data(), modelLength);
  }

  TLSModelAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getModel() const {
    return llvm::StringRef(model, modelLength);
  }
  unsigned getModelLength() const {
    return modelLength;
  }
  void setModel(ASTContext &C, llvm::StringRef S) {
    modelLength = S.size();
    this->model = new (C, 1) char [modelLength];
    std::memcpy(this->model, S.data(), modelLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::TLSModel; }
  bool isLateParsed() const override { return 0; }
};

class TestTypestateAttr : public InheritableAttr {
public:
  enum ConsumedState {
    Consumed,
    Unconsumed
  };
private:
  ConsumedState testState;


public:
  static TestTypestateAttr *CreateImplicit(ASTContext &Ctx, ConsumedState TestState, SourceRange Loc = SourceRange()) {
    TestTypestateAttr *A = new (Ctx) TestTypestateAttr(Loc, Ctx, TestState, 0);
    A->setImplicit(true);
    return A;
  }

  TestTypestateAttr(SourceRange R, ASTContext &Ctx
              , ConsumedState TestState
              , unsigned SI
             )
    : InheritableAttr(attr::TestTypestate, R, SI)
              , testState(TestState)
  {

  }

  TestTypestateAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  ConsumedState getTestState() const {
    return testState;
  }

  static bool ConvertStrToConsumedState(StringRef Val, ConsumedState &Out) {
    Optional<ConsumedState> R = llvm::StringSwitch<Optional<ConsumedState>>(Val)
      .Case("consumed", TestTypestateAttr::Consumed)
      .Case("unconsumed", TestTypestateAttr::Unconsumed)
      .Default(Optional<ConsumedState>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertConsumedStateToStr(ConsumedState Val) {
    switch(Val) {
    case TestTypestateAttr::Consumed: return "consumed";
    case TestTypestateAttr::Unconsumed: return "unconsumed";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::TestTypestate; }
  bool isLateParsed() const override { return 0; }
};

class ThisCallAttr : public InheritableAttr {

public:
  static ThisCallAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ThisCallAttr *A = new (Ctx) ThisCallAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ThisCallAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::ThisCall, R, SI)
  {
  }

  ThisCallAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::ThisCall; }
  bool isLateParsed() const override { return 0; }
};

class ThreadAttr : public Attr {

public:
  static ThreadAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    ThreadAttr *A = new (Ctx) ThreadAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  ThreadAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : Attr(attr::Thread, R, SI)
  {
  }

  ThreadAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Thread; }
  bool isLateParsed() const override { return 0; }
};

class TransparentUnionAttr : public InheritableAttr {

public:
  static TransparentUnionAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    TransparentUnionAttr *A = new (Ctx) TransparentUnionAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  TransparentUnionAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::TransparentUnion, R, SI)
  {
  }

  TransparentUnionAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::TransparentUnion; }
  bool isLateParsed() const override { return 0; }
};

class TryAcquireCapabilityAttr : public InheritableAttr {
Expr * successValue;

  unsigned args_Size;
  Expr * *args_;


public:
  enum Spelling {
    GNU_try_acquire_capability,
    CXX11_clang_try_acquire_capability,
    GNU_try_acquire_shared_capability,
    CXX11_clang_try_acquire_shared_capability
  };

  static TryAcquireCapabilityAttr *CreateImplicit(ASTContext &Ctx, Spelling S, Expr * SuccessValue, Expr * *Args, unsigned ArgsSize, SourceRange Loc = SourceRange()) {
    TryAcquireCapabilityAttr *A = new (Ctx) TryAcquireCapabilityAttr(Loc, Ctx, SuccessValue, Args, ArgsSize, S);
    A->setImplicit(true);
    return A;
  }

  TryAcquireCapabilityAttr(SourceRange R, ASTContext &Ctx
              , Expr * SuccessValue
              , Expr * *Args, unsigned ArgsSize
              , unsigned SI
             )
    : InheritableAttr(attr::TryAcquireCapability, R, SI)
              , successValue(SuccessValue)
              , args_Size(ArgsSize), args_(new (Ctx, 16) Expr *[args_Size])
  {

    std::copy(Args, Args + args_Size, args_);
  }

  TryAcquireCapabilityAttr(SourceRange R, ASTContext &Ctx
              , Expr * SuccessValue
              , unsigned SI
             )
    : InheritableAttr(attr::TryAcquireCapability, R, SI)
              , successValue()
              , args_Size(0), args_(nullptr)
  {

  }

  TryAcquireCapabilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  Spelling getSemanticSpelling() const {
  switch (SpellingListIndex) {
    default: llvm_unreachable("Unknown spelling list index");
    case 0: return GNU_try_acquire_capability;
    case 1: return CXX11_clang_try_acquire_capability;
    case 2: return GNU_try_acquire_shared_capability;
    case 3: return CXX11_clang_try_acquire_shared_capability;
  }
  }
  bool isShared() const { return SpellingListIndex == 2 ||
    SpellingListIndex == 3; }
  Expr * getSuccessValue() const {
    return successValue;
  }

  typedef Expr ** args_iterator;
  args_iterator args_begin() const { return args_; }
  args_iterator args_end() const { return args_ + args_Size; }
  unsigned args_size() const { return args_Size; }
  llvm::iterator_range<args_iterator> args() const { return llvm::make_range(args_begin(), args_end()); }




  static bool classof(const Attr *A) { return A->getKind() == attr::TryAcquireCapability; }
  bool isLateParsed() const override { return 1; }
  bool duplicatesAllowed() const override { return true; }

};

class TypeTagForDatatypeAttr : public InheritableAttr {
IdentifierInfo * argumentKind;

TypeSourceInfo * matchingCType;

bool layoutCompatible;

bool mustBeNull;


public:
  static TypeTagForDatatypeAttr *CreateImplicit(ASTContext &Ctx, IdentifierInfo * ArgumentKind, TypeSourceInfo * MatchingCType, bool LayoutCompatible, bool MustBeNull, SourceRange Loc = SourceRange()) {
    TypeTagForDatatypeAttr *A = new (Ctx) TypeTagForDatatypeAttr(Loc, Ctx, ArgumentKind, MatchingCType, LayoutCompatible, MustBeNull, 0);
    A->setImplicit(true);
    return A;
  }

  TypeTagForDatatypeAttr(SourceRange R, ASTContext &Ctx
              , IdentifierInfo * ArgumentKind
              , TypeSourceInfo * MatchingCType
              , bool LayoutCompatible
              , bool MustBeNull
              , unsigned SI
             )
    : InheritableAttr(attr::TypeTagForDatatype, R, SI)
              , argumentKind(ArgumentKind)
              , matchingCType(MatchingCType)
              , layoutCompatible(LayoutCompatible)
              , mustBeNull(MustBeNull)
  {




  }

  TypeTagForDatatypeAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  IdentifierInfo * getArgumentKind() const {
    return argumentKind;
  }

  QualType getMatchingCType() const {
    return matchingCType->getType();
  }  TypeSourceInfo * getMatchingCTypeLoc() const {
    return matchingCType;
  }

  bool getLayoutCompatible() const {
    return layoutCompatible;
  }

  bool getMustBeNull() const {
    return mustBeNull;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::TypeTagForDatatype; }
  bool isLateParsed() const override { return 0; }
};

class TypeVisibilityAttr : public InheritableAttr {
public:
  enum VisibilityType {
    Default,
    Hidden,
    Protected
  };
private:
  VisibilityType visibility;


public:
  static TypeVisibilityAttr *CreateImplicit(ASTContext &Ctx, VisibilityType Visibility, SourceRange Loc = SourceRange()) {
    TypeVisibilityAttr *A = new (Ctx) TypeVisibilityAttr(Loc, Ctx, Visibility, 0);
    A->setImplicit(true);
    return A;
  }

  TypeVisibilityAttr(SourceRange R, ASTContext &Ctx
              , VisibilityType Visibility
              , unsigned SI
             )
    : InheritableAttr(attr::TypeVisibility, R, SI)
              , visibility(Visibility)
  {

  }

  TypeVisibilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  VisibilityType getVisibility() const {
    return visibility;
  }

  static bool ConvertStrToVisibilityType(StringRef Val, VisibilityType &Out) {
    Optional<VisibilityType> R = llvm::StringSwitch<Optional<VisibilityType>>(Val)
      .Case("default", TypeVisibilityAttr::Default)
      .Case("hidden", TypeVisibilityAttr::Hidden)
      .Case("internal", TypeVisibilityAttr::Hidden)
      .Case("protected", TypeVisibilityAttr::Protected)
      .Default(Optional<VisibilityType>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertVisibilityTypeToStr(VisibilityType Val) {
    switch(Val) {
    case TypeVisibilityAttr::Default: return "default";
    case TypeVisibilityAttr::Hidden: return "hidden";
    case TypeVisibilityAttr::Protected: return "protected";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::TypeVisibility; }
  bool isLateParsed() const override { return 0; }
};

class UnavailableAttr : public InheritableAttr {
unsigned messageLength;
char *message;


public:
  static UnavailableAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Message, SourceRange Loc = SourceRange()) {
    UnavailableAttr *A = new (Ctx) UnavailableAttr(Loc, Ctx, Message, 0);
    A->setImplicit(true);
    return A;
  }

  UnavailableAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Message
              , unsigned SI
             )
    : InheritableAttr(attr::Unavailable, R, SI)
              , messageLength(Message.size()),message(new (Ctx, 1) char[messageLength])
  {
      std::memcpy(message, Message.data(), messageLength);
  }

  UnavailableAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Unavailable, R, SI)
              , messageLength(0),message(0)
  {
  }

  UnavailableAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getMessage() const {
    return llvm::StringRef(message, messageLength);
  }
  unsigned getMessageLength() const {
    return messageLength;
  }
  void setMessage(ASTContext &C, llvm::StringRef S) {
    messageLength = S.size();
    this->message = new (C, 1) char [messageLength];
    std::memcpy(this->message, S.data(), messageLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Unavailable; }
  bool isLateParsed() const override { return 0; }
};

class UnusedAttr : public InheritableAttr {

public:
  static UnusedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    UnusedAttr *A = new (Ctx) UnusedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  UnusedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Unused, R, SI)
  {
  }

  UnusedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Unused; }
  bool isLateParsed() const override { return 0; }
};

class UsedAttr : public InheritableAttr {

public:
  static UsedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    UsedAttr *A = new (Ctx) UsedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  UsedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Used, R, SI)
  {
  }

  UsedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Used; }
  bool isLateParsed() const override { return 0; }
};

class UuidAttr : public InheritableAttr {
unsigned guidLength;
char *guid;


public:
  static UuidAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Guid, SourceRange Loc = SourceRange()) {
    UuidAttr *A = new (Ctx) UuidAttr(Loc, Ctx, Guid, 0);
    A->setImplicit(true);
    return A;
  }

  UuidAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Guid
              , unsigned SI
             )
    : InheritableAttr(attr::Uuid, R, SI)
              , guidLength(Guid.size()),guid(new (Ctx, 1) char[guidLength])
  {
      std::memcpy(guid, Guid.data(), guidLength);
  }

  UuidAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getGuid() const {
    return llvm::StringRef(guid, guidLength);
  }
  unsigned getGuidLength() const {
    return guidLength;
  }
  void setGuid(ASTContext &C, llvm::StringRef S) {
    guidLength = S.size();
    this->guid = new (C, 1) char [guidLength];
    std::memcpy(this->guid, S.data(), guidLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::Uuid; }
  bool isLateParsed() const override { return 0; }
};

class VecReturnAttr : public InheritableAttr {

public:
  static VecReturnAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    VecReturnAttr *A = new (Ctx) VecReturnAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  VecReturnAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::VecReturn, R, SI)
  {
  }

  VecReturnAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::VecReturn; }
  bool isLateParsed() const override { return 0; }
};

class VecTypeHintAttr : public InheritableAttr {
TypeSourceInfo * typeHint;


public:
  static VecTypeHintAttr *CreateImplicit(ASTContext &Ctx, TypeSourceInfo * TypeHint, SourceRange Loc = SourceRange()) {
    VecTypeHintAttr *A = new (Ctx) VecTypeHintAttr(Loc, Ctx, TypeHint, 0);
    A->setImplicit(true);
    return A;
  }

  VecTypeHintAttr(SourceRange R, ASTContext &Ctx
              , TypeSourceInfo * TypeHint
              , unsigned SI
             )
    : InheritableAttr(attr::VecTypeHint, R, SI)
              , typeHint(TypeHint)
  {

  }

  VecTypeHintAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  QualType getTypeHint() const {
    return typeHint->getType();
  }  TypeSourceInfo * getTypeHintLoc() const {
    return typeHint;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::VecTypeHint; }
  bool isLateParsed() const override { return 0; }
};

class VectorCallAttr : public InheritableAttr {

public:
  static VectorCallAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    VectorCallAttr *A = new (Ctx) VectorCallAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  VectorCallAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::VectorCall, R, SI)
  {
  }

  VectorCallAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::VectorCall; }
  bool isLateParsed() const override { return 0; }
};

class VisibilityAttr : public InheritableAttr {
public:
  enum VisibilityType {
    Default,
    Hidden,
    Protected
  };
private:
  VisibilityType visibility;


public:
  static VisibilityAttr *CreateImplicit(ASTContext &Ctx, VisibilityType Visibility, SourceRange Loc = SourceRange()) {
    VisibilityAttr *A = new (Ctx) VisibilityAttr(Loc, Ctx, Visibility, 0);
    A->setImplicit(true);
    return A;
  }

  VisibilityAttr(SourceRange R, ASTContext &Ctx
              , VisibilityType Visibility
              , unsigned SI
             )
    : InheritableAttr(attr::Visibility, R, SI)
              , visibility(Visibility)
  {

  }

  VisibilityAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  VisibilityType getVisibility() const {
    return visibility;
  }

  static bool ConvertStrToVisibilityType(StringRef Val, VisibilityType &Out) {
    Optional<VisibilityType> R = llvm::StringSwitch<Optional<VisibilityType>>(Val)
      .Case("default", VisibilityAttr::Default)
      .Case("hidden", VisibilityAttr::Hidden)
      .Case("internal", VisibilityAttr::Hidden)
      .Case("protected", VisibilityAttr::Protected)
      .Default(Optional<VisibilityType>());
    if (R) {
      Out = *R;
      return true;
    }
    return false;
  }

  static const char *ConvertVisibilityTypeToStr(VisibilityType Val) {
    switch(Val) {
    case VisibilityAttr::Default: return "default";
    case VisibilityAttr::Hidden: return "hidden";
    case VisibilityAttr::Protected: return "protected";
    }
    llvm_unreachable("No enumerator with that value");
  }


  static bool classof(const Attr *A) { return A->getKind() == attr::Visibility; }
  bool isLateParsed() const override { return 0; }
};

class WarnUnusedAttr : public InheritableAttr {

public:
  static WarnUnusedAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    WarnUnusedAttr *A = new (Ctx) WarnUnusedAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  WarnUnusedAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::WarnUnused, R, SI)
  {
  }

  WarnUnusedAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::WarnUnused; }
  bool isLateParsed() const override { return 0; }
};

class WarnUnusedResultAttr : public InheritableAttr {

public:
  static WarnUnusedResultAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    WarnUnusedResultAttr *A = new (Ctx) WarnUnusedResultAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  WarnUnusedResultAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::WarnUnusedResult, R, SI)
  {
  }

  WarnUnusedResultAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::WarnUnusedResult; }
  bool isLateParsed() const override { return 0; }
};

class WeakAttr : public InheritableAttr {

public:
  static WeakAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    WeakAttr *A = new (Ctx) WeakAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  WeakAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::Weak, R, SI)
  {
  }

  WeakAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::Weak; }
  bool isLateParsed() const override { return 0; }
};

class WeakImportAttr : public InheritableAttr {

public:
  static WeakImportAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    WeakImportAttr *A = new (Ctx) WeakImportAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  WeakImportAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::WeakImport, R, SI)
  {
  }

  WeakImportAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::WeakImport; }
  bool isLateParsed() const override { return 0; }
};

class WeakRefAttr : public InheritableAttr {
unsigned aliaseeLength;
char *aliasee;


public:
  static WeakRefAttr *CreateImplicit(ASTContext &Ctx, llvm::StringRef Aliasee, SourceRange Loc = SourceRange()) {
    WeakRefAttr *A = new (Ctx) WeakRefAttr(Loc, Ctx, Aliasee, 0);
    A->setImplicit(true);
    return A;
  }

  WeakRefAttr(SourceRange R, ASTContext &Ctx
              , llvm::StringRef Aliasee
              , unsigned SI
             )
    : InheritableAttr(attr::WeakRef, R, SI)
              , aliaseeLength(Aliasee.size()),aliasee(new (Ctx, 1) char[aliaseeLength])
  {
      std::memcpy(aliasee, Aliasee.data(), aliaseeLength);
  }

  WeakRefAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::WeakRef, R, SI)
              , aliaseeLength(0),aliasee(0)
  {
  }

  WeakRefAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  llvm::StringRef getAliasee() const {
    return llvm::StringRef(aliasee, aliaseeLength);
  }
  unsigned getAliaseeLength() const {
    return aliaseeLength;
  }
  void setAliasee(ASTContext &C, llvm::StringRef S) {
    aliaseeLength = S.size();
    this->aliasee = new (C, 1) char [aliaseeLength];
    std::memcpy(this->aliasee, S.data(), aliaseeLength);
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::WeakRef; }
  bool isLateParsed() const override { return 0; }
};

class WorkGroupSizeHintAttr : public InheritableAttr {
unsigned xDim;

unsigned yDim;

unsigned zDim;


public:
  static WorkGroupSizeHintAttr *CreateImplicit(ASTContext &Ctx, unsigned XDim, unsigned YDim, unsigned ZDim, SourceRange Loc = SourceRange()) {
    WorkGroupSizeHintAttr *A = new (Ctx) WorkGroupSizeHintAttr(Loc, Ctx, XDim, YDim, ZDim, 0);
    A->setImplicit(true);
    return A;
  }

  WorkGroupSizeHintAttr(SourceRange R, ASTContext &Ctx
              , unsigned XDim
              , unsigned YDim
              , unsigned ZDim
              , unsigned SI
             )
    : InheritableAttr(attr::WorkGroupSizeHint, R, SI)
              , xDim(XDim)
              , yDim(YDim)
              , zDim(ZDim)
  {



  }

  WorkGroupSizeHintAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;
  unsigned getXDim() const {
    return xDim;
  }

  unsigned getYDim() const {
    return yDim;
  }

  unsigned getZDim() const {
    return zDim;
  }



  static bool classof(const Attr *A) { return A->getKind() == attr::WorkGroupSizeHint; }
  bool isLateParsed() const override { return 0; }
};

class X86ForceAlignArgPointerAttr : public InheritableAttr {

public:
  static X86ForceAlignArgPointerAttr *CreateImplicit(ASTContext &Ctx, SourceRange Loc = SourceRange()) {
    X86ForceAlignArgPointerAttr *A = new (Ctx) X86ForceAlignArgPointerAttr(Loc, Ctx, 0);
    A->setImplicit(true);
    return A;
  }

  X86ForceAlignArgPointerAttr(SourceRange R, ASTContext &Ctx
              , unsigned SI
             )
    : InheritableAttr(attr::X86ForceAlignArgPointer, R, SI)
  {
  }

  X86ForceAlignArgPointerAttr *clone(ASTContext &C) const override;
  void printPretty(raw_ostream &OS,
                   const PrintingPolicy &Policy) const override;
  const char *getSpelling() const override;


  static bool classof(const Attr *A) { return A->getKind() == attr::X86ForceAlignArgPointer; }
  bool isLateParsed() const override { return 0; }
};

#endif
