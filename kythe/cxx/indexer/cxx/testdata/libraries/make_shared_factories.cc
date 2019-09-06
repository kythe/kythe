#include <memory>

//- @B defines/binding StructB
struct B {
  //- @B defines/binding ConsB1
  explicit B(int, void*);
  //- @B defines/binding ConsB2
  explicit B(char);
};

void f() {
  //- @B ref StructB
  //- @"make_shared<B>" ref ConsB1
  //- @"std::make_shared<B>(1, nullptr)" ref/call ConsB1
  std::make_shared<B>(1, nullptr);

  //- @B ref StructB
  //- @"make_shared<B>" ref ConsB2
  //- @"std::make_shared<B>('a')" ref/call ConsB2
  std::make_shared<B>('a');
}
