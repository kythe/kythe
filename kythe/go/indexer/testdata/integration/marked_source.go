package marked

// - @A defines/binding AFunc
// - AFunc.code/rendered "func test/marked_source.A(test/marked_source.a string, test/marked_source.b int)"
// - AFunc.code/rendered/identifier "A"
// - AFunc.code/rendered/params "a,b"
func A(a string, b int) {}

func B() { A("hello", 42) }
