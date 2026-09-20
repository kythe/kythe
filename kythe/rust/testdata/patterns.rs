struct Point {
    // @x defines/binding PointX
    x: i32,
    // @y defines/binding PointY
    y: i32,
}

//- @x defines/binding ParamX
//- @z defines/binding ParamZ
fn assemble(x: i32, z: i32) {
    //- @x ref PointX
    //- @x ref ParamX
    //- @y ref PointY
    //- @z ref ParamZ
    Point { x, y: z };
}

fn disassemble(p: Point) {
    //- @x defines/binding LocalX
    //- @x ref PointX
    //- @y defines/binding LocalY
    //- @y ref PointY
    let Point { x, y } = p;
    //- @x ref LocalX
    x;
}

enum Option<T> {
    // @None defines/binding None
    None,
    // @Some defines/binding Some
    Some(T),
}

fn const_ref(opt: Option<i32>) {
    // @None ref None
    // !{ @None defines/binding _ }
    let Option::None = opt else {
        return;
    };
}
