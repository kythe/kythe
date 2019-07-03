export {}

const shortProperty = 0;

//- @#0"computed" defines/binding Computed
const computed = 'computed';

const Object = {
  //- @property defines/binding Property
  //- Property.node/kind variable
  property: 3,

  //- @shortProperty defines/binding ShortProperty
  //- ShortProperty.node/kind variable
  shortProperty,

  //- @computed ref Computed
  [computed]: 0,

  //- @method defines/binding Method
  //- Method.node/kind function
  method() {}
};

//- @property ref Property
const x = Object.property;

//- @method ref Method
Object.method();
