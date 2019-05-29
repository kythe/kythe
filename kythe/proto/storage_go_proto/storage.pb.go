// Code generated by protoc-gen-go. DO NOT EDIT.
// source: kythe/proto/storage.proto

package storage_go_proto

import (
	fmt "fmt"
	proto "github.com/golang/protobuf/proto"
	math "math"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion3 // please upgrade the proto package

type VName struct {
	Signature            string   `protobuf:"bytes,1,opt,name=signature,proto3" json:"signature,omitempty"`
	Corpus               string   `protobuf:"bytes,2,opt,name=corpus,proto3" json:"corpus,omitempty"`
	Root                 string   `protobuf:"bytes,3,opt,name=root,proto3" json:"root,omitempty"`
	Path                 string   `protobuf:"bytes,4,opt,name=path,proto3" json:"path,omitempty"`
	Language             string   `protobuf:"bytes,5,opt,name=language,proto3" json:"language,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *VName) Reset()         { *m = VName{} }
func (m *VName) String() string { return proto.CompactTextString(m) }
func (*VName) ProtoMessage()    {}
func (*VName) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{0}
}

func (m *VName) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_VName.Unmarshal(m, b)
}
func (m *VName) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_VName.Marshal(b, m, deterministic)
}
func (m *VName) XXX_Merge(src proto.Message) {
	xxx_messageInfo_VName.Merge(m, src)
}
func (m *VName) XXX_Size() int {
	return xxx_messageInfo_VName.Size(m)
}
func (m *VName) XXX_DiscardUnknown() {
	xxx_messageInfo_VName.DiscardUnknown(m)
}

var xxx_messageInfo_VName proto.InternalMessageInfo

func (m *VName) GetSignature() string {
	if m != nil {
		return m.Signature
	}
	return ""
}

func (m *VName) GetCorpus() string {
	if m != nil {
		return m.Corpus
	}
	return ""
}

func (m *VName) GetRoot() string {
	if m != nil {
		return m.Root
	}
	return ""
}

func (m *VName) GetPath() string {
	if m != nil {
		return m.Path
	}
	return ""
}

func (m *VName) GetLanguage() string {
	if m != nil {
		return m.Language
	}
	return ""
}

type VNameMask struct {
	Signature            bool     `protobuf:"varint,1,opt,name=signature,proto3" json:"signature,omitempty"`
	Corpus               bool     `protobuf:"varint,2,opt,name=corpus,proto3" json:"corpus,omitempty"`
	Root                 bool     `protobuf:"varint,3,opt,name=root,proto3" json:"root,omitempty"`
	Path                 bool     `protobuf:"varint,4,opt,name=path,proto3" json:"path,omitempty"`
	Language             bool     `protobuf:"varint,5,opt,name=language,proto3" json:"language,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *VNameMask) Reset()         { *m = VNameMask{} }
func (m *VNameMask) String() string { return proto.CompactTextString(m) }
func (*VNameMask) ProtoMessage()    {}
func (*VNameMask) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{1}
}

func (m *VNameMask) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_VNameMask.Unmarshal(m, b)
}
func (m *VNameMask) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_VNameMask.Marshal(b, m, deterministic)
}
func (m *VNameMask) XXX_Merge(src proto.Message) {
	xxx_messageInfo_VNameMask.Merge(m, src)
}
func (m *VNameMask) XXX_Size() int {
	return xxx_messageInfo_VNameMask.Size(m)
}
func (m *VNameMask) XXX_DiscardUnknown() {
	xxx_messageInfo_VNameMask.DiscardUnknown(m)
}

var xxx_messageInfo_VNameMask proto.InternalMessageInfo

func (m *VNameMask) GetSignature() bool {
	if m != nil {
		return m.Signature
	}
	return false
}

func (m *VNameMask) GetCorpus() bool {
	if m != nil {
		return m.Corpus
	}
	return false
}

func (m *VNameMask) GetRoot() bool {
	if m != nil {
		return m.Root
	}
	return false
}

func (m *VNameMask) GetPath() bool {
	if m != nil {
		return m.Path
	}
	return false
}

func (m *VNameMask) GetLanguage() bool {
	if m != nil {
		return m.Language
	}
	return false
}

