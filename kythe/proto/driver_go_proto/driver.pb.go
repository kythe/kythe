// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.25.0
// 	protoc        v3.15.3
// source: kythe/proto/driver.proto

package driver_go_proto

import (
	proto "github.com/golang/protobuf/proto"
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	analysis_go_proto "kythe.io/kythe/proto/analysis_go_proto"
	common_go_proto "kythe.io/kythe/proto/common_go_proto"
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

// This is a compile-time assertion that a sufficiently up-to-date version
// of the legacy proto package is being used.
const _ = proto.ProtoPackageIsVersion4

type InitRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Protocol string `protobuf:"bytes,1,opt,name=protocol,proto3" json:"protocol,omitempty"`
}

func (x *InitRequest) Reset() {
	*x = InitRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *InitRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*InitRequest) ProtoMessage() {}

func (x *InitRequest) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use InitRequest.ProtoReflect.Descriptor instead.
func (*InitRequest) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{0}
}

func (x *InitRequest) GetProtocol() string {
	if x != nil {
		return x.Protocol
	}
	return ""
}

type InitReply struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Protocol string `protobuf:"bytes,1,opt,name=protocol,proto3" json:"protocol,omitempty"`
}

func (x *InitReply) Reset() {
	*x = InitReply{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *InitReply) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*InitReply) ProtoMessage() {}

func (x *InitReply) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use InitReply.ProtoReflect.Descriptor instead.
func (*InitReply) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{1}
}

func (x *InitReply) GetProtocol() string {
	if x != nil {
		return x.Protocol
	}
	return ""
}

type AnalyzeRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Language string `protobuf:"bytes,1,opt,name=language,proto3" json:"language,omitempty"`
}

func (x *AnalyzeRequest) Reset() {
	*x = AnalyzeRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *AnalyzeRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*AnalyzeRequest) ProtoMessage() {}

func (x *AnalyzeRequest) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use AnalyzeRequest.ProtoReflect.Descriptor instead.
func (*AnalyzeRequest) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{2}
}

func (x *AnalyzeRequest) GetLanguage() string {
	if x != nil {
		return x.Language
	}
	return ""
}

type AnalyzeReply struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Id   int64                              `protobuf:"varint,1,opt,name=id,proto3" json:"id,omitempty"`
	Unit *analysis_go_proto.CompilationUnit `protobuf:"bytes,2,opt,name=unit,proto3" json:"unit,omitempty"`
}

func (x *AnalyzeReply) Reset() {
	*x = AnalyzeReply{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *AnalyzeReply) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*AnalyzeReply) ProtoMessage() {}

func (x *AnalyzeReply) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use AnalyzeReply.ProtoReflect.Descriptor instead.
func (*AnalyzeReply) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{3}
}

func (x *AnalyzeReply) GetId() int64 {
	if x != nil {
		return x.Id
	}
	return 0
}

func (x *AnalyzeReply) GetUnit() *analysis_go_proto.CompilationUnit {
	if x != nil {
		return x.Unit
	}
	return nil
}

type FileRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Id     int64  `protobuf:"varint,1,opt,name=id,proto3" json:"id,omitempty"`
	Path   string `protobuf:"bytes,2,opt,name=path,proto3" json:"path,omitempty"`
	Digest string `protobuf:"bytes,3,opt,name=digest,proto3" json:"digest,omitempty"`
}

func (x *FileRequest) Reset() {
	*x = FileRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[4]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *FileRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*FileRequest) ProtoMessage() {}

func (x *FileRequest) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[4]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use FileRequest.ProtoReflect.Descriptor instead.
func (*FileRequest) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{4}
}

func (x *FileRequest) GetId() int64 {
	if x != nil {
		return x.Id
	}
	return 0
}

func (x *FileRequest) GetPath() string {
	if x != nil {
		return x.Path
	}
	return ""
}

func (x *FileRequest) GetDigest() string {
	if x != nil {
		return x.Digest
	}
	return ""
}

