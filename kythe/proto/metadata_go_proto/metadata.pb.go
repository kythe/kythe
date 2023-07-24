// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.30.0
// 	protoc        v4.22.2
// source: kythe/proto/metadata.proto

package metadata_go_proto

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

type GeneratedCodeInfo_Type int32

const (
	GeneratedCodeInfo_NONE   GeneratedCodeInfo_Type = 0
	GeneratedCodeInfo_KYTHE0 GeneratedCodeInfo_Type = 1
)

// Enum value maps for GeneratedCodeInfo_Type.
var (
	GeneratedCodeInfo_Type_name = map[int32]string{
		0: "NONE",
		1: "KYTHE0",
	}
	GeneratedCodeInfo_Type_value = map[string]int32{
		"NONE":   0,
		"KYTHE0": 1,
	}
)

func (x GeneratedCodeInfo_Type) Enum() *GeneratedCodeInfo_Type {
	p := new(GeneratedCodeInfo_Type)
	*p = x
	return p
}

func (x GeneratedCodeInfo_Type) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (GeneratedCodeInfo_Type) Descriptor() protoreflect.EnumDescriptor {
	return file_kythe_proto_metadata_proto_enumTypes[0].Descriptor()
}

func (GeneratedCodeInfo_Type) Type() protoreflect.EnumType {
	return &file_kythe_proto_metadata_proto_enumTypes[0]
}

func (x GeneratedCodeInfo_Type) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use GeneratedCodeInfo_Type.Descriptor instead.
func (GeneratedCodeInfo_Type) EnumDescriptor() ([]byte, []int) {
	return file_kythe_proto_metadata_proto_rawDescGZIP(), []int{0, 0}
}

type MappingRule_Type int32

const (
	MappingRule_NONE           MappingRule_Type = 0
	MappingRule_NOP            MappingRule_Type = 1
	MappingRule_ANCHOR_DEFINES MappingRule_Type = 2
	MappingRule_ANCHOR_ANCHOR  MappingRule_Type = 3
)

// Enum value maps for MappingRule_Type.
var (
	MappingRule_Type_name = map[int32]string{
		0: "NONE",
		1: "NOP",
		2: "ANCHOR_DEFINES",
		3: "ANCHOR_ANCHOR",
	}
	MappingRule_Type_value = map[string]int32{
		"NONE":           0,
		"NOP":            1,
		"ANCHOR_DEFINES": 2,
		"ANCHOR_ANCHOR":  3,
	}
)

func (x MappingRule_Type) Enum() *MappingRule_Type {
	p := new(MappingRule_Type)
	*p = x
	return p
}

func (x MappingRule_Type) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (MappingRule_Type) Descriptor() protoreflect.EnumDescriptor {
	return file_kythe_proto_metadata_proto_enumTypes[1].Descriptor()
}

func (MappingRule_Type) Type() protoreflect.EnumType {
	return &file_kythe_proto_metadata_proto_enumTypes[1]
}

func (x MappingRule_Type) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use MappingRule_Type.Descriptor instead.
func (MappingRule_Type) EnumDescriptor() ([]byte, []int) {
	return file_kythe_proto_metadata_proto_rawDescGZIP(), []int{1, 0}
}

type MappingRule_Semantic int32

const (
	MappingRule_SEMA_NONE       MappingRule_Semantic = 0
	MappingRule_SEMA_WRITE      MappingRule_Semantic = 1
	MappingRule_SEMA_READ_WRITE MappingRule_Semantic = 2
	MappingRule_SEMA_TAKE_ALIAS MappingRule_Semantic = 3
)

// Enum value maps for MappingRule_Semantic.
var (
	MappingRule_Semantic_name = map[int32]string{
		0: "SEMA_NONE",
		1: "SEMA_WRITE",
		2: "SEMA_READ_WRITE",
		3: "SEMA_TAKE_ALIAS",
	}
	MappingRule_Semantic_value = map[string]int32{
		"SEMA_NONE":       0,
		"SEMA_WRITE":      1,
		"SEMA_READ_WRITE": 2,
		"SEMA_TAKE_ALIAS": 3,
	}
)

func (x MappingRule_Semantic) Enum() *MappingRule_Semantic {
	p := new(MappingRule_Semantic)
	*p = x
	return p
}

