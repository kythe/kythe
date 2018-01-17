export {}

const Object = {
  //- @property defines/binding Property
  //- Property.node/kind variable
  property: 3,

  //- @method defines/binding Method
  //- Method.node/kind function
  method() {
  }
};

//- @property ref Property
const x = Object.property;

//- @method ref Method
Object.method();
