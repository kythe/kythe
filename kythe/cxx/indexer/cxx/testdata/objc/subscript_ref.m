// Test to make sure the indexer doesn't crash with subscript syntax.

@interface Box
- (id)objectForKeyedSubscript:(id)key;
- (void)setObject:(id)value forKeyedSubscript:(id)key;
@end

@implementation Box
- (id)objectForKeyedSubscript:(id)key {
  return @"a value";
}

- (void)setObject:(id)value forKeyedSubscript:(id)key {
  // do nothing
}
@end

int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  id v = box[@"hi"];
  //- @k defines/binding KVar
  id k = @"hi";
  //- @k ref KVar
  id v2 = box[k];

  return 0;
}
