// Package pkg verifies that the required package structure is created.
//- @pkg ref Pkg
package pkg

//- Pkg=vname("package", "test", _, "pkg", "go").node/kind package
//- File=vname("", "test", _, "src/test/pkg/packages.go", "").node/kind file
//- File childof Pkg