func (x MappingRule_Semantic) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (MappingRule_Semantic) Descriptor() protoreflect.EnumDescriptor {
	return file_kythe_proto_metadata_proto_enumTypes[2].Descriptor()
}

func (MappingRule_Semantic) Type() protoreflect.EnumType {
	return &file_kythe_proto_metadata_proto_enumTypes[2]
}

func (x MappingRule_Semantic) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use MappingRule_Semantic.Descriptor instead.
func (MappingRule_Semantic) EnumDescriptor() ([]byte, []int) {
	return file_kythe_proto_metadata_proto_rawDescGZIP(), []int{1, 1}
}

type GeneratedCodeInfo struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Type GeneratedCodeInfo_Type `protobuf:"varint,1,opt,name=type,proto3,enum=kythe.proto.metadata.GeneratedCodeInfo_Type" json:"type,omitempty"`
	Meta []*MappingRule         `protobuf:"bytes,2,rep,name=meta,proto3" json:"meta,omitempty"`
}

func (x *GeneratedCodeInfo) Reset() {
	*x = GeneratedCodeInfo{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_metadata_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *GeneratedCodeInfo) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*GeneratedCodeInfo) ProtoMessage() {}

func (x *GeneratedCodeInfo) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_metadata_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use GeneratedCodeInfo.ProtoReflect.Descriptor instead.
func (*GeneratedCodeInfo) Descriptor() ([]byte, []int) {
	return file_kythe_proto_metadata_proto_rawDescGZIP(), []int{0}
}

func (x *GeneratedCodeInfo) GetType() GeneratedCodeInfo_Type {
	if x != nil {
		return x.Type
	}
	return GeneratedCodeInfo_NONE
}

func (x *GeneratedCodeInfo) GetMeta() []*MappingRule {
	if x != nil {
		return x.Meta
	}
	return nil
}

type MappingRule struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Type        MappingRule_Type        `protobuf:"varint,1,opt,name=type,proto3,enum=kythe.proto.metadata.MappingRule_Type" json:"type,omitempty"`
	Edge        string                  `protobuf:"bytes,2,opt,name=edge,proto3" json:"edge,omitempty"`
	Vname       *storage_go_proto.VName `protobuf:"bytes,3,opt,name=vname,proto3" json:"vname,omitempty"`
	Begin       uint32                  `protobuf:"varint,4,opt,name=begin,proto3" json:"begin,omitempty"`
	End         uint32                  `protobuf:"varint,5,opt,name=end,proto3" json:"end,omitempty"`
	Semantic    MappingRule_Semantic    `protobuf:"varint,11,opt,name=semantic,proto3,enum=kythe.proto.metadata.MappingRule_Semantic" json:"semantic,omitempty"`
	SourceVname *storage_go_proto.VName `protobuf:"bytes,6,opt,name=source_vname,json=sourceVname,proto3" json:"source_vname,omitempty"`
	SourceBegin uint32                  `protobuf:"varint,7,opt,name=source_begin,json=sourceBegin,proto3" json:"source_begin,omitempty"`
	SourceEnd   uint32                  `protobuf:"varint,8,opt,name=source_end,json=sourceEnd,proto3" json:"source_end,omitempty"`
	TargetBegin uint32                  `protobuf:"varint,9,opt,name=target_begin,json=targetBegin,proto3" json:"target_begin,omitempty"`
	TargetEnd   uint32                  `protobuf:"varint,10,opt,name=target_end,json=targetEnd,proto3" json:"target_end,omitempty"`
}

func (x *MappingRule) Reset() {
	*x = MappingRule{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_metadata_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *MappingRule) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*MappingRule) ProtoMessage() {}

func (x *MappingRule) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_metadata_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use MappingRule.ProtoReflect.Descriptor instead.
func (*MappingRule) Descriptor() ([]byte, []int) {
	return file_kythe_proto_metadata_proto_rawDescGZIP(), []int{1}
}

func (x *MappingRule) GetType() MappingRule_Type {
	if x != nil {
		return x.Type
	}
	return MappingRule_NONE
}

func (x *MappingRule) GetEdge() string {
	if x != nil {
		return x.Edge
	}
	return ""
}

func (x *MappingRule) GetVname() *storage_go_proto.VName {
	if x != nil {
		return x.Vname
	}
	return nil
}

func (x *MappingRule) GetBegin() uint32 {
	if x != nil {
		return x.Begin
	}
	return 0
}

