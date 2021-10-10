// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.27.1
// 	protoc        v3.18.1
// source: kythe/proto/filecontext.proto

package filecontext_go_proto

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

type ContextDependentVersion struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Row []*ContextDependentVersion_Row `protobuf:"bytes,1,rep,name=row,proto3" json:"row,omitempty"`
}

func (x *ContextDependentVersion) Reset() {
	*x = ContextDependentVersion{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_filecontext_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ContextDependentVersion) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ContextDependentVersion) ProtoMessage() {}

func (x *ContextDependentVersion) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_filecontext_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ContextDependentVersion.ProtoReflect.Descriptor instead.
func (*ContextDependentVersion) Descriptor() ([]byte, []int) {
	return file_kythe_proto_filecontext_proto_rawDescGZIP(), []int{0}
}

func (x *ContextDependentVersion) GetRow() []*ContextDependentVersion_Row {
	if x != nil {
		return x.Row
	}
	return nil
}

type ContextDependentVersion_Column struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Offset        int32  `protobuf:"varint,1,opt,name=offset,proto3" json:"offset,omitempty"`
	LinkedContext string `protobuf:"bytes,2,opt,name=linked_context,json=linkedContext,proto3" json:"linked_context,omitempty"`
}

func (x *ContextDependentVersion_Column) Reset() {
	*x = ContextDependentVersion_Column{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_filecontext_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ContextDependentVersion_Column) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ContextDependentVersion_Column) ProtoMessage() {}

func (x *ContextDependentVersion_Column) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_filecontext_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ContextDependentVersion_Column.ProtoReflect.Descriptor instead.
func (*ContextDependentVersion_Column) Descriptor() ([]byte, []int) {
	return file_kythe_proto_filecontext_proto_rawDescGZIP(), []int{0, 0}
}

func (x *ContextDependentVersion_Column) GetOffset() int32 {
	if x != nil {
		return x.Offset
	}
	return 0
}

func (x *ContextDependentVersion_Column) GetLinkedContext() string {
	if x != nil {
		return x.LinkedContext
	}
	return ""
}

type ContextDependentVersion_Row struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	SourceContext string                            `protobuf:"bytes,1,opt,name=source_context,json=sourceContext,proto3" json:"source_context,omitempty"`
	Column        []*ContextDependentVersion_Column `protobuf:"bytes,2,rep,name=column,proto3" json:"column,omitempty"`
	AlwaysProcess bool                              `protobuf:"varint,3,opt,name=always_process,json=alwaysProcess,proto3" json:"always_process,omitempty"`
}

func (x *ContextDependentVersion_Row) Reset() {
	*x = ContextDependentVersion_Row{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_filecontext_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ContextDependentVersion_Row) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ContextDependentVersion_Row) ProtoMessage() {}

func (x *ContextDependentVersion_Row) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_filecontext_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ContextDependentVersion_Row.ProtoReflect.Descriptor instead.
func (*ContextDependentVersion_Row) Descriptor() ([]byte, []int) {
	return file_kythe_proto_filecontext_proto_rawDescGZIP(), []int{0, 1}
}

func (x *ContextDependentVersion_Row) GetSourceContext() string {
	if x != nil {
		return x.SourceContext
	}
	return ""
}

func (x *ContextDependentVersion_Row) GetColumn() []*ContextDependentVersion_Column {
	if x != nil {
		return x.Column
	}
	return nil
}

func (x *ContextDependentVersion_Row) GetAlwaysProcess() bool {
	if x != nil {
		return x.AlwaysProcess
	}
	return false
}

var File_kythe_proto_filecontext_proto protoreflect.FileDescriptor

