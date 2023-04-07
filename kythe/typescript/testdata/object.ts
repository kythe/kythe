export {}

//- @shortProperty defines/binding ShortPropertyVariable
const shortProperty = 0;

//- @#0"computed" defines/binding Computed
const computed = 'computed';

const Object = {
  //- @property defines/binding Property
  //- Property.node/kind variable
  //- Property.subkind field
  property: 3,

  //- @shortProperty defines/binding ShortProperty
  //- ShortProperty.node/kind variable
  //- ShortProperty.subkind field
  //- @shortProperty ref/id ShortPropertyVariable
  shortProperty,

  //- !{@"[computed]" defines/binding _}
  //- @computed ref Computed
  [computed]: 0,

  //- @method defines/binding Method
  //- Method.node/kind function
  method() {},

  //- @"'string#literal'" defines/binding SLiteralProperty
  //- SLiteralProperty.node/kind variable
  'string#literal': 0,

  //- @"123" defines/binding NLiteralProperty
  //- NLiteralProperty.node/kind variable
  //- NLiteralProperty.subkind field
  123: 'nliteral',
};

//- @property ref Property
//- @shortProperty ref ShortProperty
const x = Object.property || Object.shortProperty;

//- @computed ref Computed
Object[computed];

//- @"'string#literal'" ref SLiteralProperty
Object['string#literal'];

//- @"123" ref NLiteralProperty
Object[123];

//- @method ref Method
Object.method();
