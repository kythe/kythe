// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.31.0
// 	protoc        v4.25.2
// source: kythe/go/util/riegeli/riegeli_test.proto

package riegeli_test_go_proto

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type Simple struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Name *string `protobuf:"bytes,1,opt,name=name" json:"name,omitempty"`
}

func (x *Simple) Reset() {
	*x = Simple{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Simple) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Simple) ProtoMessage() {}

func (x *Simple) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Simple.ProtoReflect.Descriptor instead.
func (*Simple) Descriptor() ([]byte, []int) {
	return file_kythe_go_util_riegeli_riegeli_test_proto_rawDescGZIP(), []int{0}
}

func (x *Simple) GetName() string {
	if x != nil && x.Name != nil {
		return *x.Name
	}
	return ""
}

type Complex struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Str           *string          `protobuf:"bytes,1,opt,name=str" json:"str,omitempty"`
	I32           *int32           `protobuf:"varint,2,opt,name=i32" json:"i32,omitempty"`
	I64           *int64           `protobuf:"varint,3,opt,name=i64" json:"i64,omitempty"`
	Bits          []byte           `protobuf:"bytes,4,opt,name=bits" json:"bits,omitempty"`
	Rep           []string         `protobuf:"bytes,5,rep,name=rep" json:"rep,omitempty"`
	SimpleNested  *Simple          `protobuf:"bytes,6,opt,name=simple_nested,json=simpleNested" json:"simple_nested,omitempty"`
	Group         []*Complex_Group `protobuf:"group,7,rep,name=Group,json=group" json:"group,omitempty"`
	ComplexNested *Complex         `protobuf:"bytes,8,opt,name=complex_nested,json=complexNested" json:"complex_nested,omitempty"`
}

func (x *Complex) Reset() {
	*x = Complex{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Complex) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Complex) ProtoMessage() {}

func (x *Complex) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Complex.ProtoReflect.Descriptor instead.
func (*Complex) Descriptor() ([]byte, []int) {
	return file_kythe_go_util_riegeli_riegeli_test_proto_rawDescGZIP(), []int{1}
}

func (x *Complex) GetStr() string {
	if x != nil && x.Str != nil {
		return *x.Str
	}
	return ""
}

func (x *Complex) GetI32() int32 {
	if x != nil && x.I32 != nil {
		return *x.I32
	}
	return 0
}

func (x *Complex) GetI64() int64 {
	if x != nil && x.I64 != nil {
		return *x.I64
	}
	return 0
}

func (x *Complex) GetBits() []byte {
	if x != nil {
		return x.Bits
	}
	return nil
}

func (x *Complex) GetRep() []string {
	if x != nil {
		return x.Rep
	}
	return nil
}

func (x *Complex) GetSimpleNested() *Simple {
	if x != nil {
		return x.SimpleNested
	}
	return nil
}

func (x *Complex) GetGroup() []*Complex_Group {
	if x != nil {
		return x.Group
	}
	return nil
}

func (x *Complex) GetComplexNested() *Complex {
	if x != nil {
		return x.ComplexNested
	}
	return nil
}

type Complex_Group struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	GrpStr *string `protobuf:"bytes,1,opt,name=grp_str,json=grpStr" json:"grp_str,omitempty"`
}

func (x *Complex_Group) Reset() {
	*x = Complex_Group{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Complex_Group) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Complex_Group) ProtoMessage() {}

func (x *Complex_Group) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Complex_Group.ProtoReflect.Descriptor instead.
func (*Complex_Group) Descriptor() ([]byte, []int) {
	return file_kythe_go_util_riegeli_riegeli_test_proto_rawDescGZIP(), []int{1, 0}
}

func (x *Complex_Group) GetGrpStr() string {
	if x != nil && x.GrpStr != nil {
		return *x.GrpStr
	}
	return ""
}

var File_kythe_go_util_riegeli_riegeli_test_proto protoreflect.FileDescriptor

