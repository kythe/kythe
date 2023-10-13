// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.31.0
// 	protoc        v4.24.2
// source: kythe/proto/link.proto

package link_go_proto

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	common_go_proto "kythe.io/kythe/proto/common_go_proto"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type LinkRequest_DefinitionKind int32

const (
	LinkRequest_BINDING LinkRequest_DefinitionKind = 0
	LinkRequest_FULL    LinkRequest_DefinitionKind = 1
	LinkRequest_ANY     LinkRequest_DefinitionKind = 2
)

// Enum value maps for LinkRequest_DefinitionKind.
var (
	LinkRequest_DefinitionKind_name = map[int32]string{
		0: "BINDING",
		1: "FULL",
		2: "ANY",
	}
	LinkRequest_DefinitionKind_value = map[string]int32{
		"BINDING": 0,
		"FULL":    1,
		"ANY":     2,
	}
)

func (x LinkRequest_DefinitionKind) Enum() *LinkRequest_DefinitionKind {
	p := new(LinkRequest_DefinitionKind)
	*p = x
	return p
}

func (x LinkRequest_DefinitionKind) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (LinkRequest_DefinitionKind) Descriptor() protoreflect.EnumDescriptor {
	return file_kythe_proto_link_proto_enumTypes[0].Descriptor()
}

func (LinkRequest_DefinitionKind) Type() protoreflect.EnumType {
	return &file_kythe_proto_link_proto_enumTypes[0]
}

func (x LinkRequest_DefinitionKind) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use LinkRequest_DefinitionKind.Descriptor instead.
func (LinkRequest_DefinitionKind) EnumDescriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{0, 0}
}

type LinkRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Identifier     string                     `protobuf:"bytes,1,opt,name=identifier,proto3" json:"identifier,omitempty"`
	Corpus         []string                   `protobuf:"bytes,2,rep,name=corpus,proto3" json:"corpus,omitempty"`
	Language       []string                   `protobuf:"bytes,3,rep,name=language,proto3" json:"language,omitempty"`
	NodeKind       []string                   `protobuf:"bytes,4,rep,name=node_kind,json=nodeKind,proto3" json:"node_kind,omitempty"`
	Include        []*LinkRequest_Location    `protobuf:"bytes,5,rep,name=include,proto3" json:"include,omitempty"`
	Exclude        []*LinkRequest_Location    `protobuf:"bytes,6,rep,name=exclude,proto3" json:"exclude,omitempty"`
	Params         *LinkRequest_Params        `protobuf:"bytes,7,opt,name=params,proto3" json:"params,omitempty"`
	DefinitionKind LinkRequest_DefinitionKind `protobuf:"varint,8,opt,name=definition_kind,json=definitionKind,proto3,enum=kythe.proto.LinkRequest_DefinitionKind" json:"definition_kind,omitempty"`
	IncludeNodes   bool                       `protobuf:"varint,9,opt,name=include_nodes,json=includeNodes,proto3" json:"include_nodes,omitempty"`
}

func (x *LinkRequest) Reset() {
	*x = LinkRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_link_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *LinkRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*LinkRequest) ProtoMessage() {}

func (x *LinkRequest) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_link_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use LinkRequest.ProtoReflect.Descriptor instead.
func (*LinkRequest) Descriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{0}
}

func (x *LinkRequest) GetIdentifier() string {
	if x != nil {
		return x.Identifier
	}
	return ""
}

func (x *LinkRequest) GetCorpus() []string {
	if x != nil {
		return x.Corpus
	}
	return nil
}

func (x *LinkRequest) GetLanguage() []string {
	if x != nil {
		return x.Language
	}
	return nil
}

func (x *LinkRequest) GetNodeKind() []string {
	if x != nil {
		return x.NodeKind
	}
	return nil
}

func (x *LinkRequest) GetInclude() []*LinkRequest_Location {
	if x != nil {
		return x.Include
	}
	return nil
}

func (x *LinkRequest) GetExclude() []*LinkRequest_Location {
	if x != nil {
		return x.Exclude
	}
	return nil
}

func (x *LinkRequest) GetParams() *LinkRequest_Params {
	if x != nil {
		return x.Params
	}
	return nil
}

func (x *LinkRequest) GetDefinitionKind() LinkRequest_DefinitionKind {
	if x != nil {
		return x.DefinitionKind
	}
	return LinkRequest_BINDING
}

