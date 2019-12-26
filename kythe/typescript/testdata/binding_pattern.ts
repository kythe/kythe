// Tests TypeScript binding patterns.

//- @a defines/binding A=vname("testdata/binding_pattern/a", _, _, _, _)
//- @b defines/binding B=vname("testdata/binding_pattern/b", _, _, _, _)
let [a, b] = [1, 2];

//- @a ref A
//- @b ref B
a = b;

//- @#0"c" defines/binding C=vname("testdata/binding_pattern/c", _, _, _, _)
//- @letD defines/binding D=vname("testdata/binding_pattern/letD", _, _, _, _)
let {c, d: letD} = {c: 0, d: 0};

//- @c ref C
//- @letD ref D
c = letD;