type Entry struct {
	Source               *VName   `protobuf:"bytes,1,opt,name=source,proto3" json:"source,omitempty"`
	EdgeKind             string   `protobuf:"bytes,2,opt,name=edge_kind,json=edgeKind,proto3" json:"edge_kind,omitempty"`
	Target               *VName   `protobuf:"bytes,3,opt,name=target,proto3" json:"target,omitempty"`
	FactName             string   `protobuf:"bytes,4,opt,name=fact_name,json=factName,proto3" json:"fact_name,omitempty"`
	FactValue            []byte   `protobuf:"bytes,5,opt,name=fact_value,json=factValue,proto3" json:"fact_value,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *Entry) Reset()         { *m = Entry{} }
func (m *Entry) String() string { return proto.CompactTextString(m) }
func (*Entry) ProtoMessage()    {}
func (*Entry) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{2}
}

func (m *Entry) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Entry.Unmarshal(m, b)
}
func (m *Entry) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Entry.Marshal(b, m, deterministic)
}
func (m *Entry) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Entry.Merge(m, src)
}
func (m *Entry) XXX_Size() int {
	return xxx_messageInfo_Entry.Size(m)
}
func (m *Entry) XXX_DiscardUnknown() {
	xxx_messageInfo_Entry.DiscardUnknown(m)
}

var xxx_messageInfo_Entry proto.InternalMessageInfo

func (m *Entry) GetSource() *VName {
	if m != nil {
		return m.Source
	}
	return nil
}

func (m *Entry) GetEdgeKind() string {
	if m != nil {
		return m.EdgeKind
	}
	return ""
}

func (m *Entry) GetTarget() *VName {
	if m != nil {
		return m.Target
	}
	return nil
}

func (m *Entry) GetFactName() string {
	if m != nil {
		return m.FactName
	}
	return ""
}

func (m *Entry) GetFactValue() []byte {
	if m != nil {
		return m.FactValue
	}
	return nil
}

type Entries struct {
	Entries              []*Entry `protobuf:"bytes,1,rep,name=entries,proto3" json:"entries,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *Entries) Reset()         { *m = Entries{} }
func (m *Entries) String() string { return proto.CompactTextString(m) }
func (*Entries) ProtoMessage()    {}
func (*Entries) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{3}
}

func (m *Entries) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Entries.Unmarshal(m, b)
}
func (m *Entries) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Entries.Marshal(b, m, deterministic)
}
func (m *Entries) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Entries.Merge(m, src)
}
func (m *Entries) XXX_Size() int {
	return xxx_messageInfo_Entries.Size(m)
}
func (m *Entries) XXX_DiscardUnknown() {
	xxx_messageInfo_Entries.DiscardUnknown(m)
}

var xxx_messageInfo_Entries proto.InternalMessageInfo

func (m *Entries) GetEntries() []*Entry {
	if m != nil {
		return m.Entries
	}
	return nil
}

