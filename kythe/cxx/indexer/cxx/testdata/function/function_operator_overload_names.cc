// Checks how we name operator overload functions.
// Needs --ignore_dups=true because of noncanonicalized function types.
struct S {
//- @"operator=" defines OpEq
//- OpEq named vname("OO#Equal:S#n",_,_,_,_)
  void operator=(int);
//- @"operator()" defines OpCall
//- OpCall named vname("OO#Call:S#n",_,_,_,_)
  void operator()();
//- @"operator[]" defines OpIndex
//- OpIndex named vname("OO#Subscript:S#n",_,_,_,_)
  void operator[](int);
//- @"operator->" defines OpArrow
//- OpArrow named vname("OO#Arrow:S#n",_,_,_,_)
  void operator->();
};
//- @"operator" defines OpNew
//- OpNew named vname("OO#New#n",_,_,_,_)
void* operator new(unsigned long);
//- @"operator" defines OpDelete
//- OpDelete named vname("OO#Delete#n",_,_,_,_)
void operator delete(void*);
//- @"operator" defines OpNewArray
//- OpNewArray named vname("OO#Array_New#n",_,_,_,_)
void* operator new[](unsigned long);
//- @"operator" defines OpDeleteArray
//- OpDeleteArray named vname("OO#Array_Delete#n",_,_,_,_)
void operator delete[](void*);
//- @"operator+" defines OpPlus
//- OpPlus named vname("OO#Plus#n",_,_,_,_)
void operator+(S a, S b);
//- @"operator-" defines OpMinus
//- OpMinus named vname("OO#Minus#n",_,_,_,_)
void operator-(S a, S b);
//-  @"operator*" defines OpStar
//-  OpStar named vname("OO#Star#n",_,_,_,_)
void operator*(S a, S b);
//-  @"operator/" defines OpSlash
//-  OpSlash named vname("OO#Slash#n",_,_,_,_)
void operator/(S a, S b);
//-  @"operator%" defines OpPercent
//-  OpPercent named vname("OO#Percent#n",_,_,_,_)
void operator%(S a, S b);
//-  @"operator^" defines OpCaret
//-  OpCaret named vname("OO#Caret#n",_,_,_,_)
void operator^(S a, S b);
//-  @"operator&" defines OpAmp
//-  OpAmp named vname("OO#Amp#n",_,_,_,_)
void operator&(S a, S b);
//-  @"operator|" defines OpPipe
//-  OpPipe named vname("OO#Pipe#n",_,_,_,_)
void operator|(S a, S b);
//-  @"operator~" defines OpTilde
//-  OpTilde named vname("OO#Tilde#n",_,_,_,_)
void operator~(S a);
//-  @"operator!" defines OpExclaim
//-  OpExclaim named vname("OO#Exclaim#n",_,_,_,_)
void operator!(S a);
//-  @"operator<" defines OpLess
//-  OpLess named vname("OO#Less#n",_,_,_,_)
void operator<(S a, S b);
//-  @"operator>" defines OpGreater
//-  OpGreater named vname("OO#Greater#n",_,_,_,_)
void operator>(S a, S b);
//-  @"operator+=" defines OpPlusEqual
//-  OpPlusEqual named vname("OO#PlusEqual#n",_,_,_,_)
void operator+=(S a, S b);
//-  @"operator-=" defines OpMinusEqual
//-  OpMinusEqual named vname("OO#MinusEqual#n",_,_,_,_)
void operator-=(S a, S b);
//-  @"operator*=" defines OpStarEqual
//-  OpStarEqual named vname("OO#StarEqual#n",_,_,_,_)
void operator*=(S a, S b);
//-  @"operator/=" defines OpSlashEqual
//-  OpSlashEqual named vname("OO#SlashEqual#n",_,_,_,_)
void operator/=(S a, S b);
//-  @"operator%=" defines OpPercentEqual
//-  OpPercentEqual named vname("OO#PercentEqual#n",_,_,_,_)
void operator%=(S a, S b);
//-  @"operator^=" defines OpCaretEqual
//-  OpCaretEqual named vname("OO#CaretEqual#n",_,_,_,_)
void operator^=(S a, S b);
//-  @"operator&=" defines OpAmpEqual
//-  OpAmpEqual named vname("OO#AmpEqual#n",_,_,_,_)
void operator&=(S a, S b);
//-  @"operator|=" defines OpPipeEqual
//-  OpPipeEqual named vname("OO#PipeEqual#n",_,_,_,_)
void operator|=(S a, S b);
//-  @"operator<<" defines OpLessLess
//-  OpLessLess named vname("OO#LessLess#n",_,_,_,_)
void operator<<(S a, S b);
//-  @"operator>>" defines OpGreaterGreater
//-  OpGreaterGreater named vname("OO#GreaterGreater#n",_,_,_,_)
void operator>>(S a, S b);
//-  @"operator<<=" defines OpLessLessEqual
//-  OpLessLessEqual named vname("OO#LessLessEqual#n",_,_,_,_)
void operator<<=(S a, S b);
//-  @"operator>>=" defines OpGreaterGreaterEqual
//-  OpGreaterGreaterEqual named vname("OO#GreaterGreaterEqual#n",_,_,_,_)
void operator>>=(S a, S b);
//-  @"operator==" defines OpEqualEqual
//-  OpEqualEqual named vname("OO#EqualEqual#n",_,_,_,_)
void operator==(S a, S b);
//-  @"operator!=" defines OpExclaimEqual
//-  OpExclaimEqual named vname("OO#ExclaimEqual#n",_,_,_,_)
void operator!=(S a, S b);
//-  @"operator<=" defines OpLessEqual
//-  OpLessEqual named vname("OO#LessEqual#n",_,_,_,_)
void operator<=(S a, S b);
//-  @"operator>=" defines OpGreaterEqual
//-  OpGreaterEqual named vname("OO#GreaterEqual#n",_,_,_,_)
void operator>=(S a, S b);
//-  @"operator&&" defines OpAmpAmp
//-  OpAmpAmp named vname("OO#AmpAmp#n",_,_,_,_)
void operator&&(S a, S b);
//-  @"operator||" defines OpPipePipe
//-  OpPipePipe named vname("OO#PipePipe#n",_,_,_,_)
void operator||(S a, S b);
//-  @"operator++" defines OpPlusPlus
//-  OpPlusPlus named vname("OO#PlusPlus#n",_,_,_,_)
void operator++(S a, int b);
//-  @"operator--" defines OpMinusMinus
//-  OpMinusMinus named vname("OO#MinusMinus#n",_,_,_,_)
void operator--(S a, int b);
//-  @"operator," defines OpComma
//-  OpComma named vname("OO#Comma#n",_,_,_,_)
void operator,(S a, S b);
//-  @"operator->*" defines OpArrowStar
//-  OpArrowStar named vname("OO#ArrowStar#n",_,_,_,_)
void operator->*(S a, S b);
