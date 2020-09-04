// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.22.0
// 	protoc        v3.13.0
// source: kythe/proto/cxx.proto

package cxx_go_proto

import (
	proto "github.com/golang/protobuf/proto"
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

// This is a compile-time assertion that a sufficiently up-to-date version
// of the legacy proto package is being used.
const _ = proto.ProtoPackageIsVersion4

type CxxCompilationUnitDetails struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	HeaderSearchInfo   *CxxCompilationUnitDetails_HeaderSearchInfo     `protobuf:"bytes,1,opt,name=header_search_info,json=headerSearchInfo,proto3" json:"header_search_info,omitempty"`
	SystemHeaderPrefix []*CxxCompilationUnitDetails_SystemHeaderPrefix `protobuf:"bytes,2,rep,name=system_header_prefix,json=systemHeaderPrefix,proto3" json:"system_header_prefix,omitempty"`
	StatPath           []*CxxCompilationUnitDetails_StatPath           `protobuf:"bytes,3,rep,name=stat_path,json=statPath,proto3" json:"stat_path,omitempty"`
}

func (x *CxxCompilationUnitDetails) Reset() {
	*x = CxxCompilationUnitDetails{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_cxx_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *CxxCompilationUnitDetails) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*CxxCompilationUnitDetails) ProtoMessage() {}

func (x *CxxCompilationUnitDetails) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_cxx_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use CxxCompilationUnitDetails.ProtoReflect.Descriptor instead.
func (*CxxCompilationUnitDetails) Descriptor() ([]byte, []int) {
	return file_kythe_proto_cxx_proto_rawDescGZIP(), []int{0}
}

func (x *CxxCompilationUnitDetails) GetHeaderSearchInfo() *CxxCompilationUnitDetails_HeaderSearchInfo {
	if x != nil {
		return x.HeaderSearchInfo
	}
	return nil
}

func (x *CxxCompilationUnitDetails) GetSystemHeaderPrefix() []*CxxCompilationUnitDetails_SystemHeaderPrefix {
	if x != nil {
		return x.SystemHeaderPrefix
	}
	return nil
}

func (x *CxxCompilationUnitDetails) GetStatPath() []*CxxCompilationUnitDetails_StatPath {
	if x != nil {
		return x.StatPath
	}
	return nil
}

type CxxCompilationUnitDetails_HeaderSearchDir struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Path               string `protobuf:"bytes,1,opt,name=path,proto3" json:"path,omitempty"`
	CharacteristicKind int32  `protobuf:"varint,2,opt,name=characteristic_kind,json=characteristicKind,proto3" json:"characteristic_kind,omitempty"`
	IsFramework        bool   `protobuf:"varint,3,opt,name=is_framework,json=isFramework,proto3" json:"is_framework,omitempty"`
}

func (x *CxxCompilationUnitDetails_HeaderSearchDir) Reset() {
	*x = CxxCompilationUnitDetails_HeaderSearchDir{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_cxx_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *CxxCompilationUnitDetails_HeaderSearchDir) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*CxxCompilationUnitDetails_HeaderSearchDir) ProtoMessage() {}

func (x *CxxCompilationUnitDetails_HeaderSearchDir) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_cxx_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use CxxCompilationUnitDetails_HeaderSearchDir.ProtoReflect.Descriptor instead.
func (*CxxCompilationUnitDetails_HeaderSearchDir) Descriptor() ([]byte, []int) {
	return file_kythe_proto_cxx_proto_rawDescGZIP(), []int{0, 0}
}

func (x *CxxCompilationUnitDetails_HeaderSearchDir) GetPath() string {
	if x != nil {
		return x.Path
	}
	return ""
}

func (x *CxxCompilationUnitDetails_HeaderSearchDir) GetCharacteristicKind() int32 {
	if x != nil {
		return x.CharacteristicKind
	}
	return 0
}

