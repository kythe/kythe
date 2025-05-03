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
  //- @name ref/id Name
  name: 'Alice',
  //- @getAge ref/id GetAge
  getAge() {}
};

{
  const p2 = {
    //- @name ref/id Name
    name: 'Alice',
    //- @getAge ref/id GetAge
    getAge() {}
  } as Person;

  //- @name ref Name
  p.name;

  //- @getAge ref GetAge
  p.getAge();

  //- @getAge ref/id GetAge
  const {getAge} = p;
}

//- @name ref/id Name
//- @getAge ref/id GetAge
function takesPerson({name, getAge: newGetAge}: Person) {}

//- @name ref/id Name
//- @getAge ref/id GetAge
takesPerson({name: 'Alice', getAge() {}});

class PersonTaker {
  constructor(p: Person) {}
}

//- @name ref/id Name
//- @getAge ref/id GetAge
new PersonTaker({name: 'Alice', getAge() {}});

function returnPerson(): Person {
  //- @name ref/id Name
  //- @getAge ref/id GetAge
  return {name: 'Alice', getAge() {}};
}

// test property shorthands
{
  const name = 'Alice';
  const getAge = () => {};
  //- @name ref/id Name
  //- @getAge ref/id GetAge
  const p3: Person = {name, getAge};
}

interface Address {
  //- @person defines/binding Person
  person: Person;
  street: string;
}

// Test nested objects.
{
  const address: Address = {
    //- @person ref/id Person
    person: {
      //- @name ref/id Name
      name: 'Alice',
      //- @getAge ref/id GetAge
      getAge() {}
    },
    street: '1600 Amphitheater Park',
  }

  //- @name ref/id Name
  const {person: {name}} = ({} as Address);

  //- @name ref/id Name
  function takesAddress({person: {name}}: Address) {}
}

{
  // Test properties with a type wrapper
  const p: Readonly<Person> = {
    //- @name ref/id Name
    name: 'Alice',
    //- @getAge ref/id GetAge
    getAge() {}
  };

  // test property shorthands with a type wrapper
  {
    const name = 'Alice';
    const getAge = () => {};
    //- @name ref/id Name
    //- @getAge ref/id GetAge
    const p3: Readonly<Person> = {name, getAge};
  }

  //- @getAge ref/id GetAge
  const {getAge} = p;
}
