
//- @get_vec defines/binding Fget_vec
fn get_vec() -> Vec<i32> {
  vec![1,2,3]
}
//- @foo defines/binding Ffoo
fn foo(){
  //- @"get_vec()" ref/call Fget_vec
  //- @"get_vec()" childof Ffoo
  //- @get_vec ref Fget_vec
  for num in get_vec() {
    println!("{}", num);
  }
}