type FileReply struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Path   string `protobuf:"bytes,1,opt,name=path,proto3" json:"path,omitempty"`
	Digest string `protobuf:"bytes,2,opt,name=digest,proto3" json:"digest,omitempty"`
	Data   []byte `protobuf:"bytes,3,opt,name=data,proto3" json:"data,omitempty"`
}

func (x *FileReply) Reset() {
	*x = FileReply{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[5]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *FileReply) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*FileReply) ProtoMessage() {}

func (x *FileReply) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[5]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use FileReply.ProtoReflect.Descriptor instead.
func (*FileReply) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{5}
}

func (x *FileReply) GetPath() string {
	if x != nil {
		return x.Path
	}
	return ""
}

func (x *FileReply) GetDigest() string {
	if x != nil {
		return x.Digest
	}
	return ""
}

func (x *FileReply) GetData() []byte {
	if x != nil {
		return x.Data
	}
	return nil
}

type OutRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Id      int64                     `protobuf:"varint,1,opt,name=id,proto3" json:"id,omitempty"`
	Output  [][]byte                  `protobuf:"bytes,2,rep,name=output,proto3" json:"output,omitempty"`
	Entries []*storage_go_proto.Entry `protobuf:"bytes,3,rep,name=entries,proto3" json:"entries,omitempty"`
}

func (x *OutRequest) Reset() {
	*x = OutRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[6]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *OutRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*OutRequest) ProtoMessage() {}

func (x *OutRequest) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[6]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use OutRequest.ProtoReflect.Descriptor instead.
func (*OutRequest) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{6}
}

func (x *OutRequest) GetId() int64 {
	if x != nil {
		return x.Id
	}
	return 0
}

func (x *OutRequest) GetOutput() [][]byte {
	if x != nil {
		return x.Output
	}
	return nil
}

func (x *OutRequest) GetEntries() []*storage_go_proto.Entry {
	if x != nil {
		return x.Entries
	}
	return nil
}

type LogRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Id      int64                       `protobuf:"varint,1,opt,name=id,proto3" json:"id,omitempty"`
	Message *common_go_proto.Diagnostic `protobuf:"bytes,2,opt,name=message,proto3" json:"message,omitempty"`
}

func (x *LogRequest) Reset() {
	*x = LogRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_driver_proto_msgTypes[7]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *LogRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*LogRequest) ProtoMessage() {}

func (x *LogRequest) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_driver_proto_msgTypes[7]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use LogRequest.ProtoReflect.Descriptor instead.
func (*LogRequest) Descriptor() ([]byte, []int) {
	return file_kythe_proto_driver_proto_rawDescGZIP(), []int{7}
}

func (x *LogRequest) GetId() int64 {
	if x != nil {
		return x.Id
	}
	return 0
}

func (x *LogRequest) GetMessage() *common_go_proto.Diagnostic {
	if x != nil {
		return x.Message
	}
	return nil
}

var File_kythe_proto_driver_proto protoreflect.FileDescriptor