func (x *MappingRule) GetEnd() uint32 {
	if x != nil {
		return x.End
	}
	return 0
}

func (x *MappingRule) GetSemantic() MappingRule_Semantic {
	if x != nil {
		return x.Semantic
	}
	return MappingRule_SEMA_NONE
}

func (x *MappingRule) GetSourceVname() *storage_go_proto.VName {
	if x != nil {
		return x.SourceVname
	}
	return nil
}

func (x *MappingRule) GetSourceBegin() uint32 {
	if x != nil {
		return x.SourceBegin
	}
	return 0
}

func (x *MappingRule) GetSourceEnd() uint32 {
	if x != nil {
		return x.SourceEnd
	}
	return 0
}

func (x *MappingRule) GetTargetBegin() uint32 {
	if x != nil {
		return x.TargetBegin
	}
	return 0
}

func (x *MappingRule) GetTargetEnd() uint32 {
	if x != nil {
		return x.TargetEnd
	}
	return 0
}

var File_kythe_proto_metadata_proto protoreflect.FileDescriptor

var file_kythe_proto_metadata_proto_rawDesc = []byte{
	0x0a, 0x1a, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x6d, 0x65,
	0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x14, 0x6b, 0x79,
	0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x6d, 0x65, 0x74, 0x61, 0x64, 0x61,
	0x74, 0x61, 0x1a, 0x19, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f,
	0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22, 0xaa, 0x01,
	0x0a, 0x11, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x74, 0x65, 0x64, 0x43, 0x6f, 0x64, 0x65, 0x49,
	0x6e, 0x66, 0x6f, 0x12, 0x40, 0x0a, 0x04, 0x74, 0x79, 0x70, 0x65, 0x18, 0x01, 0x20, 0x01, 0x28,
	0x0e, 0x32, 0x2c, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e,
	0x6d, 0x65, 0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x2e, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x74,
	0x65, 0x64, 0x43, 0x6f, 0x64, 0x65, 0x49, 0x6e, 0x66, 0x6f, 0x2e, 0x54, 0x79, 0x70, 0x65, 0x52,
	0x04, 0x74, 0x79, 0x70, 0x65, 0x12, 0x35, 0x0a, 0x04, 0x6d, 0x65, 0x74, 0x61, 0x18, 0x02, 0x20,
	0x03, 0x28, 0x0b, 0x32, 0x21, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x2e, 0x6d, 0x65, 0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x2e, 0x4d, 0x61, 0x70, 0x70, 0x69,
	0x6e, 0x67, 0x52, 0x75, 0x6c, 0x65, 0x52, 0x04, 0x6d, 0x65, 0x74, 0x61, 0x22, 0x1c, 0x0a, 0x04,
	0x54, 0x79, 0x70, 0x65, 0x12, 0x08, 0x0a, 0x04, 0x4e, 0x4f, 0x4e, 0x45, 0x10, 0x00, 0x12, 0x0a,
	0x0a, 0x06, 0x4b, 0x59, 0x54, 0x48, 0x45, 0x30, 0x10, 0x01, 0x22, 0xc9, 0x04, 0x0a, 0x0b, 0x4d,
	0x61, 0x70, 0x70, 0x69, 0x6e, 0x67, 0x52, 0x75, 0x6c, 0x65, 0x12, 0x3a, 0x0a, 0x04, 0x74, 0x79,
	0x70, 0x65, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x26, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65,
	0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x6d, 0x65, 0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x2e,
	0x4d, 0x61, 0x70, 0x70, 0x69, 0x6e, 0x67, 0x52, 0x75, 0x6c, 0x65, 0x2e, 0x54, 0x79, 0x70, 0x65,
	0x52, 0x04, 0x74, 0x79, 0x70, 0x65, 0x12, 0x12, 0x0a, 0x04, 0x65, 0x64, 0x67, 0x65, 0x18, 0x02,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x65, 0x64, 0x67, 0x65, 0x12, 0x28, 0x0a, 0x05, 0x76, 0x6e,
	0x61, 0x6d, 0x65, 0x18, 0x03, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x12, 0x2e, 0x6b, 0x79, 0x74, 0x68,
	0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x56, 0x4e, 0x61, 0x6d, 0x65, 0x52, 0x05, 0x76,
	0x6e, 0x61, 0x6d, 0x65, 0x12, 0x14, 0x0a, 0x05, 0x62, 0x65, 0x67, 0x69, 0x6e, 0x18, 0x04, 0x20,
	0x01, 0x28, 0x0d, 0x52, 0x05, 0x62, 0x65, 0x67, 0x69, 0x6e, 0x12, 0x10, 0x0a, 0x03, 0x65, 0x6e,
	0x64, 0x18, 0x05, 0x20, 0x01, 0x28, 0x0d, 0x52, 0x03, 0x65, 0x6e, 0x64, 0x12, 0x46, 0x0a, 0x08,
	0x73, 0x65, 0x6d, 0x61, 0x6e, 0x74, 0x69, 0x63, 0x18, 0x0b, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x2a,
	0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x6d, 0x65, 0x74,
	0x61, 0x64, 0x61, 0x74, 0x61, 0x2e, 0x4d, 0x61, 0x70, 0x70, 0x69, 0x6e, 0x67, 0x52, 0x75, 0x6c,
	0x65, 0x2e, 0x53, 0x65, 0x6d, 0x61, 0x6e, 0x74, 0x69, 0x63, 0x52, 0x08, 0x73, 0x65, 0x6d, 0x61,
	0x6e, 0x74, 0x69, 0x63, 0x12, 0x35, 0x0a, 0x0c, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x5f, 0x76,
	0x6e, 0x61, 0x6d, 0x65, 0x18, 0x06, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x12, 0x2e, 0x6b, 0x79, 0x74,
	0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x56, 0x4e, 0x61, 0x6d, 0x65, 0x52, 0x0b,
	0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x56, 0x6e, 0x61, 0x6d, 0x65, 0x12, 0x21, 0x0a, 0x0c, 0x73,
	0x6f, 0x75, 0x72, 0x63, 0x65, 0x5f, 0x62, 0x65, 0x67, 0x69, 0x6e, 0x18, 0x07, 0x20, 0x01, 0x28,
	0x0d, 0x52, 0x0b, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x42, 0x65, 0x67, 0x69, 0x6e, 0x12, 0x1d,
	0x0a, 0x0a, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x5f, 0x65, 0x6e, 0x64, 0x18, 0x08, 0x20, 0x01,
	0x28, 0x0d, 0x52, 0x09, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x45, 0x6e, 0x64, 0x12, 0x21, 0x0a,
	0x0c, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x5f, 0x62, 0x65, 0x67, 0x69, 0x6e, 0x18, 0x09, 0x20,
	0x01, 0x28, 0x0d, 0x52, 0x0b, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x42, 0x65, 0x67, 0x69, 0x6e,
	0x12, 0x1d, 0x0a, 0x0a, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x5f, 0x65, 0x6e, 0x64, 0x18, 0x0a,
	0x20, 0x01, 0x28, 0x0d, 0x52, 0x09, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x45, 0x6e, 0x64, 0x22,
	0x40, 0x0a, 0x04, 0x54, 0x79, 0x70, 0x65, 0x12, 0x08, 0x0a, 0x04, 0x4e, 0x4f, 0x4e, 0x45, 0x10,
	0x00, 0x12, 0x07, 0x0a, 0x03, 0x4e, 0x4f, 0x50, 0x10, 0x01, 0x12, 0x12, 0x0a, 0x0e, 0x41, 0x4e,
	0x43, 0x48, 0x4f, 0x52, 0x5f, 0x44, 0x45, 0x46, 0x49, 0x4e, 0x45, 0x53, 0x10, 0x02, 0x12, 0x11,
	0x0a, 0x0d, 0x41, 0x4e, 0x43, 0x48, 0x4f, 0x52, 0x5f, 0x41, 0x4e, 0x43, 0x48, 0x4f, 0x52, 0x10,
	0x03, 0x22, 0x53, 0x0a, 0x08, 0x53, 0x65, 0x6d, 0x61, 0x6e, 0x74, 0x69, 0x63, 0x12, 0x0d, 0x0a,
	0x09, 0x53, 0x45, 0x4d, 0x41, 0x5f, 0x4e, 0x4f, 0x4e, 0x45, 0x10, 0x00, 0x12, 0x0e, 0x0a, 0x0a,
	0x53, 0x45, 0x4d, 0x41, 0x5f, 0x57, 0x52, 0x49, 0x54, 0x45, 0x10, 0x01, 0x12, 0x13, 0x0a, 0x0f,
	0x53, 0x45, 0x4d, 0x41, 0x5f, 0x52, 0x45, 0x41, 0x44, 0x5f, 0x57, 0x52, 0x49, 0x54, 0x45, 0x10,
	0x02, 0x12, 0x13, 0x0a, 0x0f, 0x53, 0x45, 0x4d, 0x41, 0x5f, 0x54, 0x41, 0x4b, 0x45, 0x5f, 0x41,
	0x4c, 0x49, 0x41, 0x53, 0x10, 0x03, 0x42, 0x49, 0x0a, 0x1f, 0x63, 0x6f, 0x6d, 0x2e, 0x67, 0x6f,
	0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x76, 0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2e, 0x6b, 0x79,
	0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x5a, 0x26, 0x6b, 0x79, 0x74, 0x68, 0x65,
	0x2e, 0x69, 0x6f, 0x2f, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f,
	0x6d, 0x65, 0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x5f, 0x67, 0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_kythe_proto_metadata_proto_rawDescOnce sync.Once
	file_kythe_proto_metadata_proto_rawDescData = file_kythe_proto_metadata_proto_rawDesc
)