type ReadRequest struct {
	Source               *VName   `protobuf:"bytes,1,opt,name=source,proto3" json:"source,omitempty"`
	EdgeKind             string   `protobuf:"bytes,2,opt,name=edge_kind,json=edgeKind,proto3" json:"edge_kind,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ReadRequest) Reset()         { *m = ReadRequest{} }
func (m *ReadRequest) String() string { return proto.CompactTextString(m) }
func (*ReadRequest) ProtoMessage()    {}
func (*ReadRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{4}
}

func (m *ReadRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ReadRequest.Unmarshal(m, b)
}
func (m *ReadRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ReadRequest.Marshal(b, m, deterministic)
}
func (m *ReadRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ReadRequest.Merge(m, src)
}
func (m *ReadRequest) XXX_Size() int {
	return xxx_messageInfo_ReadRequest.Size(m)
}
func (m *ReadRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_ReadRequest.DiscardUnknown(m)
}

var xxx_messageInfo_ReadRequest proto.InternalMessageInfo

func (m *ReadRequest) GetSource() *VName {
	if m != nil {
		return m.Source
	}
	return nil
}

func (m *ReadRequest) GetEdgeKind() string {
	if m != nil {
		return m.EdgeKind
	}
	return ""
}

type WriteRequest struct {
	Source               *VName                 `protobuf:"bytes,1,opt,name=source,proto3" json:"source,omitempty"`
	Update               []*WriteRequest_Update `protobuf:"bytes,2,rep,name=update,proto3" json:"update,omitempty"`
	XXX_NoUnkeyedLiteral struct{}               `json:"-"`
	XXX_unrecognized     []byte                 `json:"-"`
	XXX_sizecache        int32                  `json:"-"`
}

func (m *WriteRequest) Reset()         { *m = WriteRequest{} }
func (m *WriteRequest) String() string { return proto.CompactTextString(m) }
func (*WriteRequest) ProtoMessage()    {}
func (*WriteRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{5}
}

func (m *WriteRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_WriteRequest.Unmarshal(m, b)
}
func (m *WriteRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_WriteRequest.Marshal(b, m, deterministic)
}
func (m *WriteRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_WriteRequest.Merge(m, src)
}
func (m *WriteRequest) XXX_Size() int {
	return xxx_messageInfo_WriteRequest.Size(m)
}
func (m *WriteRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_WriteRequest.DiscardUnknown(m)
}

var xxx_messageInfo_WriteRequest proto.InternalMessageInfo

func (m *WriteRequest) GetSource() *VName {
	if m != nil {
		return m.Source
	}
	return nil
}

func (m *WriteRequest) GetUpdate() []*WriteRequest_Update {
	if m != nil {
		return m.Update
	}
	return nil
}

type WriteRequest_Update struct {
	EdgeKind             string   `protobuf:"bytes,1,opt,name=edge_kind,json=edgeKind,proto3" json:"edge_kind,omitempty"`
	Target               *VName   `protobuf:"bytes,2,opt,name=target,proto3" json:"target,omitempty"`
	FactName             string   `protobuf:"bytes,3,opt,name=fact_name,json=factName,proto3" json:"fact_name,omitempty"`
	FactValue            []byte   `protobuf:"bytes,4,opt,name=fact_value,json=factValue,proto3" json:"fact_value,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *WriteRequest_Update) Reset()         { *m = WriteRequest_Update{} }
func (m *WriteRequest_Update) String() string { return proto.CompactTextString(m) }
func (*WriteRequest_Update) ProtoMessage()    {}
func (*WriteRequest_Update) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{5, 0}
}

func (m *WriteRequest_Update) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_WriteRequest_Update.Unmarshal(m, b)
}
func (m *WriteRequest_Update) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_WriteRequest_Update.Marshal(b, m, deterministic)
}
func (m *WriteRequest_Update) XXX_Merge(src proto.Message) {
	xxx_messageInfo_WriteRequest_Update.Merge(m, src)
}
func (m *WriteRequest_Update) XXX_Size() int {
	return xxx_messageInfo_WriteRequest_Update.Size(m)
}
func (m *WriteRequest_Update) XXX_DiscardUnknown() {
	xxx_messageInfo_WriteRequest_Update.DiscardUnknown(m)
}

var xxx_messageInfo_WriteRequest_Update proto.InternalMessageInfo

func (m *WriteRequest_Update) GetEdgeKind() string {
	if m != nil {
		return m.EdgeKind
	}
	return ""
}

func (m *WriteRequest_Update) GetTarget() *VName {
	if m != nil {
		return m.Target
	}
	return nil
}

func (m *WriteRequest_Update) GetFactName() string {
	if m != nil {
		return m.FactName
	}
	return ""
}

func (m *WriteRequest_Update) GetFactValue() []byte {
	if m != nil {
		return m.FactValue
	}
	return nil
}

type WriteReply struct {
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *WriteReply) Reset()         { *m = WriteReply{} }
func (m *WriteReply) String() string { return proto.CompactTextString(m) }
func (*WriteReply) ProtoMessage()    {}
func (*WriteReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{6}
}

func (m *WriteReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_WriteReply.Unmarshal(m, b)
}
func (m *WriteReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_WriteReply.Marshal(b, m, deterministic)
}
func (m *WriteReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_WriteReply.Merge(m, src)
}
func (m *WriteReply) XXX_Size() int {
	return xxx_messageInfo_WriteReply.Size(m)
}
func (m *WriteReply) XXX_DiscardUnknown() {
	xxx_messageInfo_WriteReply.DiscardUnknown(m)
}

var xxx_messageInfo_WriteReply proto.InternalMessageInfo

