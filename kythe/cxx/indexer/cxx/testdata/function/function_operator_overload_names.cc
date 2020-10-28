// Checks how we link operator overload functions.
struct S {
//- @"operator=" defines/binding OpEq
  void operator=(int);
//- @"operator()" defines/binding OpCall
  void operator()();
//- @"operator[]" defines/binding OpIndex
  void operator[](int);
//- @"operator->" defines/binding OpArrow
  void operator->();
};
//- @"operator new" defines/binding OpNew
void* operator new(unsigned long);
//- @"operator delete" defines/binding OpDelete
void operator delete(void*);
//- @"operator new[]" defines/binding OpNewArray
void* operator new[](unsigned long);
//- @"operator delete[]" defines/binding OpDeleteArray
void operator delete[](void*);
//- @"operator+" defines/binding OpPlus
void operator+(S a, S b);
//- @"operator-" defines/binding OpMinus
void operator-(S a, S b);
//-  @"operator*" defines/binding OpStar
void operator*(S a, S b);
//-  @"operator/" defines/binding OpSlash
void operator/(S a, S b);
//-  @"operator%" defines/binding OpPercent
void operator%(S a, S b);
//-  @"operator^" defines/binding OpCaret
void operator^(S a, S b);
//-  @"operator&" defines/binding OpAmp
void operator&(S a, S b);
//-  @"operator|" defines/binding OpPipe
void operator|(S a, S b);
//-  @"operator~" defines/binding OpTilde
void operator~(S a);
//-  @"operator!" defines/binding OpExclaim
void operator!(S a);
//-  @"operator<" defines/binding OpLess
void operator<(S a, S b);
//-  @"operator>" defines/binding OpGreater
void operator>(S a, S b);
//-  @"operator+=" defines/binding OpPlusEqual
void operator+=(S a, S b);
//-  @"operator-=" defines/binding OpMinusEqual
void operator-=(S a, S b);
//-  @"operator*=" defines/binding OpStarEqual
void operator*=(S a, S b);
//-  @"operator/=" defines/binding OpSlashEqual
void operator/=(S a, S b);
//-  @"operator%=" defines/binding OpPercentEqual
void operator%=(S a, S b);
//-  @"operator^=" defines/binding OpCaretEqual
void operator^=(S a, S b);
//-  @"operator&=" defines/binding OpAmpEqual
void operator&=(S a, S b);
//-  @"operator|=" defines/binding OpPipeEqual
void operator|=(S a, S b);
//-  @"operator<<" defines/binding OpLessLess
void operator<<(S a, S b);
//-  @"operator>>" defines/binding OpGreaterGreater
void operator>>(S a, S b);
//-  @"operator<<=" defines/binding OpLessLessEqual
void operator<<=(S a, S b);
//-  @"operator>>=" defines/binding OpGreaterGreaterEqual
void operator>>=(S a, S b);
//-  @"operator==" defines/binding OpEqualEqual
void operator==(S a, S b);
//-  @"operator!=" defines/binding OpExclaimEqual
void operator!=(S a, S b);
//-  @"operator<=" defines/binding OpLessEqual
void operator<=(S a, S b);
//-  @"operator>=" defines/binding OpGreaterEqual
void operator>=(S a, S b);
//-  @"operator&&" defines/binding OpAmpAmp
void operator&&(S a, S b);
//-  @"operator||" defines/binding OpPipePipe
void operator||(S a, S b);
//-  @"operator++" defines/binding OpPlusPlus
void operator++(S a, int b);
//-  @"operator--" defines/binding OpMinusMinus
void operator--(S a, int b);
//-  @"operator," defines/binding OpComma
void operator,(S a, S b);
//-  @"operator->*" defines/binding OpArrowStar
void operator->*(S a, S b);