var file_kythe_go_util_riegeli_riegeli_test_proto_rawDesc = []byte{
	0x0a, 0x28, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x67, 0x6f, 0x2f, 0x75, 0x74, 0x69, 0x6c, 0x2f,
	0x72, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x2f, 0x72, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x5f,
	0x74, 0x65, 0x73, 0x74, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x18, 0x6b, 0x79, 0x74, 0x68,
	0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x72, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x5f,
	0x74, 0x65, 0x73, 0x74, 0x22, 0x1c, 0x0a, 0x06, 0x53, 0x69, 0x6d, 0x70, 0x6c, 0x65, 0x12, 0x12,
	0x0a, 0x04, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x6e, 0x61,
	0x6d, 0x65, 0x22, 0xd7, 0x02, 0x0a, 0x07, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x78, 0x12, 0x10,
	0x0a, 0x03, 0x73, 0x74, 0x72, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x03, 0x73, 0x74, 0x72,
	0x12, 0x10, 0x0a, 0x03, 0x69, 0x33, 0x32, 0x18, 0x02, 0x20, 0x01, 0x28, 0x05, 0x52, 0x03, 0x69,
	0x33, 0x32, 0x12, 0x10, 0x0a, 0x03, 0x69, 0x36, 0x34, 0x18, 0x03, 0x20, 0x01, 0x28, 0x03, 0x52,
	0x03, 0x69, 0x36, 0x34, 0x12, 0x12, 0x0a, 0x04, 0x62, 0x69, 0x74, 0x73, 0x18, 0x04, 0x20, 0x01,
	0x28, 0x0c, 0x52, 0x04, 0x62, 0x69, 0x74, 0x73, 0x12, 0x10, 0x0a, 0x03, 0x72, 0x65, 0x70, 0x18,
	0x05, 0x20, 0x03, 0x28, 0x09, 0x52, 0x03, 0x72, 0x65, 0x70, 0x12, 0x45, 0x0a, 0x0d, 0x73, 0x69,
	0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x18, 0x06, 0x20, 0x01, 0x28,
	0x0b, 0x32, 0x20, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e,
	0x72, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x53, 0x69, 0x6d,
	0x70, 0x6c, 0x65, 0x52, 0x0c, 0x73, 0x69, 0x6d, 0x70, 0x6c, 0x65, 0x4e, 0x65, 0x73, 0x74, 0x65,
	0x64, 0x12, 0x3d, 0x0a, 0x05, 0x67, 0x72, 0x6f, 0x75, 0x70, 0x18, 0x07, 0x20, 0x03, 0x28, 0x0a,
	0x32, 0x27, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x72,
	0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x43, 0x6f, 0x6d, 0x70,
	0x6c, 0x65, 0x78, 0x2e, 0x47, 0x72, 0x6f, 0x75, 0x70, 0x52, 0x05, 0x67, 0x72, 0x6f, 0x75, 0x70,
	0x12, 0x48, 0x0a, 0x0e, 0x63, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x78, 0x5f, 0x6e, 0x65, 0x73, 0x74,
	0x65, 0x64, 0x18, 0x08, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x21, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65,
	0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x72, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x5f, 0x74,
	0x65, 0x73, 0x74, 0x2e, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x78, 0x52, 0x0d, 0x63, 0x6f, 0x6d,
	0x70, 0x6c, 0x65, 0x78, 0x4e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x1a, 0x20, 0x0a, 0x05, 0x47, 0x72,
	0x6f, 0x75, 0x70, 0x12, 0x17, 0x0a, 0x07, 0x67, 0x72, 0x70, 0x5f, 0x73, 0x74, 0x72, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x06, 0x67, 0x72, 0x70, 0x53, 0x74, 0x72, 0x42, 0x36, 0x5a, 0x34,
	0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x69, 0x6f, 0x2f, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x67,
	0x6f, 0x2f, 0x75, 0x74, 0x69, 0x6c, 0x2f, 0x72, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x2f, 0x72,
	0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x67, 0x6f, 0x5f, 0x70,
	0x72, 0x6f, 0x74, 0x6f,
}

var (
	file_kythe_go_util_riegeli_riegeli_test_proto_rawDescOnce sync.Once
	file_kythe_go_util_riegeli_riegeli_test_proto_rawDescData = file_kythe_go_util_riegeli_riegeli_test_proto_rawDesc
)

func file_kythe_go_util_riegeli_riegeli_test_proto_rawDescGZIP() []byte {
	file_kythe_go_util_riegeli_riegeli_test_proto_rawDescOnce.Do(func() {
		file_kythe_go_util_riegeli_riegeli_test_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_go_util_riegeli_riegeli_test_proto_rawDescData)
	})
	return file_kythe_go_util_riegeli_riegeli_test_proto_rawDescData
}

var file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes = make([]protoimpl.MessageInfo, 3)
var file_kythe_go_util_riegeli_riegeli_test_proto_goTypes = []interface{}{
	(*Simple)(nil),        // 0: kythe.proto.riegeli_test.Simple
	(*Complex)(nil),       // 1: kythe.proto.riegeli_test.Complex
	(*Complex_Group)(nil), // 2: kythe.proto.riegeli_test.Complex.Group
}
var file_kythe_go_util_riegeli_riegeli_test_proto_depIdxs = []int32{
	0, // 0: kythe.proto.riegeli_test.Complex.simple_nested:type_name -> kythe.proto.riegeli_test.Simple
	2, // 1: kythe.proto.riegeli_test.Complex.group:type_name -> kythe.proto.riegeli_test.Complex.Group
	1, // 2: kythe.proto.riegeli_test.Complex.complex_nested:type_name -> kythe.proto.riegeli_test.Complex
	3, // [3:3] is the sub-list for method output_type
	3, // [3:3] is the sub-list for method input_type
	3, // [3:3] is the sub-list for extension type_name
	3, // [3:3] is the sub-list for extension extendee
	0, // [0:3] is the sub-list for field type_name
}

func init() { file_kythe_go_util_riegeli_riegeli_test_proto_init() }
func file_kythe_go_util_riegeli_riegeli_test_proto_init() {
	if File_kythe_go_util_riegeli_riegeli_test_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Simple); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Complex); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Complex_Group); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_kythe_go_util_riegeli_riegeli_test_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   3,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_go_util_riegeli_riegeli_test_proto_goTypes,
		DependencyIndexes: file_kythe_go_util_riegeli_riegeli_test_proto_depIdxs,
		MessageInfos:      file_kythe_go_util_riegeli_riegeli_test_proto_msgTypes,
	}.Build()
	File_kythe_go_util_riegeli_riegeli_test_proto = out.File
	file_kythe_go_util_riegeli_riegeli_test_proto_rawDesc = nil
	file_kythe_go_util_riegeli_riegeli_test_proto_goTypes = nil
	file_kythe_go_util_riegeli_riegeli_test_proto_depIdxs = nil
}