func (x *LinkRequest) GetIncludeNodes() bool {
	if x != nil {
		return x.IncludeNodes
	}
	return false
}

type Link struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	FileTicket string                `protobuf:"bytes,1,opt,name=file_ticket,json=fileTicket,proto3" json:"file_ticket,omitempty"`
	Span       *common_go_proto.Span `protobuf:"bytes,2,opt,name=span,proto3" json:"span,omitempty"`
	Nodes      []*Link_Node          `protobuf:"bytes,3,rep,name=nodes,proto3" json:"nodes,omitempty"`
}

func (x *Link) Reset() {
	*x = Link{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_link_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Link) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Link) ProtoMessage() {}

func (x *Link) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_link_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Link.ProtoReflect.Descriptor instead.
func (*Link) Descriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{1}
}

func (x *Link) GetFileTicket() string {
	if x != nil {
		return x.FileTicket
	}
	return ""
}

func (x *Link) GetSpan() *common_go_proto.Span {
	if x != nil {
		return x.Span
	}
	return nil
}

func (x *Link) GetNodes() []*Link_Node {
	if x != nil {
		return x.Nodes
	}
	return nil
}

type LinkReply struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Links []*Link `protobuf:"bytes,1,rep,name=links,proto3" json:"links,omitempty"`
}

func (x *LinkReply) Reset() {
	*x = LinkReply{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_link_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *LinkReply) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*LinkReply) ProtoMessage() {}

func (x *LinkReply) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_link_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use LinkReply.ProtoReflect.Descriptor instead.
func (*LinkReply) Descriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{2}
}

func (x *LinkReply) GetLinks() []*Link {
	if x != nil {
		return x.Links
	}
	return nil
}

type LinkRequest_Location struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Path   string `protobuf:"bytes,1,opt,name=path,proto3" json:"path,omitempty"`
	Root   string `protobuf:"bytes,2,opt,name=root,proto3" json:"root,omitempty"`
	Corpus string `protobuf:"bytes,3,opt,name=corpus,proto3" json:"corpus,omitempty"`
}

func (x *LinkRequest_Location) Reset() {
	*x = LinkRequest_Location{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_link_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *LinkRequest_Location) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*LinkRequest_Location) ProtoMessage() {}

func (x *LinkRequest_Location) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_link_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use LinkRequest_Location.ProtoReflect.Descriptor instead.
func (*LinkRequest_Location) Descriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{0, 0}
}

func (x *LinkRequest_Location) GetPath() string {
	if x != nil {
		return x.Path
	}
	return ""
}

func (x *LinkRequest_Location) GetRoot() string {
	if x != nil {
		return x.Root
	}
	return ""
}

func (x *LinkRequest_Location) GetCorpus() string {
	if x != nil {
		return x.Corpus
	}
	return ""
}

type LinkRequest_Params struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Count int32 `protobuf:"varint,1,opt,name=count,proto3" json:"count,omitempty"`
}

func (x *LinkRequest_Params) Reset() {
	*x = LinkRequest_Params{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_link_proto_msgTypes[4]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *LinkRequest_Params) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*LinkRequest_Params) ProtoMessage() {}

func (x *LinkRequest_Params) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_link_proto_msgTypes[4]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use LinkRequest_Params.ProtoReflect.Descriptor instead.
func (*LinkRequest_Params) Descriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{0, 1}
}

func (x *LinkRequest_Params) GetCount() int32 {
	if x != nil {
		return x.Count
	}
	return 0
}

type Link_Node struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Ticket     string `protobuf:"bytes,1,opt,name=ticket,proto3" json:"ticket,omitempty"`
	BaseName   string `protobuf:"bytes,2,opt,name=base_name,json=baseName,proto3" json:"base_name,omitempty"`
	Identifier string `protobuf:"bytes,3,opt,name=identifier,proto3" json:"identifier,omitempty"`
}

func (x *Link_Node) Reset() {
	*x = Link_Node{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_link_proto_msgTypes[5]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Link_Node) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Link_Node) ProtoMessage() {}

func (x *Link_Node) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_link_proto_msgTypes[5]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Link_Node.ProtoReflect.Descriptor instead.
func (*Link_Node) Descriptor() ([]byte, []int) {
	return file_kythe_proto_link_proto_rawDescGZIP(), []int{1, 0}
}

func (x *Link_Node) GetTicket() string {
	if x != nil {
		return x.Ticket
	}
	return ""
}

func (x *Link_Node) GetBaseName() string {
	if x != nil {
		return x.BaseName
	}
	return ""
}

