// Package magic tests resolution of the "unsafe" metapackage.
package magic

//- @"\"unsafe\"" ref/imports Unsafe=vname("package", "golang.org", _, "unsafe", "go")
import "unsafe"

//- @foo defines/binding Foo
//- Foo.node/kind variable
//- @unsafe ref Unsafe
//- @Pointer ref _UnsafePointer
var foo unsafe.Pointer
