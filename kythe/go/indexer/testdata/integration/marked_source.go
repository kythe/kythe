package marked

//- @A defines/binding AFunc
//- AFunc.code/rendered "func test/marked_source.A(test/marked_source.a string, test/marked_source.b int)"
func A(a string, b int) {}

func B() { A("hello", 42) }