var file_kythe_proto_filecontext_proto_rawDesc = []byte{
	0x0a, 0x1d, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x66, 0x69,
	0x6c, 0x65, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12,
	0x0b, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22, 0xb9, 0x02, 0x0a,
	0x17, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x44, 0x65, 0x70, 0x65, 0x6e, 0x64, 0x65, 0x6e,
	0x74, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x12, 0x3a, 0x0a, 0x03, 0x72, 0x6f, 0x77, 0x18,
	0x01, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x28, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x44, 0x65, 0x70, 0x65, 0x6e,
	0x64, 0x65, 0x6e, 0x74, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x2e, 0x52, 0x6f, 0x77, 0x52,
	0x03, 0x72, 0x6f, 0x77, 0x1a, 0x47, 0x0a, 0x06, 0x43, 0x6f, 0x6c, 0x75, 0x6d, 0x6e, 0x12, 0x16,
	0x0a, 0x06, 0x6f, 0x66, 0x66, 0x73, 0x65, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28, 0x05, 0x52, 0x06,
	0x6f, 0x66, 0x66, 0x73, 0x65, 0x74, 0x12, 0x25, 0x0a, 0x0e, 0x6c, 0x69, 0x6e, 0x6b, 0x65, 0x64,
	0x5f, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0d,
	0x6c, 0x69, 0x6e, 0x6b, 0x65, 0x64, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x1a, 0x98, 0x01,
	0x0a, 0x03, 0x52, 0x6f, 0x77, 0x12, 0x25, 0x0a, 0x0e, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x5f,
	0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0d, 0x73,
	0x6f, 0x75, 0x72, 0x63, 0x65, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x12, 0x43, 0x0a, 0x06,
	0x63, 0x6f, 0x6c, 0x75, 0x6d, 0x6e, 0x18, 0x02, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x2b, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x6f, 0x6e, 0x74, 0x65,
	0x78, 0x74, 0x44, 0x65, 0x70, 0x65, 0x6e, 0x64, 0x65, 0x6e, 0x74, 0x56, 0x65, 0x72, 0x73, 0x69,
	0x6f, 0x6e, 0x2e, 0x43, 0x6f, 0x6c, 0x75, 0x6d, 0x6e, 0x52, 0x06, 0x63, 0x6f, 0x6c, 0x75, 0x6d,
	0x6e, 0x12, 0x25, 0x0a, 0x0e, 0x61, 0x6c, 0x77, 0x61, 0x79, 0x73, 0x5f, 0x70, 0x72, 0x6f, 0x63,
	0x65, 0x73, 0x73, 0x18, 0x03, 0x20, 0x01, 0x28, 0x08, 0x52, 0x0d, 0x61, 0x6c, 0x77, 0x61, 0x79,
	0x73, 0x50, 0x72, 0x6f, 0x63, 0x65, 0x73, 0x73, 0x42, 0x37, 0x0a, 0x1f, 0x63, 0x6f, 0x6d, 0x2e,
	0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x76, 0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2e,
	0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x5a, 0x14, 0x66, 0x69, 0x6c,
	0x65, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x5f, 0x67, 0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_kythe_proto_filecontext_proto_rawDescOnce sync.Once
	file_kythe_proto_filecontext_proto_rawDescData = file_kythe_proto_filecontext_proto_rawDesc
)

func file_kythe_proto_filecontext_proto_rawDescGZIP() []byte {
	file_kythe_proto_filecontext_proto_rawDescOnce.Do(func() {
		file_kythe_proto_filecontext_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_filecontext_proto_rawDescData)
	})
	return file_kythe_proto_filecontext_proto_rawDescData
}

var file_kythe_proto_filecontext_proto_msgTypes = make([]protoimpl.MessageInfo, 3)
var file_kythe_proto_filecontext_proto_goTypes = []interface{}{
	(*ContextDependentVersion)(nil),        // 0: kythe.proto.ContextDependentVersion
	(*ContextDependentVersion_Column)(nil), // 1: kythe.proto.ContextDependentVersion.Column
	(*ContextDependentVersion_Row)(nil),    // 2: kythe.proto.ContextDependentVersion.Row
}
var file_kythe_proto_filecontext_proto_depIdxs = []int32{
	2, // 0: kythe.proto.ContextDependentVersion.row:type_name -> kythe.proto.ContextDependentVersion.Row
	1, // 1: kythe.proto.ContextDependentVersion.Row.column:type_name -> kythe.proto.ContextDependentVersion.Column
	2, // [2:2] is the sub-list for method output_type
	2, // [2:2] is the sub-list for method input_type
	2, // [2:2] is the sub-list for extension type_name
	2, // [2:2] is the sub-list for extension extendee
	0, // [0:2] is the sub-list for field type_name
}

func init() { file_kythe_proto_filecontext_proto_init() }
func file_kythe_proto_filecontext_proto_init() {
	if File_kythe_proto_filecontext_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_filecontext_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*ContextDependentVersion); i {
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
		file_kythe_proto_filecontext_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*ContextDependentVersion_Column); i {
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
		file_kythe_proto_filecontext_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*ContextDependentVersion_Row); i {
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
			RawDescriptor: file_kythe_proto_filecontext_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   3,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_proto_filecontext_proto_goTypes,
		DependencyIndexes: file_kythe_proto_filecontext_proto_depIdxs,
		MessageInfos:      file_kythe_proto_filecontext_proto_msgTypes,
	}.Build()
	File_kythe_proto_filecontext_proto = out.File
	file_kythe_proto_filecontext_proto_rawDesc = nil
	file_kythe_proto_filecontext_proto_goTypes = nil
	file_kythe_proto_filecontext_proto_depIdxs = nil
}
