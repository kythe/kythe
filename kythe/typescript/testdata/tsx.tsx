// Test TypeScript JSX

export {};

function render() {
  //- @#0"value" defines/binding Value
  const value = 'value';
  return (
    //- @"attr" defines/binding vname("render.div.attr", _, _, _, _)
    //- @"value" ref Value
    //- @+3"src" defines/binding vname("render.div.img.src", _, _, _, _)
    //- @+2"value" ref Value
    <div attr={value}>
      <img src={value} />
    </div>
  );
}
