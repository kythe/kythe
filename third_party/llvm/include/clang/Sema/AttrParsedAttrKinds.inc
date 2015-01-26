/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Attribute name matcher                                                      *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

static AttributeList::Kind getAttrKind(StringRef Name, AttributeList::Syntax Syntax) {
  if (AttributeList::AS_GNU == Syntax) {
  switch (Name.size()) {
  default: break;
  case 3:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case 'h':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ot", 2))
        break;
      return AttributeList::AT_Hot;	 // "hot"
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "cs", 2))
        break;
      return AttributeList::AT_Pcs;	 // "pcs"
    }
    break;
  case 4:	 // 6 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "old", 3))
        break;
      return AttributeList::AT_Cold;	 // "cold"
    case 'h':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ost", 3))
        break;
      return AttributeList::AT_CUDAHost;	 // "host"
    case 'm':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ode", 3))
        break;
      return AttributeList::AT_Mode;	 // "mode"
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ure", 3))
        break;
      return AttributeList::AT_Pure;	 // "pure"
    case 'u':	 // 1 string to match.
      if (memcmp(Name.data()+1, "sed", 3))
        break;
      return AttributeList::AT_Used;	 // "used"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eak", 3))
        break;
      return AttributeList::AT_Weak;	 // "weak"
    }
    break;
  case 5:	 // 4 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lias", 4))
        break;
      return AttributeList::AT_Alias;	 // "alias"
    case 'c':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'd':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ecl", 3))
          break;
        return AttributeList::AT_CDecl;	 // "cdecl"
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+2, "nst", 3))
          break;
        return AttributeList::AT_Const;	 // "const"
      }
      break;
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "aked", 4))
        break;
      return AttributeList::AT_Naked;	 // "naked"
    }
    break;
  case 6:	 // 12 strings to match.
    switch (Name[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(Name.data()+1, "locks", 5))
        break;
      return AttributeList::AT_Blocks;	 // "blocks"
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ommon", 5))
        break;
      return AttributeList::AT_Common;	 // "common"
    case 'd':	 // 1 string to match.
      if (memcmp(Name.data()+1, "evice", 5))
        break;
      return AttributeList::AT_CUDADevice;	 // "device"
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ormat", 5))
        break;
      return AttributeList::AT_Format;	 // "format"
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lobal", 5))
        break;
      return AttributeList::AT_CUDAGlobal;	 // "global"
    case 'm':	 // 3 strings to match.
      switch (Name[1]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+2, "lloc", 4))
          break;
        return AttributeList::AT_Malloc;	 // "malloc"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ps16", 4))
          break;
        return AttributeList::AT_Mips16;	 // "mips16"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_abi", 4))
          break;
        return AttributeList::AT_MSABI;	 // "ms_abi"
      }
      break;
    case 'p':	 // 2 strings to match.
      if (Name[1] != 'a')
        break;
      switch (Name[2]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ked", 3))
          break;
        return AttributeList::AT_Packed;	 // "packed"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+3, "cal", 3))
          break;
        return AttributeList::AT_Pascal;	 // "pascal"
      }
      break;
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "hared", 5))
        break;
      return AttributeList::AT_CUDAShared;	 // "shared"
    case 'u':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nused", 5))
        break;
      return AttributeList::AT_Unused;	 // "unused"
    }
    break;
  case 7:	 // 15 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 1 string to match.
      if (memcmp(Name.data()+1, "_const", 6))
        break;
      return AttributeList::AT_Const;	 // "__const"
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ligned", 6))
        break;
      return AttributeList::AT_Aligned;	 // "aligned"
    case 'b':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ounded", 6))
        break;
      return AttributeList::IgnoredAttribute;	 // "bounded"
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "leanup", 6))
        break;
      return AttributeList::AT_Cleanup;	 // "cleanup"
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+1, "latten", 6))
        break;
      return AttributeList::AT_Flatten;	 // "flatten"
    case 'm':	 // 1 string to match.
      if (memcmp(Name.data()+1, "insize", 6))
        break;
      return AttributeList::AT_MinSize;	 // "minsize"
    case 'n':	 // 3 strings to match.
      if (Name[1] != 'o')
        break;
      switch (Name[2]) {
      default: break;
      case 'd':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ebug", 4))
          break;
        return AttributeList::AT_NoDebug;	 // "nodebug"
      case 'n':	 // 1 string to match.
        if (memcmp(Name.data()+3, "null", 4))
          break;
        return AttributeList::AT_NonNull;	 // "nonnull"
      case 't':	 // 1 string to match.
        if (memcmp(Name.data()+3, "hrow", 4))
          break;
        return AttributeList::AT_NoThrow;	 // "nothrow"
      }
      break;
    case 'o':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'b':	 // 1 string to match.
        if (memcmp(Name.data()+2, "jc_gc", 5))
          break;
        return AttributeList::AT_ObjCGC;	 // "objc_gc"
      case 'p':	 // 1 string to match.
        if (memcmp(Name.data()+2, "tnone", 5))
          break;
        return AttributeList::AT_OptimizeNone;	 // "optnone"
      }
      break;
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "egparm", 6))
        break;
      return AttributeList::AT_Regparm;	 // "regparm"
    case 's':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ction", 5))
          break;
        return AttributeList::AT_Section;	 // "section"
      case 't':	 // 1 string to match.
        if (memcmp(Name.data()+2, "dcall", 5))
          break;
        return AttributeList::AT_StdCall;	 // "stdcall"
      }
      break;
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eakref", 6))
        break;
      return AttributeList::AT_WeakRef;	 // "weakref"
    }
    break;
  case 8:	 // 14 strings to match.
    switch (Name[0]) {
    default: break;
    case 'N':	 // 1 string to match.
      if (memcmp(Name.data()+1, "SObject", 7))
        break;
      return AttributeList::AT_ObjCNSObject;	 // "NSObject"
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nnotate", 7))
        break;
      return AttributeList::AT_Annotate;	 // "annotate"
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "onstant", 7))
        break;
      return AttributeList::AT_CUDAConstant;	 // "constant"
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+1, "astcall", 7))
        break;
      return AttributeList::AT_FastCall;	 // "fastcall"
    case 'i':	 // 2 strings to match.
      if (Name[1] != 'b')
        break;
      switch (Name[2]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ction", 5))
          break;
        return AttributeList::AT_IBAction;	 // "ibaction"
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+3, "utlet", 5))
          break;
        return AttributeList::AT_IBOutlet;	 // "iboutlet"
      }
      break;
    case 'l':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ockable", 7))
        break;
      return AttributeList::AT_Lockable;	 // "lockable"
    case 'n':	 // 4 strings to match.
      if (Name[1] != 'o')
        break;
      switch (Name[2]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ommon", 5))
          break;
        return AttributeList::AT_NoCommon;	 // "nocommon"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+3, "nline", 5))
          break;
        return AttributeList::AT_NoInline;	 // "noinline"
      case 'm':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ips16", 5))
          break;
        return AttributeList::AT_NoMips16;	 // "nomips16"
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+3, "eturn", 5))
          break;
        return AttributeList::AT_NoReturn;	 // "noreturn"
      }
      break;
    case 's':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ntinel", 6))
          break;
        return AttributeList::AT_Sentinel;	 // "sentinel"
      case 'y':	 // 1 string to match.
        if (memcmp(Name.data()+2, "sv_abi", 6))
          break;
        return AttributeList::AT_SysVABI;	 // "sysv_abi"
      }
      break;
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "hiscall", 7))
        break;
      return AttributeList::AT_ThisCall;	 // "thiscall"
    }
    break;
  case 9:	 // 9 strings to match.
    switch (Name[0]) {
    default: break;
    case 'd':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "ll", 2))
        break;
      switch (Name[3]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+4, "xport", 5))
          break;
        return AttributeList::AT_DLLExport;	 // "dllexport"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+4, "mport", 5))
          break;
        return AttributeList::AT_DLLImport;	 // "dllimport"
      }
      break;
    case 'e':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nable_if", 8))
        break;
      return AttributeList::AT_EnableIf;	 // "enable_if"
    case 'i':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nterrupt", 8))
        break;
      return AttributeList::AT_Interrupt;	 // "interrupt"
    case 'm':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+2, "y_alias", 7))
          break;
        return AttributeList::AT_MayAlias;	 // "may_alias"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_struct", 7))
          break;
        return AttributeList::AT_MsStruct;	 // "ms_struct"
      }
      break;
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "naclcall", 8))
        break;
      return AttributeList::AT_PnaclCall;	 // "pnaclcall"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ls_model", 8))
        break;
      return AttributeList::AT_TLSModel;	 // "tls_model"
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ecreturn", 8))
        break;
      return AttributeList::AT_VecReturn;	 // "vecreturn"
    }
    break;
  case 10:	 // 9 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+2, "pability", 8))
          break;
        return AttributeList::AT_Capability;	 // "capability"
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+2, "nsumable", 8))
          break;
        return AttributeList::AT_Consumable;	 // "consumable"
      }
      break;
    case 'd':	 // 2 strings to match.
      if (Name[1] != 'e')
        break;
      switch (Name[2]) {
      default: break;
      case 'p':	 // 1 string to match.
        if (memcmp(Name.data()+3, "recated", 7))
          break;
        return AttributeList::AT_Deprecated;	 // "deprecated"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+3, "tructor", 7))
          break;
        return AttributeList::AT_Destructor;	 // "destructor"
      }
      break;
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ormat_arg", 9))
        break;
      return AttributeList::AT_FormatArg;	 // "format_arg"
    case 'g':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'n':	 // 1 string to match.
        if (memcmp(Name.data()+2, "u_inline", 8))
          break;
        return AttributeList::AT_GNUInline;	 // "gnu_inline"
      case 'u':	 // 1 string to match.
        if (memcmp(Name.data()+2, "arded_by", 8))
          break;
        return AttributeList::AT_GuardedBy;	 // "guarded_by"
      }
      break;
    case 'v':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ctorcall", 8))
          break;
        return AttributeList::AT_VectorCall;	 // "vectorcall"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+2, "sibility", 8))
          break;
        return AttributeList::AT_Visibility;	 // "visibility"
      }
      break;
    }
    break;
  case 11:	 // 11 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lign_value", 10))
        break;
      return AttributeList::AT_AlignValue;	 // "align_value"
    case 'c':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'f':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_consumed", 9))
          break;
        return AttributeList::AT_CFConsumed;	 // "cf_consumed"
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+2, "nstructor", 9))
          break;
        return AttributeList::AT_Constructor;	 // "constructor"
      }
      break;
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "uarded_var", 10))
        break;
      return AttributeList::AT_GuardedVar;	 // "guarded_var"
    case 'n':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+2, "duplicate", 9))
          break;
        return AttributeList::AT_NoDuplicate;	 // "noduplicate"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_consumed", 9))
          break;
        return AttributeList::AT_NSConsumed;	 // "ns_consumed"
      }
      break;
    case 'o':	 // 1 string to match.
      if (memcmp(Name.data()+1, "bjc_bridge", 10))
        break;
      return AttributeList::AT_ObjCBridge;	 // "objc_bridge"
    case 'u':	 // 1 string to match.
      if (memcmp(Name.data()+1, "navailable", 10))
        break;
      return AttributeList::AT_Unavailable;	 // "unavailable"
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ector_size", 10))
        break;
      return AttributeList::AT_VectorSize;	 // "vector_size"
    case 'w':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+2, "rn_unused", 9))
          break;
        return AttributeList::AT_WarnUnused;	 // "warn_unused"
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ak_import", 9))
          break;
        return AttributeList::AT_WeakImport;	 // "weak_import"
      }
      break;
    }
    break;
  case 12:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "vailability", 11))
        break;
      return AttributeList::AT_Availability;	 // "availability"
    case 'o':	 // 1 string to match.
      if (memcmp(Name.data()+1, "verloadable", 11))
        break;
      return AttributeList::AT_Overloadable;	 // "overloadable"
    }
    break;
  case 13:	 // 10 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'd':	 // 1 string to match.
        if (memcmp(Name.data()+2, "dress_space", 11))
          break;
        return AttributeList::AT_AddressSpace;	 // "address_space"
      case 'l':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ways_inline", 11))
          break;
        return AttributeList::AT_AlwaysInline;	 // "always_inline"
      }
      break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "allable_when", 12))
        break;
      return AttributeList::AT_CallableWhen;	 // "callable_when"
    case 'i':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nit_priority", 12))
        break;
      return AttributeList::AT_InitPriority;	 // "init_priority"
    case 'l':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+2, "unch_bounds", 11))
          break;
        return AttributeList::AT_CUDALaunchBounds;	 // "launch_bounds"
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ck_returned", 11))
          break;
        return AttributeList::AT_LockReturned;	 // "lock_returned"
      }
      break;
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "t_guarded_by", 12))
        break;
      return AttributeList::AT_PtGuardedBy;	 // "pt_guarded_by"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eturns_twice", 12))
        break;
      return AttributeList::AT_ReturnsTwice;	 // "returns_twice"
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "et_typestate", 12))
        break;
      return AttributeList::AT_SetTypestate;	 // "set_typestate"
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ec_type_hint", 12))
        break;
      return AttributeList::AT_VecTypeHint;	 // "vec_type_hint"
    }
    break;
  case 14:	 // 9 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+2, "quired_after", 12))
          break;
        return AttributeList::AT_AcquiredAfter;	 // "acquired_after"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "sume_aligned", 12))
          break;
        return AttributeList::AT_AssumeAligned;	 // "assume_aligned"
      }
      break;
    case 'i':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ntel_ocl_bicc", 13))
        break;
      return AttributeList::AT_IntelOclBicc;	 // "intel_ocl_bicc"
    case 'l':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ocks_excluded", 13))
        break;
      return AttributeList::AT_LocksExcluded;	 // "locks_excluded"
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "o_split_stack", 13))
        break;
      return AttributeList::AT_NoSplitStack;	 // "no_split_stack"
    case 'o':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "bjc_", 4))
        break;
      switch (Name[5]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+6, "xception", 8))
          break;
        return AttributeList::AT_ObjCException;	 // "objc_exception"
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+6, "wnership", 8))
          break;
        return AttributeList::AT_ObjCOwnership;	 // "objc_ownership"
      }
      break;
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "t_guarded_var", 13))
        break;
      return AttributeList::AT_PtGuardedVar;	 // "pt_guarded_var"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "est_typestate", 13))
        break;
      return AttributeList::AT_TestTypestate;	 // "test_typestate"
    }
    break;
  case 15:	 // 10 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "cquired_before", 14))
        break;
      return AttributeList::AT_AcquiredBefore;	 // "acquired_before"
    case 'e':	 // 1 string to match.
      if (memcmp(Name.data()+1, "xt_vector_type", 14))
        break;
      return AttributeList::AT_ExtVectorType;	 // "ext_vector_type"
    case 'o':	 // 3 strings to match.
      switch (Name[1]) {
      default: break;
      case 'b':	 // 1 string to match.
        if (memcmp(Name.data()+2, "jc_root_class", 13))
          break;
        return AttributeList::AT_ObjCRootClass;	 // "objc_root_class"
      case 'w':	 // 2 strings to match.
        if (memcmp(Name.data()+2, "nership_", 8))
          break;
        switch (Name[10]) {
        default: break;
        case 'h':	 // 1 string to match.
          if (memcmp(Name.data()+11, "olds", 4))
            break;
          return AttributeList::AT_Ownership;	 // "ownership_holds"
        case 't':	 // 1 string to match.
          if (memcmp(Name.data()+11, "akes", 4))
            break;
          return AttributeList::AT_Ownership;	 // "ownership_takes"
        }
        break;
      }
      break;
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "aram_typestate", 14))
        break;
      return AttributeList::AT_ParamTypestate;	 // "param_typestate"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eturns_nonnull", 14))
        break;
      return AttributeList::AT_ReturnsNonNull;	 // "returns_nonnull"
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "coped_lockable", 14))
        break;
      return AttributeList::AT_ScopedLockable;	 // "scoped_lockable"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ype_visibility", 14))
        break;
      return AttributeList::AT_TypeVisibility;	 // "type_visibility"
    case 'u':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nlock_function", 14))
        break;
      return AttributeList::AT_ReleaseCapability;	 // "unlock_function"
    }
    break;
  case 16:	 // 3 strings to match.
    switch (Name[0]) {
    default: break;
    case 'n':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+2, "on_vector_type", 14))
          break;
        return AttributeList::AT_NeonVectorType;	 // "neon_vector_type"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_consumes_self", 14))
          break;
        return AttributeList::AT_NSConsumesSelf;	 // "ns_consumes_self"
      }
      break;
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eturn_typestate", 15))
        break;
      return AttributeList::AT_ReturnTypestate;	 // "return_typestate"
    }
    break;
  case 17:	 // 6 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'n':	 // 1 string to match.
        if (memcmp(Name.data()+2, "alyzer_noreturn", 15))
          break;
        return AttributeList::AT_AnalyzerNoReturn;	 // "analyzer_noreturn"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "sert_capability", 15))
          break;
        return AttributeList::AT_AssertCapability;	 // "assert_capability"
      }
      break;
    case 'o':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'b':	 // 1 string to match.
        if (memcmp(Name.data()+2, "jc_runtime_name", 15))
          break;
        return AttributeList::AT_ObjCRuntimeName;	 // "objc_runtime_name"
      case 'w':	 // 1 string to match.
        if (memcmp(Name.data()+2, "nership_returns", 15))
          break;
        return AttributeList::AT_Ownership;	 // "ownership_returns"
      }
      break;
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "hared_capability", 16))
        break;
      return AttributeList::AT_Capability;	 // "shared_capability"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ransparent_union", 16))
        break;
      return AttributeList::AT_TransparentUnion;	 // "transparent_union"
    }
    break;
  case 18:	 // 9 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+2, "quire_capability", 16))
          break;
        return AttributeList::AT_AcquireCapability;	 // "acquire_capability"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "sert_shared_lock", 16))
          break;
        return AttributeList::AT_AssertSharedLock;	 // "assert_shared_lock"
      }
      break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "arries_dependency", 17))
        break;
      return AttributeList::AT_CarriesDependency;	 // "carries_dependency"
    case 'i':	 // 1 string to match.
      if (memcmp(Name.data()+1, "boutletcollection", 17))
        break;
      return AttributeList::AT_IBOutletCollection;	 // "iboutletcollection"
    case 'n':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "o_sanitize_", 11))
        break;
      switch (Name[12]) {
      default: break;
      case 'm':	 // 1 string to match.
        if (memcmp(Name.data()+13, "emory", 5))
          break;
        return AttributeList::AT_NoSanitizeMemory;	 // "no_sanitize_memory"
      case 't':	 // 1 string to match.
        if (memcmp(Name.data()+13, "hread", 5))
          break;
        return AttributeList::AT_NoSanitizeThread;	 // "no_sanitize_thread"
      }
      break;
    case 'o':	 // 1 string to match.
      if (memcmp(Name.data()+1, "bjc_method_family", 17))
        break;
      return AttributeList::AT_ObjCMethodFamily;	 // "objc_method_family"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "elease_capability", 17))
        break;
      return AttributeList::AT_ReleaseCapability;	 // "release_capability"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+1, "arn_unused_result", 17))
        break;
      return AttributeList::AT_WarnUnusedResult;	 // "warn_unused_result"
    }
    break;
  case 19:	 // 9 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 3 strings to match.
      if (memcmp(Name.data()+1, "f_", 2))
        break;
      switch (Name[3]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+4, "udited_transfer", 15))
          break;
        return AttributeList::AT_CFAuditedTransfer;	 // "cf_audited_transfer"
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+4, "eturns_retained", 15))
          break;
        return AttributeList::AT_CFReturnsRetained;	 // "cf_returns_retained"
      case 'u':	 // 1 string to match.
        if (memcmp(Name.data()+4, "nknown_transfer", 15))
          break;
        return AttributeList::AT_CFUnknownTransfer;	 // "cf_unknown_transfer"
      }
      break;
    case 'n':	 // 2 strings to match.
      switch (Name[1]) {
      default: break;
      case 'o':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_sanitize_address", 17))
          break;
        return AttributeList::AT_NoSanitizeAddress;	 // "no_sanitize_address"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "_returns_retained", 17))
          break;
        return AttributeList::AT_NSReturnsRetained;	 // "ns_returns_retained"
      }
      break;
    case 'o':	 // 3 strings to match.
      if (memcmp(Name.data()+1, "bjc_", 4))
        break;
      switch (Name[5]) {
      default: break;
      case 'b':	 // 2 strings to match.
        if (memcmp(Name.data()+6, "ridge_", 6))
          break;
        switch (Name[12]) {
        default: break;
        case 'm':	 // 1 string to match.
          if (memcmp(Name.data()+13, "utable", 6))
            break;
          return AttributeList::AT_ObjCBridgeMutable;	 // "objc_bridge_mutable"
        case 'r':	 // 1 string to match.
          if (memcmp(Name.data()+13, "elated", 6))
            break;
          return AttributeList::AT_ObjCBridgeRelated;	 // "objc_bridge_related"
        }
        break;
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+6, "equires_super", 13))
          break;
        return AttributeList::AT_ObjCRequiresSuper;	 // "objc_requires_super"
      }
      break;
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "equires_capability", 18))
        break;
      return AttributeList::AT_RequiresCapability;	 // "requires_capability"
    }
    break;
  case 20:	 // 4 strings to match.
    switch (Name[0]) {
    default: break;
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eon_polyvector_type", 19))
        break;
      return AttributeList::AT_NeonPolyVectorType;	 // "neon_polyvector_type"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "eqd_work_group_size", 19))
        break;
      return AttributeList::AT_ReqdWorkGroupSize;	 // "reqd_work_group_size"
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "hared_lock_function", 19))
        break;
      return AttributeList::AT_AcquireCapability;	 // "shared_lock_function"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ork_group_size_hint", 19))
        break;
      return AttributeList::AT_WorkGroupSizeHint;	 // "work_group_size_hint"
    }
    break;
  case 21:	 // 5 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ssert_exclusive_lock", 20))
        break;
      return AttributeList::AT_AssertExclusiveLock;	 // "assert_exclusive_lock"
    case 'o':	 // 1 string to match.
      if (memcmp(Name.data()+1, "bjc_precise_lifetime", 20))
        break;
      return AttributeList::AT_ObjCPreciseLifetime;	 // "objc_precise_lifetime"
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ointer_with_type_tag", 20))
        break;
      return AttributeList::AT_ArgumentWithTypeTag;	 // "pointer_with_type_tag"
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "hared_locks_required", 20))
        break;
      return AttributeList::AT_RequiresCapability;	 // "shared_locks_required"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ype_tag_for_datatype", 20))
        break;
      return AttributeList::AT_TypeTagForDatatype;	 // "type_tag_for_datatype"
    }
    break;
  case 22:	 // 3 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "rgument_with_type_tag", 21))
        break;
      return AttributeList::AT_ArgumentWithTypeTag;	 // "argument_with_type_tag"
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "o_instrument_function", 21))
        break;
      return AttributeList::AT_NoInstrumentFunction;	 // "no_instrument_function"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ry_acquire_capability", 21))
        break;
      return AttributeList::AT_TryAcquireCapability;	 // "try_acquire_capability"
    }
    break;
  case 23:	 // 6 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "f_returns_not_retained", 22))
        break;
      return AttributeList::AT_CFReturnsNotRetained;	 // "cf_returns_not_retained"
    case 'e':	 // 1 string to match.
      if (memcmp(Name.data()+1, "xclusive_lock_function", 22))
        break;
      return AttributeList::AT_AcquireCapability;	 // "exclusive_lock_function"
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+1, "orce_align_arg_pointer", 22))
        break;
      return AttributeList::AT_X86ForceAlignArgPointer;	 // "force_align_arg_pointer"
    case 'n':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "s_returns_", 10))
        break;
      switch (Name[11]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+12, "utoreleased", 11))
          break;
        return AttributeList::AT_NSReturnsAutoreleased;	 // "ns_returns_autoreleased"
      case 'n':	 // 1 string to match.
        if (memcmp(Name.data()+12, "ot_retained", 11))
          break;
        return AttributeList::AT_NSReturnsNotRetained;	 // "ns_returns_not_retained"
      }
      break;
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "hared_trylock_function", 22))
        break;
      return AttributeList::AT_SharedTrylockFunction;	 // "shared_trylock_function"
    }
    break;
  case 24:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ssert_shared_capability", 23))
        break;
      return AttributeList::AT_AssertCapability;	 // "assert_shared_capability"
    case 'e':	 // 1 string to match.
      if (memcmp(Name.data()+1, "xclusive_locks_required", 23))
        break;
      return AttributeList::AT_RequiresCapability;	 // "exclusive_locks_required"
    }
    break;
  case 25:	 // 3 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "cquire_shared_capability", 24))
        break;
      return AttributeList::AT_AcquireCapability;	 // "acquire_shared_capability"
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "o_thread_safety_analysis", 24))
        break;
      return AttributeList::AT_NoThreadSafetyAnalysis;	 // "no_thread_safety_analysis"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "elease_shared_capability", 24))
        break;
      return AttributeList::AT_ReleaseCapability;	 // "release_shared_capability"
    }
    break;
  case 26:	 // 6 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "onsumable_auto_cast_state", 25))
        break;
      return AttributeList::AT_ConsumableAutoCast;	 // "consumable_auto_cast_state"
    case 'e':	 // 1 string to match.
      if (memcmp(Name.data()+1, "xclusive_trylock_function", 25))
        break;
      return AttributeList::AT_ExclusiveTrylockFunction;	 // "exclusive_trylock_function"
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "o_address_safety_analysis", 25))
        break;
      return AttributeList::AT_NoSanitizeAddress;	 // "no_address_safety_analysis"
    case 'o':	 // 1 string to match.
      if (memcmp(Name.data()+1, "bjc_returns_inner_pointer", 25))
        break;
      return AttributeList::AT_ObjCReturnsInnerPointer;	 // "objc_returns_inner_pointer"
    case 'r':	 // 2 strings to match.
      if (Name[1] != 'e')
        break;
      switch (Name[2]) {
      default: break;
      case 'l':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ease_generic_capability", 23))
          break;
        return AttributeList::AT_ReleaseCapability;	 // "release_generic_capability"
      case 'q':	 // 1 string to match.
        if (memcmp(Name.data()+3, "uires_shared_capability", 23))
          break;
        return AttributeList::AT_RequiresCapability;	 // "requires_shared_capability"
      }
      break;
    }
    break;
  case 27:	 // 1 string to match.
    if (memcmp(Name.data()+0, "objc_designated_initializer", 27))
      break;
    return AttributeList::AT_ObjCDesignatedInitializer;	 // "objc_designated_initializer"
  case 28:	 // 1 string to match.
    if (memcmp(Name.data()+0, "consumable_set_state_on_read", 28))
      break;
    return AttributeList::AT_ConsumableSetOnRead;	 // "consumable_set_state_on_read"
  case 29:	 // 1 string to match.
    if (memcmp(Name.data()+0, "try_acquire_shared_capability", 29))
      break;
    return AttributeList::AT_TryAcquireCapability;	 // "try_acquire_shared_capability"
  case 34:	 // 1 string to match.
    if (memcmp(Name.data()+0, "objc_requires_property_definitions", 34))
      break;
    return AttributeList::AT_ObjCRequiresPropertyDefs;	 // "objc_requires_property_definitions"
  case 35:	 // 1 string to match.
    if (memcmp(Name.data()+0, "objc_arc_weak_reference_unavailable", 35))
      break;
    return AttributeList::AT_ArcWeakrefUnavailable;	 // "objc_arc_weak_reference_unavailable"
  case 46:	 // 1 string to match.
    if (memcmp(Name.data()+0, "objc_protocol_requires_explicit_implementation", 46))
      break;
    return AttributeList::AT_ObjCExplicitProtocolImpl;	 // "objc_protocol_requires_explicit_implementation"
  }
  } else if (AttributeList::AS_Declspec == Syntax) {
  switch (Name.size()) {
  default: break;
  case 4:	 // 1 string to match.
    if (memcmp(Name.data()+0, "uuid", 4))
      break;
    return AttributeList::AT_Uuid;	 // "uuid"
  case 5:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lign", 4))
        break;
      return AttributeList::AT_Aligned;	 // "align"
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+1, "aked", 4))
        break;
      return AttributeList::AT_Naked;	 // "naked"
    }
    break;
  case 6:	 // 1 string to match.
    if (memcmp(Name.data()+0, "thread", 6))
      break;
    return AttributeList::AT_Thread;	 // "thread"
  case 7:	 // 1 string to match.
    if (memcmp(Name.data()+0, "nothrow", 7))
      break;
    return AttributeList::AT_NoThrow;	 // "nothrow"
  case 8:	 // 4 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "llocate", 7))
        break;
      return AttributeList::AT_Section;	 // "allocate"
    case 'n':	 // 2 strings to match.
      if (Name[1] != 'o')
        break;
      switch (Name[2]) {
      default: break;
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+3, "nline", 5))
          break;
        return AttributeList::AT_NoInline;	 // "noinline"
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+3, "eturn", 5))
          break;
        return AttributeList::AT_NoReturn;	 // "noreturn"
      }
      break;
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "roperty", 7))
        break;
      return AttributeList::IgnoredAttribute;	 // "property"
    }
    break;
  case 9:	 // 3 strings to match.
    switch (Name[0]) {
    default: break;
    case 'd':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "ll", 2))
        break;
      switch (Name[3]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+4, "xport", 5))
          break;
        return AttributeList::AT_DLLExport;	 // "dllexport"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+4, "mport", 5))
          break;
        return AttributeList::AT_DLLImport;	 // "dllimport"
      }
      break;
    case 's':	 // 1 string to match.
      if (memcmp(Name.data()+1, "electany", 8))
        break;
      return AttributeList::AT_SelectAny;	 // "selectany"
    }
    break;
  case 10:	 // 1 string to match.
    if (memcmp(Name.data()+0, "deprecated", 10))
      break;
    return AttributeList::AT_Deprecated;	 // "deprecated"
  }
  } else if (AttributeList::AS_CXX11 == Syntax) {
  switch (Name.size()) {
  default: break;
  case 8:	 // 2 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'h':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ot", 2))
        break;
      return AttributeList::AT_Hot;	 // "gnu::hot"
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+6, "cs", 2))
        break;
      return AttributeList::AT_Pcs;	 // "gnu::pcs"
    }
    break;
  case 9:	 // 5 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+6, "old", 3))
        break;
      return AttributeList::AT_Cold;	 // "gnu::cold"
    case 'm':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ode", 3))
        break;
      return AttributeList::AT_Mode;	 // "gnu::mode"
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ure", 3))
        break;
      return AttributeList::AT_Pure;	 // "gnu::pure"
    case 'u':	 // 1 string to match.
      if (memcmp(Name.data()+6, "sed", 3))
        break;
      return AttributeList::AT_Used;	 // "gnu::used"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+6, "eak", 3))
        break;
      return AttributeList::AT_Weak;	 // "gnu::weak"
    }
    break;
  case 10:	 // 5 strings to match.
    switch (Name[0]) {
    default: break;
    case ':':	 // 1 string to match.
      if (memcmp(Name.data()+1, ":noreturn", 9))
        break;
      return AttributeList::AT_CXX11NoReturn;	 // "::noreturn"
    case 'g':	 // 4 strings to match.
      if (memcmp(Name.data()+1, "nu::", 4))
        break;
      switch (Name[5]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+6, "lias", 4))
          break;
        return AttributeList::AT_Alias;	 // "gnu::alias"
      case 'c':	 // 2 strings to match.
        switch (Name[6]) {
        default: break;
        case 'd':	 // 1 string to match.
          if (memcmp(Name.data()+7, "ecl", 3))
            break;
          return AttributeList::AT_CDecl;	 // "gnu::cdecl"
        case 'o':	 // 1 string to match.
          if (memcmp(Name.data()+7, "nst", 3))
            break;
          return AttributeList::AT_Const;	 // "gnu::const"
        }
        break;
      case 'n':	 // 1 string to match.
        if (memcmp(Name.data()+6, "aked", 4))
          break;
        return AttributeList::AT_Naked;	 // "gnu::naked"
      }
      break;
    }
    break;
  case 11:	 // 7 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ommon", 5))
        break;
      return AttributeList::AT_Common;	 // "gnu::common"
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ormat", 5))
        break;
      return AttributeList::AT_Format;	 // "gnu::format"
    case 'm':	 // 3 strings to match.
      switch (Name[6]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+7, "lloc", 4))
          break;
        return AttributeList::AT_Malloc;	 // "gnu::malloc"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+7, "ps16", 4))
          break;
        return AttributeList::AT_Mips16;	 // "gnu::mips16"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+7, "_abi", 4))
          break;
        return AttributeList::AT_MSABI;	 // "gnu::ms_abi"
      }
      break;
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+6, "acked", 5))
        break;
      return AttributeList::AT_Packed;	 // "gnu::packed"
    case 'u':	 // 1 string to match.
      if (memcmp(Name.data()+6, "nused", 5))
        break;
      return AttributeList::AT_Unused;	 // "gnu::unused"
    }
    break;
  case 12:	 // 12 strings to match.
    switch (Name[0]) {
    default: break;
    case ':':	 // 1 string to match.
      if (memcmp(Name.data()+1, ":deprecated", 11))
        break;
      return AttributeList::AT_Deprecated;	 // "::deprecated"
    case 'g':	 // 11 strings to match.
      if (memcmp(Name.data()+1, "nu::", 4))
        break;
      switch (Name[5]) {
      default: break;
      case '_':	 // 1 string to match.
        if (memcmp(Name.data()+6, "_const", 6))
          break;
        return AttributeList::AT_Const;	 // "gnu::__const"
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+6, "ligned", 6))
          break;
        return AttributeList::AT_Aligned;	 // "gnu::aligned"
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+6, "leanup", 6))
          break;
        return AttributeList::AT_Cleanup;	 // "gnu::cleanup"
      case 'f':	 // 1 string to match.
        if (memcmp(Name.data()+6, "latten", 6))
          break;
        return AttributeList::AT_Flatten;	 // "gnu::flatten"
      case 'n':	 // 3 strings to match.
        if (Name[6] != 'o')
          break;
        switch (Name[7]) {
        default: break;
        case 'd':	 // 1 string to match.
          if (memcmp(Name.data()+8, "ebug", 4))
            break;
          return AttributeList::AT_NoDebug;	 // "gnu::nodebug"
        case 'n':	 // 1 string to match.
          if (memcmp(Name.data()+8, "null", 4))
            break;
          return AttributeList::AT_NonNull;	 // "gnu::nonnull"
        case 't':	 // 1 string to match.
          if (memcmp(Name.data()+8, "hrow", 4))
            break;
          return AttributeList::AT_NoThrow;	 // "gnu::nothrow"
        }
        break;
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+6, "egparm", 6))
          break;
        return AttributeList::AT_Regparm;	 // "gnu::regparm"
      case 's':	 // 2 strings to match.
        switch (Name[6]) {
        default: break;
        case 'e':	 // 1 string to match.
          if (memcmp(Name.data()+7, "ction", 5))
            break;
          return AttributeList::AT_Section;	 // "gnu::section"
        case 't':	 // 1 string to match.
          if (memcmp(Name.data()+7, "dcall", 5))
            break;
          return AttributeList::AT_StdCall;	 // "gnu::stdcall"
        }
        break;
      case 'w':	 // 1 string to match.
        if (memcmp(Name.data()+6, "eakref", 6))
          break;
        return AttributeList::AT_WeakRef;	 // "gnu::weakref"
      }
      break;
    }
    break;
  case 13:	 // 8 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+6, "astcall", 7))
        break;
      return AttributeList::AT_FastCall;	 // "gnu::fastcall"
    case 'n':	 // 4 strings to match.
      if (Name[6] != 'o')
        break;
      switch (Name[7]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+8, "ommon", 5))
          break;
        return AttributeList::AT_NoCommon;	 // "gnu::nocommon"
      case 'i':	 // 1 string to match.
        if (memcmp(Name.data()+8, "nline", 5))
          break;
        return AttributeList::AT_NoInline;	 // "gnu::noinline"
      case 'm':	 // 1 string to match.
        if (memcmp(Name.data()+8, "ips16", 5))
          break;
        return AttributeList::AT_NoMips16;	 // "gnu::nomips16"
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+8, "eturn", 5))
          break;
        return AttributeList::AT_NoReturn;	 // "gnu::noreturn"
      }
      break;
    case 's':	 // 2 strings to match.
      switch (Name[6]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(Name.data()+7, "ntinel", 6))
          break;
        return AttributeList::AT_Sentinel;	 // "gnu::sentinel"
      case 'y':	 // 1 string to match.
        if (memcmp(Name.data()+7, "sv_abi", 6))
          break;
        return AttributeList::AT_SysVABI;	 // "gnu::sysv_abi"
      }
      break;
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+6, "hiscall", 7))
        break;
      return AttributeList::AT_ThisCall;	 // "gnu::thiscall"
    }
    break;
  case 14:	 // 6 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lang::optnone", 13))
        break;
      return AttributeList::AT_OptimizeNone;	 // "clang::optnone"
    case 'g':	 // 5 strings to match.
      if (memcmp(Name.data()+1, "nu::", 4))
        break;
      switch (Name[5]) {
      default: break;
      case 'd':	 // 2 strings to match.
        if (memcmp(Name.data()+6, "ll", 2))
          break;
        switch (Name[8]) {
        default: break;
        case 'e':	 // 1 string to match.
          if (memcmp(Name.data()+9, "xport", 5))
            break;
          return AttributeList::AT_DLLExport;	 // "gnu::dllexport"
        case 'i':	 // 1 string to match.
          if (memcmp(Name.data()+9, "mport", 5))
            break;
          return AttributeList::AT_DLLImport;	 // "gnu::dllimport"
        }
        break;
      case 'm':	 // 2 strings to match.
        switch (Name[6]) {
        default: break;
        case 'a':	 // 1 string to match.
          if (memcmp(Name.data()+7, "y_alias", 7))
            break;
          return AttributeList::AT_MayAlias;	 // "gnu::may_alias"
        case 's':	 // 1 string to match.
          if (memcmp(Name.data()+7, "_struct", 7))
            break;
          return AttributeList::AT_MsStruct;	 // "gnu::ms_struct"
        }
        break;
      case 't':	 // 1 string to match.
        if (memcmp(Name.data()+6, "ls_model", 8))
          break;
        return AttributeList::AT_TLSModel;	 // "gnu::tls_model"
      }
      break;
    }
    break;
  case 15:	 // 5 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'd':	 // 2 strings to match.
      if (Name[6] != 'e')
        break;
      switch (Name[7]) {
      default: break;
      case 'p':	 // 1 string to match.
        if (memcmp(Name.data()+8, "recated", 7))
          break;
        return AttributeList::AT_Deprecated;	 // "gnu::deprecated"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+8, "tructor", 7))
          break;
        return AttributeList::AT_Destructor;	 // "gnu::destructor"
      }
      break;
    case 'f':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ormat_arg", 9))
        break;
      return AttributeList::AT_FormatArg;	 // "gnu::format_arg"
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+6, "nu_inline", 9))
        break;
      return AttributeList::AT_GNUInline;	 // "gnu::gnu_inline"
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+6, "isibility", 9))
        break;
      return AttributeList::AT_Visibility;	 // "gnu::visibility"
    }
    break;
  case 16:	 // 2 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+6, "onstructor", 10))
        break;
      return AttributeList::AT_Constructor;	 // "gnu::constructor"
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ector_size", 10))
        break;
      return AttributeList::AT_VectorSize;	 // "gnu::vector_size"
    }
    break;
  case 17:	 // 1 string to match.
    if (memcmp(Name.data()+0, "clang::capability", 17))
      break;
    return AttributeList::AT_Capability;	 // "clang::capability"
  case 18:	 // 4 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "lang::", 6))
        break;
      switch (Name[7]) {
      default: break;
      case 'f':	 // 1 string to match.
        if (memcmp(Name.data()+8, "allthrough", 10))
          break;
        return AttributeList::AT_FallThrough;	 // "clang::fallthrough"
      case 'n':	 // 1 string to match.
        if (memcmp(Name.data()+8, "oduplicate", 10))
          break;
        return AttributeList::AT_NoDuplicate;	 // "clang::noduplicate"
      }
      break;
    case 'g':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "nu::", 4))
        break;
      switch (Name[5]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+6, "lways_inline", 12))
          break;
        return AttributeList::AT_AlwaysInline;	 // "gnu::always_inline"
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+6, "eturns_twice", 12))
          break;
        return AttributeList::AT_ReturnsTwice;	 // "gnu::returns_twice"
      }
      break;
    }
    break;
  case 19:	 // 2 strings to match.
    if (memcmp(Name.data()+0, "gnu::", 5))
      break;
    switch (Name[5]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+6, "ssume_aligned", 13))
        break;
      return AttributeList::AT_AssumeAligned;	 // "gnu::assume_aligned"
    case 'n':	 // 1 string to match.
      if (memcmp(Name.data()+6, "o_split_stack", 13))
        break;
      return AttributeList::AT_NoSplitStack;	 // "gnu::no_split_stack"
    }
    break;
  case 20:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case ':':	 // 1 string to match.
      if (memcmp(Name.data()+1, ":carries_dependency", 19))
        break;
      return AttributeList::AT_CarriesDependency;	 // "::carries_dependency"
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nu::returns_nonnull", 19))
        break;
      return AttributeList::AT_ReturnsNonNull;	 // "gnu::returns_nonnull"
    }
    break;
  case 22:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lang::type_visibility", 21))
        break;
      return AttributeList::AT_TypeVisibility;	 // "clang::type_visibility"
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nu::transparent_union", 21))
        break;
      return AttributeList::AT_TransparentUnion;	 // "gnu::transparent_union"
    }
    break;
  case 23:	 // 1 string to match.
    if (memcmp(Name.data()+0, "gnu::warn_unused_result", 23))
      break;
    return AttributeList::AT_WarnUnusedResult;	 // "gnu::warn_unused_result"
  case 24:	 // 3 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "lang::", 6))
        break;
      switch (Name[7]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(Name.data()+8, "ssert_capability", 16))
          break;
        return AttributeList::AT_AssertCapability;	 // "clang::assert_capability"
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+8, "hared_capability", 16))
          break;
        return AttributeList::AT_Capability;	 // "clang::shared_capability"
      }
      break;
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nu::no_sanitize_address", 23))
        break;
      return AttributeList::AT_NoSanitizeAddress;	 // "gnu::no_sanitize_address"
    }
    break;
  case 25:	 // 3 strings to match.
    if (memcmp(Name.data()+0, "clang::", 7))
      break;
    switch (Name[7]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+8, "cquire_capability", 17))
        break;
      return AttributeList::AT_AcquireCapability;	 // "clang::acquire_capability"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+8, "elease_capability", 17))
        break;
      return AttributeList::AT_ReleaseCapability;	 // "clang::release_capability"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+8, "arn_unused_result", 17))
        break;
      return AttributeList::AT_WarnUnusedResult;	 // "clang::warn_unused_result"
    }
    break;
  case 26:	 // 1 string to match.
    if (memcmp(Name.data()+0, "clang::requires_capability", 26))
      break;
    return AttributeList::AT_RequiresCapability;	 // "clang::requires_capability"
  case 27:	 // 1 string to match.
    if (memcmp(Name.data()+0, "gnu::no_instrument_function", 27))
      break;
    return AttributeList::AT_NoInstrumentFunction;	 // "gnu::no_instrument_function"
  case 29:	 // 1 string to match.
    if (memcmp(Name.data()+0, "clang::try_acquire_capability", 29))
      break;
    return AttributeList::AT_TryAcquireCapability;	 // "clang::try_acquire_capability"
  case 31:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lang::assert_shared_capability", 30))
        break;
      return AttributeList::AT_AssertCapability;	 // "clang::assert_shared_capability"
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "nu::no_address_safety_analysis", 30))
        break;
      return AttributeList::AT_NoSanitizeAddress;	 // "gnu::no_address_safety_analysis"
    }
    break;
  case 32:	 // 2 strings to match.
    if (memcmp(Name.data()+0, "clang::", 7))
      break;
    switch (Name[7]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+8, "cquire_shared_capability", 24))
        break;
      return AttributeList::AT_AcquireCapability;	 // "clang::acquire_shared_capability"
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+8, "elease_shared_capability", 24))
        break;
      return AttributeList::AT_ReleaseCapability;	 // "clang::release_shared_capability"
    }
    break;
  case 33:	 // 2 strings to match.
    if (memcmp(Name.data()+0, "clang::re", 9))
      break;
    switch (Name[9]) {
    default: break;
    case 'l':	 // 1 string to match.
      if (memcmp(Name.data()+10, "ease_generic_capability", 23))
        break;
      return AttributeList::AT_ReleaseCapability;	 // "clang::release_generic_capability"
    case 'q':	 // 1 string to match.
      if (memcmp(Name.data()+10, "uires_shared_capability", 23))
        break;
      return AttributeList::AT_RequiresCapability;	 // "clang::requires_shared_capability"
    }
    break;
  case 36:	 // 1 string to match.
    if (memcmp(Name.data()+0, "clang::try_acquire_shared_capability", 36))
      break;
    return AttributeList::AT_TryAcquireCapability;	 // "clang::try_acquire_shared_capability"
  }
  } else if (AttributeList::AS_Keyword == Syntax) {
  switch (Name.size()) {
  default: break;
  case 5:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 1 string to match.
      if (memcmp(Name.data()+1, "_w64", 4))
        break;
      return AttributeList::IgnoredAttribute;	 // "__w64"
    case 'l':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ocal", 4))
        break;
      return AttributeList::AT_OpenCLLocalAddressSpace;	 // "local"
    }
    break;
  case 6:	 // 5 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 3 strings to match.
      switch (Name[1]) {
      default: break;
      case '_':	 // 2 strings to match.
        switch (Name[2]) {
        default: break;
        case 's':	 // 1 string to match.
          if (memcmp(Name.data()+3, "ptr", 3))
            break;
          return AttributeList::AT_SPtr;	 // "__sptr"
        case 'u':	 // 1 string to match.
          if (memcmp(Name.data()+3, "ptr", 3))
            break;
          return AttributeList::AT_UPtr;	 // "__uptr"
        }
        break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+2, "decl", 4))
          break;
        return AttributeList::AT_CDecl;	 // "_cdecl"
      }
      break;
    case 'g':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lobal", 5))
        break;
      return AttributeList::AT_OpenCLGlobalAddressSpace;	 // "global"
    case 'k':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ernel", 5))
        break;
      return AttributeList::AT_OpenCLKernel;	 // "kernel"
    }
    break;
  case 7:	 // 7 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 5 strings to match.
      switch (Name[1]) {
      default: break;
      case '_':	 // 4 strings to match.
        switch (Name[2]) {
        default: break;
        case 'c':	 // 1 string to match.
          if (memcmp(Name.data()+3, "decl", 4))
            break;
          return AttributeList::AT_CDecl;	 // "__cdecl"
        case 'l':	 // 1 string to match.
          if (memcmp(Name.data()+3, "ocal", 4))
            break;
          return AttributeList::AT_OpenCLLocalAddressSpace;	 // "__local"
        case 'p':	 // 2 strings to match.
          if (memcmp(Name.data()+3, "tr", 2))
            break;
          switch (Name[5]) {
          default: break;
          case '3':	 // 1 string to match.
            if (Name[6] != '2')
              break;
            return AttributeList::AT_Ptr32;	 // "__ptr32"
          case '6':	 // 1 string to match.
            if (Name[6] != '4')
              break;
            return AttributeList::AT_Ptr64;	 // "__ptr64"
          }
          break;
        }
        break;
      case 'p':	 // 1 string to match.
        if (memcmp(Name.data()+2, "ascal", 5))
          break;
        return AttributeList::AT_Pascal;	 // "_pascal"
      }
      break;
    case 'a':	 // 1 string to match.
      if (memcmp(Name.data()+1, "lignas", 6))
        break;
      return AttributeList::AT_Aligned;	 // "alignas"
    case 'p':	 // 1 string to match.
      if (memcmp(Name.data()+1, "rivate", 6))
        break;
      return AttributeList::AT_OpenCLPrivateAddressSpace;	 // "private"
    }
    break;
  case 8:	 // 6 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 5 strings to match.
      switch (Name[1]) {
      default: break;
      case 'A':	 // 1 string to match.
        if (memcmp(Name.data()+2, "lignas", 6))
          break;
        return AttributeList::AT_Aligned;	 // "_Alignas"
      case '_':	 // 3 strings to match.
        switch (Name[2]) {
        default: break;
        case 'g':	 // 1 string to match.
          if (memcmp(Name.data()+3, "lobal", 5))
            break;
          return AttributeList::AT_OpenCLGlobalAddressSpace;	 // "__global"
        case 'k':	 // 1 string to match.
          if (memcmp(Name.data()+3, "ernel", 5))
            break;
          return AttributeList::AT_OpenCLKernel;	 // "__kernel"
        case 'p':	 // 1 string to match.
          if (memcmp(Name.data()+3, "ascal", 5))
            break;
          return AttributeList::AT_Pascal;	 // "__pascal"
        }
        break;
      case 's':	 // 1 string to match.
        if (memcmp(Name.data()+2, "tdcall", 6))
          break;
        return AttributeList::AT_StdCall;	 // "_stdcall"
      }
      break;
    case 'c':	 // 1 string to match.
      if (memcmp(Name.data()+1, "onstant", 7))
        break;
      return AttributeList::AT_OpenCLConstantAddressSpace;	 // "constant"
    }
    break;
  case 9:	 // 5 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 4 strings to match.
      switch (Name[1]) {
      default: break;
      case '_':	 // 2 strings to match.
        switch (Name[2]) {
        default: break;
        case 'p':	 // 1 string to match.
          if (memcmp(Name.data()+3, "rivate", 6))
            break;
          return AttributeList::AT_OpenCLPrivateAddressSpace;	 // "__private"
        case 's':	 // 1 string to match.
          if (memcmp(Name.data()+3, "tdcall", 6))
            break;
          return AttributeList::AT_StdCall;	 // "__stdcall"
        }
        break;
      case 'f':	 // 1 string to match.
        if (memcmp(Name.data()+2, "astcall", 7))
          break;
        return AttributeList::AT_FastCall;	 // "_fastcall"
      case 't':	 // 1 string to match.
        if (memcmp(Name.data()+2, "hiscall", 7))
          break;
        return AttributeList::AT_ThisCall;	 // "_thiscall"
      }
      break;
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ead_only", 8))
        break;
      return AttributeList::AT_OpenCLImageAccess;	 // "read_only"
    }
    break;
  case 10:	 // 5 strings to match.
    switch (Name[0]) {
    default: break;
    case '_':	 // 3 strings to match.
      if (Name[1] != '_')
        break;
      switch (Name[2]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(Name.data()+3, "onstant", 7))
          break;
        return AttributeList::AT_OpenCLConstantAddressSpace;	 // "__constant"
      case 'f':	 // 1 string to match.
        if (memcmp(Name.data()+3, "astcall", 7))
          break;
        return AttributeList::AT_FastCall;	 // "__fastcall"
      case 't':	 // 1 string to match.
        if (memcmp(Name.data()+3, "hiscall", 7))
          break;
        return AttributeList::AT_ThisCall;	 // "__thiscall"
      }
      break;
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ead_write", 9))
        break;
      return AttributeList::AT_OpenCLImageAccess;	 // "read_write"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+1, "rite_only", 9))
        break;
      return AttributeList::AT_OpenCLImageAccess;	 // "write_only"
    }
    break;
  case 11:	 // 3 strings to match.
    if (Name[0] != '_')
      break;
    switch (Name[1]) {
    default: break;
    case '_':	 // 2 strings to match.
      switch (Name[2]) {
      default: break;
      case 'r':	 // 1 string to match.
        if (memcmp(Name.data()+3, "ead_only", 8))
          break;
        return AttributeList::AT_OpenCLImageAccess;	 // "__read_only"
      case 'u':	 // 1 string to match.
        if (memcmp(Name.data()+3, "naligned", 8))
          break;
        return AttributeList::IgnoredAttribute;	 // "__unaligned"
      }
      break;
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+2, "ectorcall", 9))
        break;
      return AttributeList::AT_VectorCall;	 // "_vectorcall"
    }
    break;
  case 12:	 // 3 strings to match.
    if (memcmp(Name.data()+0, "__", 2))
      break;
    switch (Name[2]) {
    default: break;
    case 'r':	 // 1 string to match.
      if (memcmp(Name.data()+3, "ead_write", 9))
        break;
      return AttributeList::AT_OpenCLImageAccess;	 // "__read_write"
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+3, "ectorcall", 9))
        break;
      return AttributeList::AT_VectorCall;	 // "__vectorcall"
    case 'w':	 // 1 string to match.
      if (memcmp(Name.data()+3, "rite_only", 9))
        break;
      return AttributeList::AT_OpenCLImageAccess;	 // "__write_only"
    }
    break;
  case 13:	 // 1 string to match.
    if (memcmp(Name.data()+0, "__forceinline", 13))
      break;
    return AttributeList::AT_AlwaysInline;	 // "__forceinline"
  case 20:	 // 1 string to match.
    if (memcmp(Name.data()+0, "__single_inheritance", 20))
      break;
    return AttributeList::AT_MSInheritance;	 // "__single_inheritance"
  case 21:	 // 1 string to match.
    if (memcmp(Name.data()+0, "__virtual_inheritance", 21))
      break;
    return AttributeList::AT_MSInheritance;	 // "__virtual_inheritance"
  case 22:	 // 1 string to match.
    if (memcmp(Name.data()+0, "__multiple_inheritance", 22))
      break;
    return AttributeList::AT_MSInheritance;	 // "__multiple_inheritance"
  case 25:	 // 1 string to match.
    if (memcmp(Name.data()+0, "__unspecified_inheritance", 25))
      break;
    return AttributeList::AT_MSInheritance;	 // "__unspecified_inheritance"
  }
  } else if (AttributeList::AS_Pragma == Syntax) {
  switch (Name.size()) {
  default: break;
  case 4:	 // 1 string to match.
    if (memcmp(Name.data()+0, "loop", 4))
      break;
    return AttributeList::AT_LoopHint;	 // "loop"
  case 6:	 // 1 string to match.
    if (memcmp(Name.data()+0, "unroll", 6))
      break;
    return AttributeList::AT_LoopHint;	 // "unroll"
  case 8:	 // 1 string to match.
    if (memcmp(Name.data()+0, "nounroll", 8))
      break;
    return AttributeList::AT_LoopHint;	 // "nounroll"
  }
  }
  return AttributeList::UnknownAttribute;
}
