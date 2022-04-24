/**
 * @fileoverview File that test that interface properties are indexed correctly.
 */

interface Person {
  //- @name defines/binding Name
  name: string;
  //- @getAge defines/binding GetAge
  getAge(): void;
}

const p: Person = {
  //- @name ref Name
  name: 'Alice',
  //- @getAge ref GetAge
  getAge() {}
};

const p2 = {
  //- @name ref Name
  name: 'Alice',
  //- @getAge ref GetAge
  getAge() {}
} as Person;

//- @name ref Name
p.name;

//- @getAge ref GetAge
p.getAge();

//- @getAge ref GetAge
const {getAge} = p;

//- @name ref Name
//- @getAge ref GetAge
function takesPerson({name, getAge: newGetAge}: Person) {}

//- @name ref Name
//- @getAge ref GetAge
takesPerson({name: 'Alice', getAge() {}});

class PersonTaker {
  constructor(p: Person) {}
}

//- @name ref Name
//- @getAge ref GetAge
new PersonTaker({name: 'Alice', getAge() {}});

function returnPerson(): Person {
  //- @name ref Name
  //- @getAge ref GetAge
  return {name: 'Alice', getAge() {}};
}
