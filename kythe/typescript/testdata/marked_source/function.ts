//- @p1 defines/binding A_p1
//- A_p1 code A_p1_code
//- A_p1_code child.0 A_p1_context
//- A_p1_context.pre_text "(parameter)"
//- A_p1_code child.2 A_p1_name
//- A_p1_name.pre_text "p1"
//- A_p1_code child.3 A_p1_ty
//- A_p1_ty.post_text "number"
//- @p2 defines/binding A_p2
//- A_p2 code A_p2_code
//- A_p2_code child.0 A_p2_context
//- A_p2_context.pre_text "(parameter)"
//- A_p2_code child.2 A_p2_name
//- A_p2_name.pre_text "p2"
//- A_p2_code child.3 A_p2_ty
//- A_p2_ty.post_text "number"
//- A_p2_code child.5 A_p2_init
//- A_p2_init.pre_text "2"
//- @p3 defines/binding A_p3
//- A_p3 code A_p3_code
//- A_p3_code child.0 A_p3_context
//- A_p3_context.pre_text "(parameter)"
//- A_p3_code child.2 A_p3_name
//- A_p3_name.pre_text "p3"
//- A_p3_code child.3 A_p3_ty
//- A_p3_ty.post_text "number"
//- A_p3_code child.5 A_p3_init
//- A_p3_init.pre_text "3"
//- @p4 defines/binding A_p4
//- A_p4 code A_p4_code
//- A_p4_code child.0 A_p4_context
//- A_p4_context.pre_text "(parameter)"
//- A_p4_code child.2 A_p4_name
//- A_p4_name.pre_text "p4"
//- A_p4_code child.3 A_p4_ty
//- A_p4_ty.post_text "number"
//- A_p4_code child.5 A_p4_init
//- A_p4_init.pre_text "4"
function a(p1: number, p2: number = 2, {p3, prop: {p4}} = {
  p3: 3,
  prop: {p4: 4}
}) {
  return p1 + p2 + p3 + p4;
}