func file_kythe_proto_metadata_proto_rawDescGZIP() []byte {
	file_kythe_proto_metadata_proto_rawDescOnce.Do(func() {
		file_kythe_proto_metadata_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_metadata_proto_rawDescData)
	})
	return file_kythe_proto_metadata_proto_rawDescData
}

var file_kythe_proto_metadata_proto_enumTypes = make([]protoimpl.EnumInfo, 3)
var file_kythe_proto_metadata_proto_msgTypes = make([]protoimpl.MessageInfo, 2)
var file_kythe_proto_metadata_proto_goTypes = []interface{}{
	(GeneratedCodeInfo_Type)(0),    // 0: kythe.proto.metadata.GeneratedCodeInfo.Type
	(MappingRule_Type)(0),          // 1: kythe.proto.metadata.MappingRule.Type
	(MappingRule_Semantic)(0),      // 2: kythe.proto.metadata.MappingRule.Semantic
	(*GeneratedCodeInfo)(nil),      // 3: kythe.proto.metadata.GeneratedCodeInfo
	(*MappingRule)(nil),            // 4: kythe.proto.metadata.MappingRule
	(*storage_go_proto.VName)(nil), // 5: kythe.proto.VName
}
var file_kythe_proto_metadata_proto_depIdxs = []int32{
	0, // 0: kythe.proto.metadata.GeneratedCodeInfo.type:type_name -> kythe.proto.metadata.GeneratedCodeInfo.Type
	4, // 1: kythe.proto.metadata.GeneratedCodeInfo.meta:type_name -> kythe.proto.metadata.MappingRule
	1, // 2: kythe.proto.metadata.MappingRule.type:type_name -> kythe.proto.metadata.MappingRule.Type
	5, // 3: kythe.proto.metadata.MappingRule.vname:type_name -> kythe.proto.VName
	2, // 4: kythe.proto.metadata.MappingRule.semantic:type_name -> kythe.proto.metadata.MappingRule.Semantic
	5, // 5: kythe.proto.metadata.MappingRule.source_vname:type_name -> kythe.proto.VName
	6, // [6:6] is the sub-list for method output_type
	6, // [6:6] is the sub-list for method input_type
	6, // [6:6] is the sub-list for extension type_name
	6, // [6:6] is the sub-list for extension extendee
	0, // [0:6] is the sub-list for field type_name
}

func init() { file_kythe_proto_metadata_proto_init() }
func file_kythe_proto_metadata_proto_init() {
	if File_kythe_proto_metadata_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_metadata_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*GeneratedCodeInfo); i {
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
		file_kythe_proto_metadata_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*MappingRule); i {
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
			RawDescriptor: file_kythe_proto_metadata_proto_rawDesc,
			NumEnums:      3,
			NumMessages:   2,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_proto_metadata_proto_goTypes,
		DependencyIndexes: file_kythe_proto_metadata_proto_depIdxs,
		EnumInfos:         file_kythe_proto_metadata_proto_enumTypes,
		MessageInfos:      file_kythe_proto_metadata_proto_msgTypes,
	}.Build()
	File_kythe_proto_metadata_proto = out.File
	file_kythe_proto_metadata_proto_rawDesc = nil
	file_kythe_proto_metadata_proto_goTypes = nil
	file_kythe_proto_metadata_proto_depIdxs = nil
}
