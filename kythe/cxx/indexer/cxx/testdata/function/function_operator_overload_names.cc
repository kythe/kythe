// Checks how we name operator overload functions.
// Needs --ignore_dups=true because of noncanonicalized function types.
struct S {
//- @"operator=" defines/binding OpEq
//- OpEq named vname("OO#Equal:S#n",_,_,_,_)
  void operator=(int);
//- @"operator()" defines/binding OpCall
//- OpCall named vname("OO#Call:S#n",_,_,_,_)
  void operator()();
//- @"operator[]" defines/binding OpIndex
//- OpIndex named vname("OO#Subscript:S#n",_,_,_,_)
  void operator[](int);
//- @"operator->" defines/binding OpArrow
//- OpArrow named vname("OO#Arrow:S#n",_,_,_,_)
  void operator->();
};
//- @"operator" defines/binding OpNew
//- OpNew named vname("OO#New#n",_,_,_,_)
void* operator new(unsigned long);
//- @"operator" defines/binding OpDelete
//- OpDelete named vname("OO#Delete#n",_,_,_,_)
void operator delete(void*);
//- @"operator" defines/binding OpNewArray
//- OpNewArray named vname("OO#Array_New#n",_,_,_,_)
void* operator new[](unsigned long);
//- @"operator" defines/binding OpDeleteArray
//- OpDeleteArray named vname("OO#Array_Delete#n",_,_,_,_)
void operator delete[](void*);
//- @"operator+" defines/binding OpPlus
//- OpPlus named vname("OO#Plus#n",_,_,_,_)
void operator+(S a, S b);
//- @"operator-" defines/binding OpMinus
//- OpMinus named vname("OO#Minus#n",_,_,_,_)
void operator-(S a, S b);
//-  @"operator*" defines/binding OpStar
//-  OpStar named vname("OO#Star#n",_,_,_,_)
void operator*(S a, S b);
//-  @"operator/" defines/binding OpSlash
//-  OpSlash named vname("OO#Slash#n",_,_,_,_)
void operator/(S a, S b);
//-  @"operator%" defines/binding OpPercent
//-  OpPercent named vname("OO#Percent#n",_,_,_,_)
void operator%(S a, S b);
//-  @"operator^" defines/binding OpCaret
//-  OpCaret named vname("OO#Caret#n",_,_,_,_)
void operator^(S a, S b);
//-  @"operator&" defines/binding OpAmp
//-  OpAmp named vname("OO#Amp#n",_,_,_,_)
void operator&(S a, S b);
//-  @"operator|" defines/binding OpPipe
//-  OpPipe named vname("OO#Pipe#n",_,_,_,_)
void operator|(S a, S b);
//-  @"operator~" defines/binding OpTilde
//-  OpTilde named vname("OO#Tilde#n",_,_,_,_)
void operator~(S a);
//-  @"operator!" defines/binding OpExclaim
//-  OpExclaim named vname("OO#Exclaim#n",_,_,_,_)
void operator!(S a);
//-  @"operator<" defines/binding OpLess
//-  OpLess named vname("OO#Less#n",_,_,_,_)
void operator<(S a, S b);
//-  @"operator>" defines/binding OpGreater
//-  OpGreater named vname("OO#Greater#n",_,_,_,_)
void operator>(S a, S b);
//-  @"operator+=" defines/binding OpPlusEqual
//-  OpPlusEqual named vname("OO#PlusEqual#n",_,_,_,_)
void operator+=(S a, S b);
//-  @"operator-=" defines/binding OpMinusEqual
//-  OpMinusEqual named vname("OO#MinusEqual#n",_,_,_,_)
void operator-=(S a, S b);
//-  @"operator*=" defines/binding OpStarEqual
//-  OpStarEqual named vname("OO#StarEqual#n",_,_,_,_)
void operator*=(S a, S b);
//-  @"operator/=" defines/binding OpSlashEqual
//-  OpSlashEqual named vname("OO#SlashEqual#n",_,_,_,_)
void operator/=(S a, S b);
//-  @"operator%=" defines/binding OpPercentEqual
//-  OpPercentEqual named vname("OO#PercentEqual#n",_,_,_,_)
void operator%=(S a, S b);
//-  @"operator^=" defines/binding OpCaretEqual
//-  OpCaretEqual named vname("OO#CaretEqual#n",_,_,_,_)
void operator^=(S a, S b);
//-  @"operator&=" defines/binding OpAmpEqual
//-  OpAmpEqual named vname("OO#AmpEqual#n",_,_,_,_)
void operator&=(S a, S b);
//-  @"operator|=" defines/binding OpPipeEqual
//-  OpPipeEqual named vname("OO#PipeEqual#n",_,_,_,_)
void operator|=(S a, S b);
//-  @"operator<<" defines/binding OpLessLess
//-  OpLessLess named vname("OO#LessLess#n",_,_,_,_)
void operator<<(S a, S b);
//-  @"operator>>" defines/binding OpGreaterGreater
//-  OpGreaterGreater named vname("OO#GreaterGreater#n",_,_,_,_)
void operator>>(S a, S b);
//-  @"operator<<=" defines/binding OpLessLessEqual
//-  OpLessLessEqual named vname("OO#LessLessEqual#n",_,_,_,_)
void operator<<=(S a, S b);
//-  @"operator>>=" defines/binding OpGreaterGreaterEqual
//-  OpGreaterGreaterEqual named vname("OO#GreaterGreaterEqual#n",_,_,_,_)
void operator>>=(S a, S b);
//-  @"operator==" defines/binding OpEqualEqual
//-  OpEqualEqual named vname("OO#EqualEqual#n",_,_,_,_)
void operator==(S a, S b);
//-  @"operator!=" defines/binding OpExclaimEqual
//-  OpExclaimEqual named vname("OO#ExclaimEqual#n",_,_,_,_)
void operator!=(S a, S b);
//-  @"operator<=" defines/binding OpLessEqual
//-  OpLessEqual named vname("OO#LessEqual#n",_,_,_,_)
void operator<=(S a, S b);
//-  @"operator>=" defines/binding OpGreaterEqual
//-  OpGreaterEqual named vname("OO#GreaterEqual#n",_,_,_,_)
void operator>=(S a, S b);
//-  @"operator&&" defines/binding OpAmpAmp
//-  OpAmpAmp named vname("OO#AmpAmp#n",_,_,_,_)
void operator&&(S a, S b);
//-  @"operator||" defines/binding OpPipePipe
//-  OpPipePipe named vname("OO#PipePipe#n",_,_,_,_)
void operator||(S a, S b);
//-  @"operator++" defines/binding OpPlusPlus
//-  OpPlusPlus named vname("OO#PlusPlus#n",_,_,_,_)
void operator++(S a, int b);
//-  @"operator--" defines/binding OpMinusMinus
//-  OpMinusMinus named vname("OO#MinusMinus#n",_,_,_,_)
void operator--(S a, int b);
//-  @"operator," defines/binding OpComma
//-  OpComma named vname("OO#Comma#n",_,_,_,_)
void operator,(S a, S b);
//-  @"operator->*" defines/binding OpArrowStar
//-  OpArrowStar named vname("OO#ArrowStar#n",_,_,_,_)
void operator->*(S a, S b);