func (x *Link_Node) GetIdentifier() string {
	if x != nil {
		return x.Identifier
	}
	return ""
}

var File_kythe_proto_link_proto protoreflect.FileDescriptor

var file_kythe_proto_link_proto_rawDesc = []byte{
	0x0a, 0x16, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x6c, 0x69,
	0x6e, 0x6b, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0b, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x1a, 0x18, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x2f, 0x63, 0x6f, 0x6d, 0x6d, 0x6f, 0x6e, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22,
	0xc6, 0x04, 0x0a, 0x0b, 0x4c, 0x69, 0x6e, 0x6b, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12,
	0x1e, 0x0a, 0x0a, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x65, 0x72, 0x18, 0x01, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x0a, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x65, 0x72, 0x12,
	0x16, 0x0a, 0x06, 0x63, 0x6f, 0x72, 0x70, 0x75, 0x73, 0x18, 0x02, 0x20, 0x03, 0x28, 0x09, 0x52,
	0x06, 0x63, 0x6f, 0x72, 0x70, 0x75, 0x73, 0x12, 0x1a, 0x0a, 0x08, 0x6c, 0x61, 0x6e, 0x67, 0x75,
	0x61, 0x67, 0x65, 0x18, 0x03, 0x20, 0x03, 0x28, 0x09, 0x52, 0x08, 0x6c, 0x61, 0x6e, 0x67, 0x75,
	0x61, 0x67, 0x65, 0x12, 0x1b, 0x0a, 0x09, 0x6e, 0x6f, 0x64, 0x65, 0x5f, 0x6b, 0x69, 0x6e, 0x64,
	0x18, 0x04, 0x20, 0x03, 0x28, 0x09, 0x52, 0x08, 0x6e, 0x6f, 0x64, 0x65, 0x4b, 0x69, 0x6e, 0x64,
	0x12, 0x3b, 0x0a, 0x07, 0x69, 0x6e, 0x63, 0x6c, 0x75, 0x64, 0x65, 0x18, 0x05, 0x20, 0x03, 0x28,
	0x0b, 0x32, 0x21, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e,
	0x4c, 0x69, 0x6e, 0x6b, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x2e, 0x4c, 0x6f, 0x63, 0x61,
	0x74, 0x69, 0x6f, 0x6e, 0x52, 0x07, 0x69, 0x6e, 0x63, 0x6c, 0x75, 0x64, 0x65, 0x12, 0x3b, 0x0a,
	0x07, 0x65, 0x78, 0x63, 0x6c, 0x75, 0x64, 0x65, 0x18, 0x06, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x21,
	0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69, 0x6e,
	0x6b, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x2e, 0x4c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f,
	0x6e, 0x52, 0x07, 0x65, 0x78, 0x63, 0x6c, 0x75, 0x64, 0x65, 0x12, 0x37, 0x0a, 0x06, 0x70, 0x61,
	0x72, 0x61, 0x6d, 0x73, 0x18, 0x07, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x1f, 0x2e, 0x6b, 0x79, 0x74,
	0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69, 0x6e, 0x6b, 0x52, 0x65, 0x71,
	0x75, 0x65, 0x73, 0x74, 0x2e, 0x50, 0x61, 0x72, 0x61, 0x6d, 0x73, 0x52, 0x06, 0x70, 0x61, 0x72,
	0x61, 0x6d, 0x73, 0x12, 0x50, 0x0a, 0x0f, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f,
	0x6e, 0x5f, 0x6b, 0x69, 0x6e, 0x64, 0x18, 0x08, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x27, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69, 0x6e, 0x6b, 0x52,
	0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x2e, 0x44, 0x65, 0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f,
	0x6e, 0x4b, 0x69, 0x6e, 0x64, 0x52, 0x0e, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f,
	0x6e, 0x4b, 0x69, 0x6e, 0x64, 0x12, 0x23, 0x0a, 0x0d, 0x69, 0x6e, 0x63, 0x6c, 0x75, 0x64, 0x65,
	0x5f, 0x6e, 0x6f, 0x64, 0x65, 0x73, 0x18, 0x09, 0x20, 0x01, 0x28, 0x08, 0x52, 0x0c, 0x69, 0x6e,
	0x63, 0x6c, 0x75, 0x64, 0x65, 0x4e, 0x6f, 0x64, 0x65, 0x73, 0x1a, 0x4a, 0x0a, 0x08, 0x4c, 0x6f,
	0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x12, 0x12, 0x0a, 0x04, 0x70, 0x61, 0x74, 0x68, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x70, 0x61, 0x74, 0x68, 0x12, 0x12, 0x0a, 0x04, 0x72, 0x6f,
	0x6f, 0x74, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x72, 0x6f, 0x6f, 0x74, 0x12, 0x16,
	0x0a, 0x06, 0x63, 0x6f, 0x72, 0x70, 0x75, 0x73, 0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52, 0x06,
	0x63, 0x6f, 0x72, 0x70, 0x75, 0x73, 0x1a, 0x1e, 0x0a, 0x06, 0x50, 0x61, 0x72, 0x61, 0x6d, 0x73,
	0x12, 0x14, 0x0a, 0x05, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28, 0x05, 0x52,
	0x05, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x22, 0x30, 0x0a, 0x0e, 0x44, 0x65, 0x66, 0x69, 0x6e, 0x69,
	0x74, 0x69, 0x6f, 0x6e, 0x4b, 0x69, 0x6e, 0x64, 0x12, 0x0b, 0x0a, 0x07, 0x42, 0x49, 0x4e, 0x44,
	0x49, 0x4e, 0x47, 0x10, 0x00, 0x12, 0x08, 0x0a, 0x04, 0x46, 0x55, 0x4c, 0x4c, 0x10, 0x01, 0x12,
	0x07, 0x0a, 0x03, 0x41, 0x4e, 0x59, 0x10, 0x02, 0x22, 0xe0, 0x01, 0x0a, 0x04, 0x4c, 0x69, 0x6e,
	0x6b, 0x12, 0x1f, 0x0a, 0x0b, 0x66, 0x69, 0x6c, 0x65, 0x5f, 0x74, 0x69, 0x63, 0x6b, 0x65, 0x74,
	0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0a, 0x66, 0x69, 0x6c, 0x65, 0x54, 0x69, 0x63, 0x6b,
	0x65, 0x74, 0x12, 0x2c, 0x0a, 0x04, 0x73, 0x70, 0x61, 0x6e, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b,
	0x32, 0x18, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x63,
	0x6f, 0x6d, 0x6d, 0x6f, 0x6e, 0x2e, 0x53, 0x70, 0x61, 0x6e, 0x52, 0x04, 0x73, 0x70, 0x61, 0x6e,
	0x12, 0x2c, 0x0a, 0x05, 0x6e, 0x6f, 0x64, 0x65, 0x73, 0x18, 0x03, 0x20, 0x03, 0x28, 0x0b, 0x32,
	0x16, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69,
	0x6e, 0x6b, 0x2e, 0x4e, 0x6f, 0x64, 0x65, 0x52, 0x05, 0x6e, 0x6f, 0x64, 0x65, 0x73, 0x1a, 0x5b,
	0x0a, 0x04, 0x4e, 0x6f, 0x64, 0x65, 0x12, 0x16, 0x0a, 0x06, 0x74, 0x69, 0x63, 0x6b, 0x65, 0x74,
	0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x06, 0x74, 0x69, 0x63, 0x6b, 0x65, 0x74, 0x12, 0x1b,
	0x0a, 0x09, 0x62, 0x61, 0x73, 0x65, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28,
	0x09, 0x52, 0x08, 0x62, 0x61, 0x73, 0x65, 0x4e, 0x61, 0x6d, 0x65, 0x12, 0x1e, 0x0a, 0x0a, 0x69,
	0x64, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x65, 0x72, 0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52,
	0x0a, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x65, 0x72, 0x22, 0x34, 0x0a, 0x09, 0x4c,
	0x69, 0x6e, 0x6b, 0x52, 0x65, 0x70, 0x6c, 0x79, 0x12, 0x27, 0x0a, 0x05, 0x6c, 0x69, 0x6e, 0x6b,
	0x73, 0x18, 0x01, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x11, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69, 0x6e, 0x6b, 0x52, 0x05, 0x6c, 0x69, 0x6e, 0x6b,
	0x73, 0x32, 0x4c, 0x0a, 0x0b, 0x4c, 0x69, 0x6e, 0x6b, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65,
	0x12, 0x3d, 0x0a, 0x07, 0x52, 0x65, 0x73, 0x6f, 0x6c, 0x76, 0x65, 0x12, 0x18, 0x2e, 0x6b, 0x79,
	0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69, 0x6e, 0x6b, 0x52, 0x65,
	0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x16, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x2e, 0x4c, 0x69, 0x6e, 0x6b, 0x52, 0x65, 0x70, 0x6c, 0x79, 0x22, 0x00, 0x42,
	0x45, 0x0a, 0x1f, 0x63, 0x6f, 0x6d, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65,
	0x76, 0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x5a, 0x22, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x69, 0x6f, 0x2f, 0x6b, 0x79, 0x74,
	0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x6c, 0x69, 0x6e, 0x6b, 0x5f, 0x67, 0x6f,
	0x5f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_kythe_proto_link_proto_rawDescOnce sync.Once
	file_kythe_proto_link_proto_rawDescData = file_kythe_proto_link_proto_rawDesc
)

