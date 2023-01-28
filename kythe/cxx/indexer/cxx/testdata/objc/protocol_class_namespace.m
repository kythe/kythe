// Test that we correctly support the case where a protocol has the same name
// as a class.

//- @Clock defines/binding ClockProtoDecl
//- ClockProtoDecl.node/kind interface
@protocol Clock

//- @getTime defines/binding GetTimeDecl
//- GetTimeDecl childof ClockProtoDecl
-(int)getTime;

@end

//- @Clock defines/binding ClockClassDecl
@interface Clock

//- @wind defines/binding WindDecl
//- WindDecl childof ClockClassDecl
-(int)wind;

@end

//- @Clock defines/binding ClockClassImpl
@implementation Clock

//- @wind defines/binding WindClockImpl
//- WindClockImpl childof ClockClassImpl
//- WindDecl completedby WindClockImpl
-(int)wind {
  return 2;
}

@end

//- @OldClock defines/binding OldClockDecl
//- OldClockDecl extends ClockClassImpl
//- OldClockDecl extends ClockProtoDecl
@interface OldClock : Clock<Clock>
@end

//- @OldClock defines/binding OldClockImpl
@implementation OldClock

//- @getTime defines/binding GetTimeOldClockImpl
//- GetTimeOldClockImpl childof OldClockImpl
//- GetTimeOldClockImpl overrides GetTimeDecl
-(int)getTime {
  return 1009;
}

//- @wind defines/binding WindOldClockImpl
//- WindOldClockImpl childof OldClockImpl
//- WindOldClockImpl overrides WindDecl
-(int)wind {
  return 4;
}
@end

int main(int argc, char **argv) {
  OldClock *c = [[OldClock alloc] init];

  //- @"[c wind]" ref/call WindImpl
  //- @wind ref WindImpl
  [c wind];

  //- @"[c getTime]" ref/call GetTimeImpl
  //- @getTime ref GetTimeImpl
  [c getTime];
  return 0;
}
