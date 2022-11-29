// Tests TypeScript binding patterns.

//- @a defines/binding A=vname("a", _, _, _, _)
//- @b defines/binding B=vname("b", _, _, _, _)
let [a, b] = [1, 2];

//- @a ref/writes A
//- @b ref B
a = b;

//- @#0"c" defines/binding C=vname("c", _, _, _, _)
//- @letD defines/binding D=vname("letD", _, _, _, _)
let {c, d: letD} = {c: 0, d: 0};

//- @c ref/writes C
//- @letD ref D
c = letD;