func file_kythe_proto_link_proto_rawDescGZIP() []byte {
	file_kythe_proto_link_proto_rawDescOnce.Do(func() {
		file_kythe_proto_link_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_link_proto_rawDescData)
	})
	return file_kythe_proto_link_proto_rawDescData
}

var file_kythe_proto_link_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_kythe_proto_link_proto_msgTypes = make([]protoimpl.MessageInfo, 6)
var file_kythe_proto_link_proto_goTypes = []interface{}{
	(LinkRequest_DefinitionKind)(0), // 0: kythe.proto.LinkRequest.DefinitionKind
	(*LinkRequest)(nil),             // 1: kythe.proto.LinkRequest
	(*Link)(nil),                    // 2: kythe.proto.Link
	(*LinkReply)(nil),               // 3: kythe.proto.LinkReply
	(*LinkRequest_Location)(nil),    // 4: kythe.proto.LinkRequest.Location
	(*LinkRequest_Params)(nil),      // 5: kythe.proto.LinkRequest.Params
	(*Link_Node)(nil),               // 6: kythe.proto.Link.Node
	(*common_go_proto.Span)(nil),    // 7: kythe.proto.common.Span
}
var file_kythe_proto_link_proto_depIdxs = []int32{
	4, // 0: kythe.proto.LinkRequest.include:type_name -> kythe.proto.LinkRequest.Location
	4, // 1: kythe.proto.LinkRequest.exclude:type_name -> kythe.proto.LinkRequest.Location
	5, // 2: kythe.proto.LinkRequest.params:type_name -> kythe.proto.LinkRequest.Params
	0, // 3: kythe.proto.LinkRequest.definition_kind:type_name -> kythe.proto.LinkRequest.DefinitionKind
	7, // 4: kythe.proto.Link.span:type_name -> kythe.proto.common.Span
	6, // 5: kythe.proto.Link.nodes:type_name -> kythe.proto.Link.Node
	2, // 6: kythe.proto.LinkReply.links:type_name -> kythe.proto.Link
	1, // 7: kythe.proto.LinkService.Resolve:input_type -> kythe.proto.LinkRequest
	3, // 8: kythe.proto.LinkService.Resolve:output_type -> kythe.proto.LinkReply
	8, // [8:9] is the sub-list for method output_type
	7, // [7:8] is the sub-list for method input_type
	7, // [7:7] is the sub-list for extension type_name
	7, // [7:7] is the sub-list for extension extendee
	0, // [0:7] is the sub-list for field type_name
}

func init() { file_kythe_proto_link_proto_init() }
func file_kythe_proto_link_proto_init() {
	if File_kythe_proto_link_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_link_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*LinkRequest); i {
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
		file_kythe_proto_link_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Link); i {
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
		file_kythe_proto_link_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*LinkReply); i {
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
		file_kythe_proto_link_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*LinkRequest_Location); i {
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
		file_kythe_proto_link_proto_msgTypes[4].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*LinkRequest_Params); i {
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
		file_kythe_proto_link_proto_msgTypes[5].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Link_Node); i {
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
			RawDescriptor: file_kythe_proto_link_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   6,
			NumExtensions: 0,
			NumServices:   1,
		},
		GoTypes:           file_kythe_proto_link_proto_goTypes,
		DependencyIndexes: file_kythe_proto_link_proto_depIdxs,
		EnumInfos:         file_kythe_proto_link_proto_enumTypes,
		MessageInfos:      file_kythe_proto_link_proto_msgTypes,
	}.Build()
	File_kythe_proto_link_proto = out.File
	file_kythe_proto_link_proto_rawDesc = nil
	file_kythe_proto_link_proto_goTypes = nil
	file_kythe_proto_link_proto_depIdxs = nil
}
