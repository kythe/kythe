import {IFace} from './interface';

//- @Implementation defines/binding Implementation
//- Implementation extends IFace
class Implementation implements IFace {
  //- @ifaceMethod defines/binding ImplementedIFaceMethod
  //- ImplementedIFaceMethod overrides IFaceMethod
  ifaceMethod(): void {}
}
