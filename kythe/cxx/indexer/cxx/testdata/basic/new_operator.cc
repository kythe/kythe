//- @"operator new" defines/binding OpNew
void *operator new(__SIZE_TYPE__);

void f() {
  //- @"new" ref OpNew
  new int;
}
