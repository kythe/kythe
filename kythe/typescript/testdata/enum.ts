export {}

//- @Fruit defines/binding FruitType
//- FruitType.node/kind record
//- @Fruit defines/binding FruitVal
//- FruitVal.node/kind constant
enum Fruit {
  //- @APPLE defines/binding Apple
  //- Apple.node/kind constant
  //- Apple childof FruitType
  APPLE,
  PEAR
}

//- @Fruit ref FruitType
let x: Fruit;
//- @Fruit ref FruitVal
//- @APPLE ref Apple
x = Fruit.APPLE;
