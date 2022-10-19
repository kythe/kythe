// Package writes tests ref/writes edges
package writes

import "fmt"

func f() {
	//- @x defines/binding X
	var x int

	//- @x ref/writes X
	//- @y defines/binding Y
	x, y := 1, 2

	//- @y ref Y
	//- @y ref/writes Y
	y++

	//- @#0i defines/binding I
	//- @#1i ref I
	//- @#2i ref/writes I
	//- @#3i ref I
	for i := 0; i < 10; i = i + 1 {
	}

	//- @x ref X
	//- @y ref Y
	fmt.Println(x, y)
}
