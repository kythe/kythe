@interface Base {
  int _field;
}

@property int field;

-(instancetype) init;
-(int) baseFoo;
-(int) baseBar:(int) arg1 withF:(int) f;

@end