var file_kythe_proto_driver_proto_rawDesc = []byte{
	0x0a, 0x18, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x64, 0x72,
	0x69, 0x76, 0x65, 0x72, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0c, 0x6b, 0x79, 0x74, 0x68,
	0x65, 0x2e, 0x64, 0x72, 0x69, 0x76, 0x65, 0x72, 0x1a, 0x1a, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x61, 0x6e, 0x61, 0x6c, 0x79, 0x73, 0x69, 0x73, 0x2e, 0x70,
	0x72, 0x6f, 0x74, 0x6f, 0x1a, 0x18, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x2f, 0x63, 0x6f, 0x6d, 0x6d, 0x6f, 0x6e, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x1a, 0x19,
	0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x73, 0x74, 0x6f, 0x72,
	0x61, 0x67, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22, 0x29, 0x0a, 0x0b, 0x49, 0x6e, 0x69,
	0x74, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12, 0x1a, 0x0a, 0x08, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x63, 0x6f, 0x6c, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x63, 0x6f, 0x6c, 0x22, 0x27, 0x0a, 0x09, 0x49, 0x6e, 0x69, 0x74, 0x52, 0x65, 0x70, 0x6c,
	0x79, 0x12, 0x1a, 0x0a, 0x08, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f, 0x6c, 0x18, 0x01, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x08, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f, 0x6c, 0x22, 0x2c, 0x0a,
	0x0e, 0x41, 0x6e, 0x61, 0x6c, 0x79, 0x7a, 0x65, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12,
	0x1a, 0x0a, 0x08, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x18, 0x01, 0x20, 0x01, 0x28,
	0x09, 0x52, 0x08, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x22, 0x50, 0x0a, 0x0c, 0x41,
	0x6e, 0x61, 0x6c, 0x79, 0x7a, 0x65, 0x52, 0x65, 0x70, 0x6c, 0x79, 0x12, 0x0e, 0x0a, 0x02, 0x69,
	0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x03, 0x52, 0x02, 0x69, 0x64, 0x12, 0x30, 0x0a, 0x04, 0x75,
	0x6e, 0x69, 0x74, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x1c, 0x2e, 0x6b, 0x79, 0x74, 0x68,
	0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61, 0x74,
	0x69, 0x6f, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x52, 0x04, 0x75, 0x6e, 0x69, 0x74, 0x22, 0x49, 0x0a,
	0x0b, 0x46, 0x69, 0x6c, 0x65, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12, 0x0e, 0x0a, 0x02,
	0x69, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x03, 0x52, 0x02, 0x69, 0x64, 0x12, 0x12, 0x0a, 0x04,
	0x70, 0x61, 0x74, 0x68, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x70, 0x61, 0x74, 0x68,
	0x12, 0x16, 0x0a, 0x06, 0x64, 0x69, 0x67, 0x65, 0x73, 0x74, 0x18, 0x03, 0x20, 0x01, 0x28, 0x09,
	0x52, 0x06, 0x64, 0x69, 0x67, 0x65, 0x73, 0x74, 0x22, 0x4b, 0x0a, 0x09, 0x46, 0x69, 0x6c, 0x65,
	0x52, 0x65, 0x70, 0x6c, 0x79, 0x12, 0x12, 0x0a, 0x04, 0x70, 0x61, 0x74, 0x68, 0x18, 0x01, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x04, 0x70, 0x61, 0x74, 0x68, 0x12, 0x16, 0x0a, 0x06, 0x64, 0x69, 0x67,
	0x65, 0x73, 0x74, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x06, 0x64, 0x69, 0x67, 0x65, 0x73,
	0x74, 0x12, 0x12, 0x0a, 0x04, 0x64, 0x61, 0x74, 0x61, 0x18, 0x03, 0x20, 0x01, 0x28, 0x0c, 0x52,
	0x04, 0x64, 0x61, 0x74, 0x61, 0x22, 0x62, 0x0a, 0x0a, 0x4f, 0x75, 0x74, 0x52, 0x65, 0x71, 0x75,
	0x65, 0x73, 0x74, 0x12, 0x0e, 0x0a, 0x02, 0x69, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x03, 0x52,
	0x02, 0x69, 0x64, 0x12, 0x16, 0x0a, 0x06, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x18, 0x02, 0x20,
	0x03, 0x28, 0x0c, 0x52, 0x06, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x12, 0x2c, 0x0a, 0x07, 0x65,
	0x6e, 0x74, 0x72, 0x69, 0x65, 0x73, 0x18, 0x03, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x12, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x45, 0x6e, 0x74, 0x72, 0x79,
	0x52, 0x07, 0x65, 0x6e, 0x74, 0x72, 0x69, 0x65, 0x73, 0x22, 0x56, 0x0a, 0x0a, 0x4c, 0x6f, 0x67,
	0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12, 0x0e, 0x0a, 0x02, 0x69, 0x64, 0x18, 0x01, 0x20,
	0x01, 0x28, 0x03, 0x52, 0x02, 0x69, 0x64, 0x12, 0x38, 0x0a, 0x07, 0x6d, 0x65, 0x73, 0x73, 0x61,
	0x67, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x1e, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65,
	0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x63, 0x6f, 0x6d, 0x6d, 0x6f, 0x6e, 0x2e, 0x44, 0x69,
	0x61, 0x67, 0x6e, 0x6f, 0x73, 0x74, 0x69, 0x63, 0x52, 0x07, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67,
	0x65, 0x42, 0x11, 0x5a, 0x0f, 0x64, 0x72, 0x69, 0x76, 0x65, 0x72, 0x5f, 0x67, 0x6f, 0x5f, 0x70,
	0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_kythe_proto_driver_proto_rawDescOnce sync.Once
	file_kythe_proto_driver_proto_rawDescData = file_kythe_proto_driver_proto_rawDesc
)