func (x *CxxCompilationUnitDetails_HeaderSearchDir) GetIsFramework() bool {
	if x != nil {
		return x.IsFramework
	}
	return false
}

type CxxCompilationUnitDetails_HeaderSearchInfo struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	FirstAngledDir int32                                        `protobuf:"varint,1,opt,name=first_angled_dir,json=firstAngledDir,proto3" json:"first_angled_dir,omitempty"`
	FirstSystemDir int32                                        `protobuf:"varint,2,opt,name=first_system_dir,json=firstSystemDir,proto3" json:"first_system_dir,omitempty"`
	Dir            []*CxxCompilationUnitDetails_HeaderSearchDir `protobuf:"bytes,3,rep,name=dir,proto3" json:"dir,omitempty"`
}

func (x *CxxCompilationUnitDetails_HeaderSearchInfo) Reset() {
	*x = CxxCompilationUnitDetails_HeaderSearchInfo{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_cxx_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *CxxCompilationUnitDetails_HeaderSearchInfo) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*CxxCompilationUnitDetails_HeaderSearchInfo) ProtoMessage() {}

func (x *CxxCompilationUnitDetails_HeaderSearchInfo) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_cxx_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use CxxCompilationUnitDetails_HeaderSearchInfo.ProtoReflect.Descriptor instead.
func (*CxxCompilationUnitDetails_HeaderSearchInfo) Descriptor() ([]byte, []int) {
	return file_kythe_proto_cxx_proto_rawDescGZIP(), []int{0, 1}
}

func (x *CxxCompilationUnitDetails_HeaderSearchInfo) GetFirstAngledDir() int32 {
	if x != nil {
		return x.FirstAngledDir
	}
	return 0
}

func (x *CxxCompilationUnitDetails_HeaderSearchInfo) GetFirstSystemDir() int32 {
	if x != nil {
		return x.FirstSystemDir
	}
	return 0
}

func (x *CxxCompilationUnitDetails_HeaderSearchInfo) GetDir() []*CxxCompilationUnitDetails_HeaderSearchDir {
	if x != nil {
		return x.Dir
	}
	return nil
}

type CxxCompilationUnitDetails_SystemHeaderPrefix struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Prefix         string `protobuf:"bytes,1,opt,name=prefix,proto3" json:"prefix,omitempty"`
	IsSystemHeader bool   `protobuf:"varint,2,opt,name=is_system_header,json=isSystemHeader,proto3" json:"is_system_header,omitempty"`
}

func (x *CxxCompilationUnitDetails_SystemHeaderPrefix) Reset() {
	*x = CxxCompilationUnitDetails_SystemHeaderPrefix{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_cxx_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *CxxCompilationUnitDetails_SystemHeaderPrefix) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*CxxCompilationUnitDetails_SystemHeaderPrefix) ProtoMessage() {}

func (x *CxxCompilationUnitDetails_SystemHeaderPrefix) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_cxx_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use CxxCompilationUnitDetails_SystemHeaderPrefix.ProtoReflect.Descriptor instead.
func (*CxxCompilationUnitDetails_SystemHeaderPrefix) Descriptor() ([]byte, []int) {
	return file_kythe_proto_cxx_proto_rawDescGZIP(), []int{0, 2}
}

func (x *CxxCompilationUnitDetails_SystemHeaderPrefix) GetPrefix() string {
	if x != nil {
		return x.Prefix
	}
	return ""
}

func (x *CxxCompilationUnitDetails_SystemHeaderPrefix) GetIsSystemHeader() bool {
	if x != nil {
		return x.IsSystemHeader
	}
	return false
}

type CxxCompilationUnitDetails_StatPath struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Path string `protobuf:"bytes,1,opt,name=path,proto3" json:"path,omitempty"`
}

func (x *CxxCompilationUnitDetails_StatPath) Reset() {
	*x = CxxCompilationUnitDetails_StatPath{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_cxx_proto_msgTypes[4]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *CxxCompilationUnitDetails_StatPath) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*CxxCompilationUnitDetails_StatPath) ProtoMessage() {}

