// Test TypeScript JSX

export {};

function render() {
  //- @#0"value" defines/binding Value
  const value = 'value';
  return (
    //- @"attr" defines/binding Attr
    //- @"value" ref Value
    //- Attr code AttrCode
    //- AttrCode child.0 AttrName
    //- AttrName.pre_text "attr"
    //- AttrCode child.1 AttrTy
    //- AttrTy.post_text "string"
    //- AttrCode child.2 AttrInit
    //- AttrInit.pre_text "{value}"
    //- @+4"src" defines/binding _Src1
    //- @+3"value" ref Value
    //- @+3"src" defines/binding _Src2
    <div attr={value}>
      <img src={value} />
      <img src={value} />
    </div>
  );
}