func file_kythe_proto_driver_proto_rawDescGZIP() []byte {
	file_kythe_proto_driver_proto_rawDescOnce.Do(func() {
		file_kythe_proto_driver_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_driver_proto_rawDescData)
	})
	return file_kythe_proto_driver_proto_rawDescData
}

var file_kythe_proto_driver_proto_msgTypes = make([]protoimpl.MessageInfo, 8)
var file_kythe_proto_driver_proto_goTypes = []interface{}{
	(*InitRequest)(nil),                       // 0: kythe.driver.InitRequest
	(*InitReply)(nil),                         // 1: kythe.driver.InitReply
	(*AnalyzeRequest)(nil),                    // 2: kythe.driver.AnalyzeRequest
	(*AnalyzeReply)(nil),                      // 3: kythe.driver.AnalyzeReply
	(*FileRequest)(nil),                       // 4: kythe.driver.FileRequest
	(*FileReply)(nil),                         // 5: kythe.driver.FileReply
	(*OutRequest)(nil),                        // 6: kythe.driver.OutRequest
	(*LogRequest)(nil),                        // 7: kythe.driver.LogRequest
	(*analysis_go_proto.CompilationUnit)(nil), // 8: kythe.proto.CompilationUnit
	(*storage_go_proto.Entry)(nil),            // 9: kythe.proto.Entry
	(*common_go_proto.Diagnostic)(nil),        // 10: kythe.proto.common.Diagnostic
}
var file_kythe_proto_driver_proto_depIdxs = []int32{
	8,  // 0: kythe.driver.AnalyzeReply.unit:type_name -> kythe.proto.CompilationUnit
	9,  // 1: kythe.driver.OutRequest.entries:type_name -> kythe.proto.Entry
	10, // 2: kythe.driver.LogRequest.message:type_name -> kythe.proto.common.Diagnostic
	3,  // [3:3] is the sub-list for method output_type
	3,  // [3:3] is the sub-list for method input_type
	3,  // [3:3] is the sub-list for extension type_name
	3,  // [3:3] is the sub-list for extension extendee
	0,  // [0:3] is the sub-list for field type_name
}

func init() { file_kythe_proto_driver_proto_init() }
func file_kythe_proto_driver_proto_init() {
	if File_kythe_proto_driver_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_driver_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*InitRequest); i {
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
		file_kythe_proto_driver_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*InitReply); i {
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
		file_kythe_proto_driver_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*AnalyzeRequest); i {
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
		file_kythe_proto_driver_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*AnalyzeReply); i {
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
		file_kythe_proto_driver_proto_msgTypes[4].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*FileRequest); i {
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
		file_kythe_proto_driver_proto_msgTypes[5].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*FileReply); i {
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
		file_kythe_proto_driver_proto_msgTypes[6].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*OutRequest); i {
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
		file_kythe_proto_driver_proto_msgTypes[7].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*LogRequest); i {
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
			RawDescriptor: file_kythe_proto_driver_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   8,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_proto_driver_proto_goTypes,
		DependencyIndexes: file_kythe_proto_driver_proto_depIdxs,
		MessageInfos:      file_kythe_proto_driver_proto_msgTypes,
	}.Build()
	File_kythe_proto_driver_proto = out.File
	file_kythe_proto_driver_proto_rawDesc = nil
	file_kythe_proto_driver_proto_goTypes = nil
	file_kythe_proto_driver_proto_depIdxs = nil
}
