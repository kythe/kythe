// Package pkg verifies that the required package structure is created.
//- @pkg defines/binding Pkg
package pkg

//- Pkg=vname("package", "test", _, "pkg", "go").node/kind package
//- Pkg.doc/uri "http://godoc.org/test/pkg"
//- File=vname("", _, _, "src/test/pkg/packages.go", "").node/kind file
//- File childof Pkg
