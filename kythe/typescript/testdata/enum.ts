export {}

//- @Fruit defines/binding FruitType
//- FruitType.node/kind record
//- @Fruit defines/binding FruitVal
//- FruitVal.node/kind constant
enum Fruit {
  //- @APPLE defines/binding APPLE
  //- APPLE.node/kind constant
  APPLE,
  PEAR
}

//- @Fruit ref FruitType
let x: Fruit;
//- @Fruit ref FruitVal
//- @APPLE ref APPLE
x = Fruit.APPLE;
