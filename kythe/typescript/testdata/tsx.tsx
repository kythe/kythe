// Test TypeScript JSX

export {};

function render() {
  //- @#0"value" defines/binding Value
  const value = 'value';
  return (
    //- @"attr" defines/binding _Attr
    //- @"value" ref Value
    //- @+4"src" defines/binding _Src
    //- @+3"value" ref Value
    //- @+3"src" defines/binding _Src2
    <div attr={value}>
      <img src={value} />
      <img src={value} />
    </div>
  );
}
