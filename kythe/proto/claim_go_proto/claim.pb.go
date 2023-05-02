// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.30.0
// 	protoc        v4.22.2
// source: kythe/proto/claim.proto

package claim_go_proto

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	storage_go_proto "kythe.io/kythe/proto/storage_go_proto"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type ClaimAssignment struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	CompilationVName *storage_go_proto.VName `protobuf:"bytes,1,opt,name=compilation_v_name,json=compilationVName,proto3" json:"compilation_v_name,omitempty"`
	DependencyVName  *storage_go_proto.VName `protobuf:"bytes,2,opt,name=dependency_v_name,json=dependencyVName,proto3" json:"dependency_v_name,omitempty"`
}

func (x *ClaimAssignment) Reset() {
	*x = ClaimAssignment{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_claim_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ClaimAssignment) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ClaimAssignment) ProtoMessage() {}

func (x *ClaimAssignment) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_claim_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ClaimAssignment.ProtoReflect.Descriptor instead.
func (*ClaimAssignment) Descriptor() ([]byte, []int) {
	return file_kythe_proto_claim_proto_rawDescGZIP(), []int{0}
}

func (x *ClaimAssignment) GetCompilationVName() *storage_go_proto.VName {
	if x != nil {
		return x.CompilationVName
	}
	return nil
}

func (x *ClaimAssignment) GetDependencyVName() *storage_go_proto.VName {
	if x != nil {
		return x.DependencyVName
	}
	return nil
}

var File_kythe_proto_claim_proto protoreflect.FileDescriptor

var file_kythe_proto_claim_proto_rawDesc = []byte{
	0x0a, 0x17, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x63, 0x6c,
	0x61, 0x69, 0x6d, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0b, 0x6b, 0x79, 0x74, 0x68, 0x65,
	0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x1a, 0x19, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x2f, 0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x22, 0x93, 0x01, 0x0a, 0x0f, 0x43, 0x6c, 0x61, 0x69, 0x6d, 0x41, 0x73, 0x73, 0x69, 0x67,
	0x6e, 0x6d, 0x65, 0x6e, 0x74, 0x12, 0x40, 0x0a, 0x12, 0x63, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61,
	0x74, 0x69, 0x6f, 0x6e, 0x5f, 0x76, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x01, 0x20, 0x01, 0x28,
	0x0b, 0x32, 0x12, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e,
	0x56, 0x4e, 0x61, 0x6d, 0x65, 0x52, 0x10, 0x63, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61, 0x74, 0x69,
	0x6f, 0x6e, 0x56, 0x4e, 0x61, 0x6d, 0x65, 0x12, 0x3e, 0x0a, 0x11, 0x64, 0x65, 0x70, 0x65, 0x6e,
	0x64, 0x65, 0x6e, 0x63, 0x79, 0x5f, 0x76, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x02, 0x20, 0x01,
	0x28, 0x0b, 0x32, 0x12, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x2e, 0x56, 0x4e, 0x61, 0x6d, 0x65, 0x52, 0x0f, 0x64, 0x65, 0x70, 0x65, 0x6e, 0x64, 0x65, 0x6e,
	0x63, 0x79, 0x56, 0x4e, 0x61, 0x6d, 0x65, 0x42, 0x31, 0x0a, 0x1f, 0x63, 0x6f, 0x6d, 0x2e, 0x67,
	0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x76, 0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x5a, 0x0e, 0x63, 0x6c, 0x61, 0x69,
	0x6d, 0x5f, 0x67, 0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x33,
}

var (
	file_kythe_proto_claim_proto_rawDescOnce sync.Once
	file_kythe_proto_claim_proto_rawDescData = file_kythe_proto_claim_proto_rawDesc
)

func file_kythe_proto_claim_proto_rawDescGZIP() []byte {
	file_kythe_proto_claim_proto_rawDescOnce.Do(func() {
		file_kythe_proto_claim_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_claim_proto_rawDescData)
	})
	return file_kythe_proto_claim_proto_rawDescData
}

var file_kythe_proto_claim_proto_msgTypes = make([]protoimpl.MessageInfo, 1)
var file_kythe_proto_claim_proto_goTypes = []interface{}{
	(*ClaimAssignment)(nil),        // 0: kythe.proto.ClaimAssignment
	(*storage_go_proto.VName)(nil), // 1: kythe.proto.VName
}
var file_kythe_proto_claim_proto_depIdxs = []int32{
	1, // 0: kythe.proto.ClaimAssignment.compilation_v_name:type_name -> kythe.proto.VName
	1, // 1: kythe.proto.ClaimAssignment.dependency_v_name:type_name -> kythe.proto.VName
	2, // [2:2] is the sub-list for method output_type
	2, // [2:2] is the sub-list for method input_type
	2, // [2:2] is the sub-list for extension type_name
	2, // [2:2] is the sub-list for extension extendee
	0, // [0:2] is the sub-list for field type_name
}

func init() { file_kythe_proto_claim_proto_init() }
func file_kythe_proto_claim_proto_init() {
	if File_kythe_proto_claim_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_claim_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*ClaimAssignment); i {
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
			RawDescriptor: file_kythe_proto_claim_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   1,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_proto_claim_proto_goTypes,
		DependencyIndexes: file_kythe_proto_claim_proto_depIdxs,
		MessageInfos:      file_kythe_proto_claim_proto_msgTypes,
	}.Build()
	File_kythe_proto_claim_proto = out.File
	file_kythe_proto_claim_proto_rawDesc = nil
	file_kythe_proto_claim_proto_goTypes = nil
	file_kythe_proto_claim_proto_depIdxs = nil
}