type ScanRequest struct {
	Target               *VName   `protobuf:"bytes,1,opt,name=target,proto3" json:"target,omitempty"`
	EdgeKind             string   `protobuf:"bytes,2,opt,name=edge_kind,json=edgeKind,proto3" json:"edge_kind,omitempty"`
	FactPrefix           string   `protobuf:"bytes,3,opt,name=fact_prefix,json=factPrefix,proto3" json:"fact_prefix,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ScanRequest) Reset()         { *m = ScanRequest{} }
func (m *ScanRequest) String() string { return proto.CompactTextString(m) }
func (*ScanRequest) ProtoMessage()    {}
func (*ScanRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{7}
}

func (m *ScanRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ScanRequest.Unmarshal(m, b)
}
func (m *ScanRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ScanRequest.Marshal(b, m, deterministic)
}
func (m *ScanRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ScanRequest.Merge(m, src)
}
func (m *ScanRequest) XXX_Size() int {
	return xxx_messageInfo_ScanRequest.Size(m)
}
func (m *ScanRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_ScanRequest.DiscardUnknown(m)
}

var xxx_messageInfo_ScanRequest proto.InternalMessageInfo

func (m *ScanRequest) GetTarget() *VName {
	if m != nil {
		return m.Target
	}
	return nil
}

func (m *ScanRequest) GetEdgeKind() string {
	if m != nil {
		return m.EdgeKind
	}
	return ""
}

func (m *ScanRequest) GetFactPrefix() string {
	if m != nil {
		return m.FactPrefix
	}
	return ""
}

type CountRequest struct {
	Index                int64    `protobuf:"varint,1,opt,name=index,proto3" json:"index,omitempty"`
	Shards               int64    `protobuf:"varint,2,opt,name=shards,proto3" json:"shards,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CountRequest) Reset()         { *m = CountRequest{} }
func (m *CountRequest) String() string { return proto.CompactTextString(m) }
func (*CountRequest) ProtoMessage()    {}
func (*CountRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{8}
}

func (m *CountRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CountRequest.Unmarshal(m, b)
}
func (m *CountRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CountRequest.Marshal(b, m, deterministic)
}
func (m *CountRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CountRequest.Merge(m, src)
}
func (m *CountRequest) XXX_Size() int {
	return xxx_messageInfo_CountRequest.Size(m)
}
func (m *CountRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_CountRequest.DiscardUnknown(m)
}

var xxx_messageInfo_CountRequest proto.InternalMessageInfo

func (m *CountRequest) GetIndex() int64 {
	if m != nil {
		return m.Index
	}
	return 0
}

func (m *CountRequest) GetShards() int64 {
	if m != nil {
		return m.Shards
	}
	return 0
}

type CountReply struct {
	Entries              int64    `protobuf:"varint,1,opt,name=entries,proto3" json:"entries,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CountReply) Reset()         { *m = CountReply{} }
func (m *CountReply) String() string { return proto.CompactTextString(m) }
func (*CountReply) ProtoMessage()    {}
func (*CountReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{9}
}

func (m *CountReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CountReply.Unmarshal(m, b)
}
func (m *CountReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CountReply.Marshal(b, m, deterministic)
}
func (m *CountReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CountReply.Merge(m, src)
}
func (m *CountReply) XXX_Size() int {
	return xxx_messageInfo_CountReply.Size(m)
}
func (m *CountReply) XXX_DiscardUnknown() {
	xxx_messageInfo_CountReply.DiscardUnknown(m)
}

var xxx_messageInfo_CountReply proto.InternalMessageInfo

func (m *CountReply) GetEntries() int64 {
	if m != nil {
		return m.Entries
	}
	return 0
}

type ShardRequest struct {
	Index                int64    `protobuf:"varint,1,opt,name=index,proto3" json:"index,omitempty"`
	Shards               int64    `protobuf:"varint,2,opt,name=shards,proto3" json:"shards,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ShardRequest) Reset()         { *m = ShardRequest{} }
func (m *ShardRequest) String() string { return proto.CompactTextString(m) }
func (*ShardRequest) ProtoMessage()    {}
func (*ShardRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_b1438903ecc9440e, []int{10}
}

func (m *ShardRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ShardRequest.Unmarshal(m, b)
}
func (m *ShardRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ShardRequest.Marshal(b, m, deterministic)
}
func (m *ShardRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ShardRequest.Merge(m, src)
}
func (m *ShardRequest) XXX_Size() int {
	return xxx_messageInfo_ShardRequest.Size(m)
}
func (m *ShardRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_ShardRequest.DiscardUnknown(m)
}

var xxx_messageInfo_ShardRequest proto.InternalMessageInfo

func (m *ShardRequest) GetIndex() int64 {
	if m != nil {
		return m.Index
	}
	return 0
}

func (m *ShardRequest) GetShards() int64 {
	if m != nil {
		return m.Shards
	}
	return 0
}

func init() {
	proto.RegisterType((*VName)(nil), "kythe.proto.VName")
	proto.RegisterType((*VNameMask)(nil), "kythe.proto.VNameMask")
	proto.RegisterType((*Entry)(nil), "kythe.proto.Entry")
	proto.RegisterType((*Entries)(nil), "kythe.proto.Entries")
	proto.RegisterType((*ReadRequest)(nil), "kythe.proto.ReadRequest")
	proto.RegisterType((*WriteRequest)(nil), "kythe.proto.WriteRequest")
	proto.RegisterType((*WriteRequest_Update)(nil), "kythe.proto.WriteRequest.Update")
	proto.RegisterType((*WriteReply)(nil), "kythe.proto.WriteReply")
	proto.RegisterType((*ScanRequest)(nil), "kythe.proto.ScanRequest")
	proto.RegisterType((*CountRequest)(nil), "kythe.proto.CountRequest")
	proto.RegisterType((*CountReply)(nil), "kythe.proto.CountReply")
	proto.RegisterType((*ShardRequest)(nil), "kythe.proto.ShardRequest")
}

func init() { proto.RegisterFile("kythe/proto/storage.proto", fileDescriptor_b1438903ecc9440e) }

var fileDescriptor_b1438903ecc9440e = []byte{
	// 499 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xac, 0x94, 0xcf, 0x6e, 0xd4, 0x30,
	0x10, 0xc6, 0x95, 0xfd, 0x93, 0x66, 0x27, 0x39, 0x20, 0x0b, 0xa1, 0x50, 0x40, 0x5d, 0xe5, 0x80,
	0x10, 0x42, 0xa9, 0x44, 0x0f, 0x70, 0xe8, 0x09, 0xc4, 0x09, 0x81, 0x50, 0x2a, 0x16, 0x89, 0xcb,
	0xca, 0x24, 0xd3, 0x6c, 0xb4, 0xd9, 0x38, 0xd8, 0x4e, 0xe9, 0x1e, 0x91, 0xe0, 0xc8, 0xfb, 0xf0,
	0x78, 0xc8, 0x63, 0xb7, 0xa4, 0xb4, 0x41, 0x50, 0xf5, 0xb4, 0x9e, 0xcf, 0x5f, 0x66, 0x7e, 0x9e,
	0xb1, 0x17, 0xee, 0xae, 0xb7, 0x7a, 0x85, 0xfb, 0xad, 0x14, 0x5a, 0xec, 0x2b, 0x2d, 0x24, 0x2f,
	0x31, 0xa5, 0x88, 0x85, 0xb4, 0x65, 0x83, 0xe4, 0xab, 0x07, 0xd3, 0xc5, 0x5b, 0xbe, 0x41, 0x76,
	0x1f, 0x66, 0xaa, 0x2a, 0x1b, 0xae, 0x3b, 0x89, 0xb1, 0x37, 0xf7, 0x1e, 0xcd, 0xb2, 0xdf, 0x02,
	0xbb, 0x03, 0x7e, 0x2e, 0x64, 0xdb, 0xa9, 0x78, 0x44, 0x5b, 0x2e, 0x62, 0x0c, 0x26, 0x52, 0x08,
	0x1d, 0x8f, 0x49, 0xa5, 0xb5, 0xd1, 0x5a, 0xae, 0x57, 0xf1, 0xc4, 0x6a, 0x66, 0xcd, 0x76, 0x21,
	0xa8, 0x79, 0x53, 0x76, 0xbc, 0xc4, 0x78, 0x4a, 0xfa, 0x79, 0x9c, 0x7c, 0xf7, 0x60, 0x46, 0x0c,
	0x6f, 0xb8, 0x5a, 0x5f, 0xe6, 0x08, 0x86, 0x39, 0x82, 0x2b, 0x39, 0x82, 0x2b, 0x38, 0x82, 0x01,
	0x8e, 0xa0, 0xc7, 0xf1, 0xd3, 0x83, 0xe9, 0xab, 0x46, 0xcb, 0x2d, 0x7b, 0x0c, 0xbe, 0x12, 0x9d,
	0xcc, 0x2d, 0x40, 0xf8, 0x94, 0xa5, 0xbd, 0x9e, 0xa5, 0xc4, 0x9a, 0x39, 0x07, 0xbb, 0x07, 0x33,
	0x2c, 0x4a, 0x5c, 0xae, 0xab, 0xa6, 0x70, 0xcd, 0x09, 0x8c, 0xf0, 0xba, 0x6a, 0x0a, 0x93, 0x48,
	0x73, 0x59, 0xa2, 0x05, 0x1b, 0x48, 0x64, 0x1d, 0x26, 0xd1, 0x31, 0xcf, 0xf5, 0xb2, 0xe1, 0x1b,
	0x74, 0xbd, 0x0b, 0x8c, 0x40, 0xd3, 0x79, 0x00, 0x40, 0x9b, 0x27, 0xbc, 0xee, 0x2c, 0x79, 0x94,
	0x91, 0x7d, 0x61, 0x84, 0xe4, 0x19, 0xec, 0x18, 0xf2, 0x0a, 0x15, 0x7b, 0x02, 0x3b, 0x68, 0x97,
	0xb1, 0x37, 0x1f, 0x5f, 0xaa, 0x49, 0x07, 0xcc, 0xce, 0x2c, 0xc9, 0x02, 0xc2, 0x0c, 0x79, 0x91,
	0xe1, 0xe7, 0x0e, 0x95, 0xbe, 0xb1, 0x83, 0x27, 0xdf, 0x46, 0x10, 0x7d, 0x90, 0x95, 0xc6, 0xeb,
	0x64, 0x7e, 0x0e, 0x7e, 0xd7, 0x16, 0x5c, 0x63, 0x3c, 0xa2, 0x13, 0xcc, 0x2f, 0x78, 0xfb, 0x69,
	0xd3, 0xf7, 0xe4, 0xcb, 0x9c, 0x7f, 0xf7, 0x87, 0x07, 0xbe, 0x95, 0x2e, 0xe2, 0x79, 0x83, 0x73,
	0x19, 0xfd, 0xdf, 0x5c, 0xc6, 0x7f, 0x9d, 0xcb, 0xe4, 0xcf, 0xb9, 0x44, 0x00, 0x0e, 0xb7, 0xad,
	0xb7, 0xc9, 0x17, 0x08, 0x8f, 0x72, 0xde, 0xf4, 0x5a, 0xe2, 0x20, 0xbc, 0x7f, 0x81, 0x18, 0xbe,
	0x65, 0x7b, 0x10, 0x12, 0x44, 0x2b, 0xf1, 0xb8, 0x3a, 0x75, 0x8c, 0xc4, 0xf5, 0x8e, 0x94, 0xe4,
	0x10, 0xa2, 0x97, 0xa2, 0x6b, 0xf4, 0x59, 0xe5, 0xdb, 0x30, 0xad, 0x9a, 0x02, 0x4f, 0xa9, 0xf0,
	0x38, 0xb3, 0x81, 0x79, 0x5b, 0x6a, 0xc5, 0x65, 0x61, 0xdf, 0xd6, 0x38, 0x73, 0x51, 0xf2, 0x10,
	0xc0, 0x7d, 0xdd, 0xd6, 0x5b, 0x16, 0xf7, 0xef, 0x97, 0xb1, 0x9d, 0xdf, 0xa5, 0x43, 0x88, 0x8e,
	0xcc, 0x17, 0xd7, 0xaa, 0xf2, 0xe2, 0x00, 0xf6, 0x72, 0xb1, 0x49, 0x4b, 0x21, 0xca, 0x1a, 0xd3,
	0x02, 0x4f, 0xb4, 0x10, 0xb5, 0xea, 0xb7, 0xe4, 0xe3, 0x2d, 0xf7, 0x47, 0xb6, 0x2c, 0xc5, 0x92,
	0x94, 0x4f, 0x3e, 0xfd, 0x1c, 0xfc, 0x0a, 0x00, 0x00, 0xff, 0xff, 0xa9, 0x5d, 0x20, 0xcd, 0xef,
	0x04, 0x00, 0x00,
}