func (x *CxxCompilationUnitDetails_StatPath) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_cxx_proto_msgTypes[4]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use CxxCompilationUnitDetails_StatPath.ProtoReflect.Descriptor instead.
func (*CxxCompilationUnitDetails_StatPath) Descriptor() ([]byte, []int) {
	return file_kythe_proto_cxx_proto_rawDescGZIP(), []int{0, 3}
}

func (x *CxxCompilationUnitDetails_StatPath) GetPath() string {
	if x != nil {
		return x.Path
	}
	return ""
}

var File_kythe_proto_cxx_proto protoreflect.FileDescriptor

var file_kythe_proto_cxx_proto_rawDesc = []byte{
	0x0a, 0x15, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x63, 0x78,
	0x78, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0b, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70,
	0x72, 0x6f, 0x74, 0x6f, 0x22, 0xe3, 0x05, 0x0a, 0x19, 0x43, 0x78, 0x78, 0x43, 0x6f, 0x6d, 0x70,
	0x69, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x44, 0x65, 0x74, 0x61, 0x69,
	0x6c, 0x73, 0x12, 0x65, 0x0a, 0x12, 0x68, 0x65, 0x61, 0x64, 0x65, 0x72, 0x5f, 0x73, 0x65, 0x61,
	0x72, 0x63, 0x68, 0x5f, 0x69, 0x6e, 0x66, 0x6f, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x37,
	0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x78, 0x78,
	0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x44,
	0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x2e, 0x48, 0x65, 0x61, 0x64, 0x65, 0x72, 0x53, 0x65, 0x61,
	0x72, 0x63, 0x68, 0x49, 0x6e, 0x66, 0x6f, 0x52, 0x10, 0x68, 0x65, 0x61, 0x64, 0x65, 0x72, 0x53,
	0x65, 0x61, 0x72, 0x63, 0x68, 0x49, 0x6e, 0x66, 0x6f, 0x12, 0x6b, 0x0a, 0x14, 0x73, 0x79, 0x73,
	0x74, 0x65, 0x6d, 0x5f, 0x68, 0x65, 0x61, 0x64, 0x65, 0x72, 0x5f, 0x70, 0x72, 0x65, 0x66, 0x69,
	0x78, 0x18, 0x02, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x39, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x78, 0x78, 0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61,
	0x74, 0x69, 0x6f, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x44, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x2e,
	0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x48, 0x65, 0x61, 0x64, 0x65, 0x72, 0x50, 0x72, 0x65, 0x66,
	0x69, 0x78, 0x52, 0x12, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x48, 0x65, 0x61, 0x64, 0x65, 0x72,
	0x50, 0x72, 0x65, 0x66, 0x69, 0x78, 0x12, 0x4c, 0x0a, 0x09, 0x73, 0x74, 0x61, 0x74, 0x5f, 0x70,
	0x61, 0x74, 0x68, 0x18, 0x03, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x2f, 0x2e, 0x6b, 0x79, 0x74, 0x68,
	0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x78, 0x78, 0x43, 0x6f, 0x6d, 0x70, 0x69,
	0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x44, 0x65, 0x74, 0x61, 0x69, 0x6c,
	0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x50, 0x61, 0x74, 0x68, 0x52, 0x08, 0x73, 0x74, 0x61, 0x74,
	0x50, 0x61, 0x74, 0x68, 0x1a, 0x79, 0x0a, 0x0f, 0x48, 0x65, 0x61, 0x64, 0x65, 0x72, 0x53, 0x65,
	0x61, 0x72, 0x63, 0x68, 0x44, 0x69, 0x72, 0x12, 0x12, 0x0a, 0x04, 0x70, 0x61, 0x74, 0x68, 0x18,
	0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x70, 0x61, 0x74, 0x68, 0x12, 0x2f, 0x0a, 0x13, 0x63,
	0x68, 0x61, 0x72, 0x61, 0x63, 0x74, 0x65, 0x72, 0x69, 0x73, 0x74, 0x69, 0x63, 0x5f, 0x6b, 0x69,
	0x6e, 0x64, 0x18, 0x02, 0x20, 0x01, 0x28, 0x05, 0x52, 0x12, 0x63, 0x68, 0x61, 0x72, 0x61, 0x63,
	0x74, 0x65, 0x72, 0x69, 0x73, 0x74, 0x69, 0x63, 0x4b, 0x69, 0x6e, 0x64, 0x12, 0x21, 0x0a, 0x0c,
	0x69, 0x73, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x77, 0x6f, 0x72, 0x6b, 0x18, 0x03, 0x20, 0x01,
	0x28, 0x08, 0x52, 0x0b, 0x69, 0x73, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x77, 0x6f, 0x72, 0x6b, 0x1a,
	0xb0, 0x01, 0x0a, 0x10, 0x48, 0x65, 0x61, 0x64, 0x65, 0x72, 0x53, 0x65, 0x61, 0x72, 0x63, 0x68,
	0x49, 0x6e, 0x66, 0x6f, 0x12, 0x28, 0x0a, 0x10, 0x66, 0x69, 0x72, 0x73, 0x74, 0x5f, 0x61, 0x6e,
	0x67, 0x6c, 0x65, 0x64, 0x5f, 0x64, 0x69, 0x72, 0x18, 0x01, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0e,
	0x66, 0x69, 0x72, 0x73, 0x74, 0x41, 0x6e, 0x67, 0x6c, 0x65, 0x64, 0x44, 0x69, 0x72, 0x12, 0x28,
	0x0a, 0x10, 0x66, 0x69, 0x72, 0x73, 0x74, 0x5f, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x5f, 0x64,
	0x69, 0x72, 0x18, 0x02, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0e, 0x66, 0x69, 0x72, 0x73, 0x74, 0x53,
	0x79, 0x73, 0x74, 0x65, 0x6d, 0x44, 0x69, 0x72, 0x12, 0x48, 0x0a, 0x03, 0x64, 0x69, 0x72, 0x18,
	0x03, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x36, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x78, 0x78, 0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61, 0x74, 0x69,
	0x6f, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x44, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x2e, 0x48, 0x65,
	0x61, 0x64, 0x65, 0x72, 0x53, 0x65, 0x61, 0x72, 0x63, 0x68, 0x44, 0x69, 0x72, 0x52, 0x03, 0x64,
	0x69, 0x72, 0x1a, 0x56, 0x0a, 0x12, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x48, 0x65, 0x61, 0x64,
	0x65, 0x72, 0x50, 0x72, 0x65, 0x66, 0x69, 0x78, 0x12, 0x16, 0x0a, 0x06, 0x70, 0x72, 0x65, 0x66,
	0x69, 0x78, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x06, 0x70, 0x72, 0x65, 0x66, 0x69, 0x78,
	0x12, 0x28, 0x0a, 0x10, 0x69, 0x73, 0x5f, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x5f, 0x68, 0x65,
	0x61, 0x64, 0x65, 0x72, 0x18, 0x02, 0x20, 0x01, 0x28, 0x08, 0x52, 0x0e, 0x69, 0x73, 0x53, 0x79,
	0x73, 0x74, 0x65, 0x6d, 0x48, 0x65, 0x61, 0x64, 0x65, 0x72, 0x1a, 0x1e, 0x0a, 0x08, 0x53, 0x74,
	0x61, 0x74, 0x50, 0x61, 0x74, 0x68, 0x12, 0x12, 0x0a, 0x04, 0x70, 0x61, 0x74, 0x68, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x70, 0x61, 0x74, 0x68, 0x42, 0x2f, 0x0a, 0x1f, 0x63, 0x6f,
	0x6d, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x76, 0x74, 0x6f, 0x6f, 0x6c,
	0x73, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x5a, 0x0c, 0x63,
	0x78, 0x78, 0x5f, 0x67, 0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x33,
}

