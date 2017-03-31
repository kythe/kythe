// We index calls to function templates with parameter pack arguments.
template <typename... Ts>
//- @f defines/binding FnTF
void f(Ts... ts) { }

void g() {
//- @f ref AppFnTFSigma
  f(1, "one");
}

//- @cc defines/binding CC CC typed String
const char *cc;
//- @tn defines/binding TN TN typed Int
int tn;

//- FnF instantiates AppFnTFSigma
//- FnF.node/kind function
//- AppFnTFSigma param.0 FnTF
//- AppFnTFSigma param.1 Sigma
//- Sigma.node/kind tsigma
//- Sigma param.0 Int
//- Sigma param.1 String
