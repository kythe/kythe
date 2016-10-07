#import "base.h"

@implementation Base

@synthesize field = _field;

-(instancetype)init {
  self->_field = 5;
  return self;
}

-(int) baseFoo {
  return _field;
}

-(int) baseBar:(int)arg1 withF:(int)f {
  return arg1 * f * _field;
}
@end