var (
	file_kythe_proto_cxx_proto_rawDescOnce sync.Once
	file_kythe_proto_cxx_proto_rawDescData = file_kythe_proto_cxx_proto_rawDesc
)

func file_kythe_proto_cxx_proto_rawDescGZIP() []byte {
	file_kythe_proto_cxx_proto_rawDescOnce.Do(func() {
		file_kythe_proto_cxx_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_cxx_proto_rawDescData)
	})
	return file_kythe_proto_cxx_proto_rawDescData
}

var file_kythe_proto_cxx_proto_msgTypes = make([]protoimpl.MessageInfo, 5)
var file_kythe_proto_cxx_proto_goTypes = []interface{}{
	(*CxxCompilationUnitDetails)(nil),                    // 0: kythe.proto.CxxCompilationUnitDetails
	(*CxxCompilationUnitDetails_HeaderSearchDir)(nil),    // 1: kythe.proto.CxxCompilationUnitDetails.HeaderSearchDir
	(*CxxCompilationUnitDetails_HeaderSearchInfo)(nil),   // 2: kythe.proto.CxxCompilationUnitDetails.HeaderSearchInfo
	(*CxxCompilationUnitDetails_SystemHeaderPrefix)(nil), // 3: kythe.proto.CxxCompilationUnitDetails.SystemHeaderPrefix
	(*CxxCompilationUnitDetails_StatPath)(nil),           // 4: kythe.proto.CxxCompilationUnitDetails.StatPath
}
var file_kythe_proto_cxx_proto_depIdxs = []int32{
	2, // 0: kythe.proto.CxxCompilationUnitDetails.header_search_info:type_name -> kythe.proto.CxxCompilationUnitDetails.HeaderSearchInfo
	3, // 1: kythe.proto.CxxCompilationUnitDetails.system_header_prefix:type_name -> kythe.proto.CxxCompilationUnitDetails.SystemHeaderPrefix
	4, // 2: kythe.proto.CxxCompilationUnitDetails.stat_path:type_name -> kythe.proto.CxxCompilationUnitDetails.StatPath
	1, // 3: kythe.proto.CxxCompilationUnitDetails.HeaderSearchInfo.dir:type_name -> kythe.proto.CxxCompilationUnitDetails.HeaderSearchDir
	4, // [4:4] is the sub-list for method output_type
	4, // [4:4] is the sub-list for method input_type
	4, // [4:4] is the sub-list for extension type_name
	4, // [4:4] is the sub-list for extension extendee
	0, // [0:4] is the sub-list for field type_name
}

func init() { file_kythe_proto_cxx_proto_init() }
func file_kythe_proto_cxx_proto_init() {
	if File_kythe_proto_cxx_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_cxx_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*CxxCompilationUnitDetails); i {
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
		file_kythe_proto_cxx_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*CxxCompilationUnitDetails_HeaderSearchDir); i {
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
		file_kythe_proto_cxx_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*CxxCompilationUnitDetails_HeaderSearchInfo); i {
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
		file_kythe_proto_cxx_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*CxxCompilationUnitDetails_SystemHeaderPrefix); i {
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
		file_kythe_proto_cxx_proto_msgTypes[4].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*CxxCompilationUnitDetails_StatPath); i {
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
			RawDescriptor: file_kythe_proto_cxx_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   5,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_proto_cxx_proto_goTypes,
		DependencyIndexes: file_kythe_proto_cxx_proto_depIdxs,
		MessageInfos:      file_kythe_proto_cxx_proto_msgTypes,
	}.Build()
	File_kythe_proto_cxx_proto = out.File
	file_kythe_proto_cxx_proto_rawDesc = nil
	file_kythe_proto_cxx_proto_goTypes = nil
	file_kythe_proto_cxx_proto_depIdxs = nil
}
