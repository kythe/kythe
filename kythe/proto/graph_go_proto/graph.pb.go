// Code generated by protoc-gen-go. DO NOT EDIT.
// source: kythe/proto/graph.proto

package graph_go_proto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import common_go_proto "kythe.io/kythe/proto/common_go_proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type NodesRequest struct {
	Ticket               []string `protobuf:"bytes,1,rep,name=ticket,proto3" json:"ticket,omitempty"`
	Filter               []string `protobuf:"bytes,2,rep,name=filter,proto3" json:"filter,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *NodesRequest) Reset()         { *m = NodesRequest{} }
func (m *NodesRequest) String() string { return proto.CompactTextString(m) }
func (*NodesRequest) ProtoMessage()    {}
func (*NodesRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{0}
}
func (m *NodesRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_NodesRequest.Unmarshal(m, b)
}
func (m *NodesRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_NodesRequest.Marshal(b, m, deterministic)
}
func (dst *NodesRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_NodesRequest.Merge(dst, src)
}
func (m *NodesRequest) XXX_Size() int {
	return xxx_messageInfo_NodesRequest.Size(m)
}
func (m *NodesRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_NodesRequest.DiscardUnknown(m)
}

var xxx_messageInfo_NodesRequest proto.InternalMessageInfo

func (m *NodesRequest) GetTicket() []string {
	if m != nil {
		return m.Ticket
	}
	return nil
}

func (m *NodesRequest) GetFilter() []string {
	if m != nil {
		return m.Filter
	}
	return nil
}

type NodesReply struct {
	Nodes                map[string]*common_go_proto.NodeInfo `protobuf:"bytes,1,rep,name=nodes,proto3" json:"nodes,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral struct{}                             `json:"-"`
	XXX_unrecognized     []byte                               `json:"-"`
	XXX_sizecache        int32                                `json:"-"`
}

func (m *NodesReply) Reset()         { *m = NodesReply{} }
func (m *NodesReply) String() string { return proto.CompactTextString(m) }
func (*NodesReply) ProtoMessage()    {}
func (*NodesReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{1}
}
func (m *NodesReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_NodesReply.Unmarshal(m, b)
}
func (m *NodesReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_NodesReply.Marshal(b, m, deterministic)
}
func (dst *NodesReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_NodesReply.Merge(dst, src)
}
func (m *NodesReply) XXX_Size() int {
	return xxx_messageInfo_NodesReply.Size(m)
}
func (m *NodesReply) XXX_DiscardUnknown() {
	xxx_messageInfo_NodesReply.DiscardUnknown(m)
}

var xxx_messageInfo_NodesReply proto.InternalMessageInfo

func (m *NodesReply) GetNodes() map[string]*common_go_proto.NodeInfo {
	if m != nil {
		return m.Nodes
	}
	return nil
}

type EdgesRequest struct {
	Ticket               []string `protobuf:"bytes,1,rep,name=ticket,proto3" json:"ticket,omitempty"`
	Kind                 []string `protobuf:"bytes,2,rep,name=kind,proto3" json:"kind,omitempty"`
	Filter               []string `protobuf:"bytes,3,rep,name=filter,proto3" json:"filter,omitempty"`
	PageSize             int32    `protobuf:"varint,8,opt,name=page_size,json=pageSize,proto3" json:"page_size,omitempty"`
	PageToken            string   `protobuf:"bytes,9,opt,name=page_token,json=pageToken,proto3" json:"page_token,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *EdgesRequest) Reset()         { *m = EdgesRequest{} }
func (m *EdgesRequest) String() string { return proto.CompactTextString(m) }
func (*EdgesRequest) ProtoMessage()    {}
func (*EdgesRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{2}
}
func (m *EdgesRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_EdgesRequest.Unmarshal(m, b)
}
func (m *EdgesRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_EdgesRequest.Marshal(b, m, deterministic)
}
func (dst *EdgesRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_EdgesRequest.Merge(dst, src)
}
func (m *EdgesRequest) XXX_Size() int {
	return xxx_messageInfo_EdgesRequest.Size(m)
}
func (m *EdgesRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_EdgesRequest.DiscardUnknown(m)
}

var xxx_messageInfo_EdgesRequest proto.InternalMessageInfo

func (m *EdgesRequest) GetTicket() []string {
	if m != nil {
		return m.Ticket
	}
	return nil
}

func (m *EdgesRequest) GetKind() []string {
	if m != nil {
		return m.Kind
	}
	return nil
}

func (m *EdgesRequest) GetFilter() []string {
	if m != nil {
		return m.Filter
	}
	return nil
}

func (m *EdgesRequest) GetPageSize() int32 {
	if m != nil {
		return m.PageSize
	}
	return 0
}

func (m *EdgesRequest) GetPageToken() string {
	if m != nil {
		return m.PageToken
	}
	return ""
}

type EdgeSet struct {
	Groups               map[string]*EdgeSet_Group `protobuf:"bytes,2,rep,name=groups,proto3" json:"groups,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral struct{}                  `json:"-"`
	XXX_unrecognized     []byte                    `json:"-"`
	XXX_sizecache        int32                     `json:"-"`
}

func (m *EdgeSet) Reset()         { *m = EdgeSet{} }
func (m *EdgeSet) String() string { return proto.CompactTextString(m) }
func (*EdgeSet) ProtoMessage()    {}
func (*EdgeSet) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{3}
}
func (m *EdgeSet) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_EdgeSet.Unmarshal(m, b)
}
func (m *EdgeSet) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_EdgeSet.Marshal(b, m, deterministic)
}
func (dst *EdgeSet) XXX_Merge(src proto.Message) {
	xxx_messageInfo_EdgeSet.Merge(dst, src)
}
func (m *EdgeSet) XXX_Size() int {
	return xxx_messageInfo_EdgeSet.Size(m)
}
func (m *EdgeSet) XXX_DiscardUnknown() {
	xxx_messageInfo_EdgeSet.DiscardUnknown(m)
}

var xxx_messageInfo_EdgeSet proto.InternalMessageInfo

func (m *EdgeSet) GetGroups() map[string]*EdgeSet_Group {
	if m != nil {
		return m.Groups
	}
	return nil
}

type EdgeSet_Group struct {
	Edge                 []*EdgeSet_Group_Edge `protobuf:"bytes,2,rep,name=edge,proto3" json:"edge,omitempty"`
	XXX_NoUnkeyedLiteral struct{}              `json:"-"`
	XXX_unrecognized     []byte                `json:"-"`
	XXX_sizecache        int32                 `json:"-"`
}

func (m *EdgeSet_Group) Reset()         { *m = EdgeSet_Group{} }
func (m *EdgeSet_Group) String() string { return proto.CompactTextString(m) }
func (*EdgeSet_Group) ProtoMessage()    {}
func (*EdgeSet_Group) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{3, 0}
}
func (m *EdgeSet_Group) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_EdgeSet_Group.Unmarshal(m, b)
}
func (m *EdgeSet_Group) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_EdgeSet_Group.Marshal(b, m, deterministic)
}
func (dst *EdgeSet_Group) XXX_Merge(src proto.Message) {
	xxx_messageInfo_EdgeSet_Group.Merge(dst, src)
}
func (m *EdgeSet_Group) XXX_Size() int {
	return xxx_messageInfo_EdgeSet_Group.Size(m)
}
func (m *EdgeSet_Group) XXX_DiscardUnknown() {
	xxx_messageInfo_EdgeSet_Group.DiscardUnknown(m)
}

var xxx_messageInfo_EdgeSet_Group proto.InternalMessageInfo

func (m *EdgeSet_Group) GetEdge() []*EdgeSet_Group_Edge {
	if m != nil {
		return m.Edge
	}
	return nil
}

type EdgeSet_Group_Edge struct {
	TargetTicket         string   `protobuf:"bytes,1,opt,name=target_ticket,json=targetTicket,proto3" json:"target_ticket,omitempty"`
	Ordinal              int32    `protobuf:"varint,2,opt,name=ordinal,proto3" json:"ordinal,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *EdgeSet_Group_Edge) Reset()         { *m = EdgeSet_Group_Edge{} }
func (m *EdgeSet_Group_Edge) String() string { return proto.CompactTextString(m) }
func (*EdgeSet_Group_Edge) ProtoMessage()    {}
func (*EdgeSet_Group_Edge) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{3, 0, 0}
}
func (m *EdgeSet_Group_Edge) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_EdgeSet_Group_Edge.Unmarshal(m, b)
}
func (m *EdgeSet_Group_Edge) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_EdgeSet_Group_Edge.Marshal(b, m, deterministic)
}
func (dst *EdgeSet_Group_Edge) XXX_Merge(src proto.Message) {
	xxx_messageInfo_EdgeSet_Group_Edge.Merge(dst, src)
}
func (m *EdgeSet_Group_Edge) XXX_Size() int {
	return xxx_messageInfo_EdgeSet_Group_Edge.Size(m)
}
func (m *EdgeSet_Group_Edge) XXX_DiscardUnknown() {
	xxx_messageInfo_EdgeSet_Group_Edge.DiscardUnknown(m)
}

var xxx_messageInfo_EdgeSet_Group_Edge proto.InternalMessageInfo

func (m *EdgeSet_Group_Edge) GetTargetTicket() string {
	if m != nil {
		return m.TargetTicket
	}
	return ""
}

func (m *EdgeSet_Group_Edge) GetOrdinal() int32 {
	if m != nil {
		return m.Ordinal
	}
	return 0
}

type EdgesReply struct {
	EdgeSets             map[string]*EdgeSet                  `protobuf:"bytes,1,rep,name=edge_sets,json=edgeSets,proto3" json:"edge_sets,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	Nodes                map[string]*common_go_proto.NodeInfo `protobuf:"bytes,2,rep,name=nodes,proto3" json:"nodes,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	TotalEdgesByKind     map[string]int64                     `protobuf:"bytes,5,rep,name=total_edges_by_kind,json=totalEdgesByKind,proto3" json:"total_edges_by_kind,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"varint,2,opt,name=value,proto3"`
	NextPageToken        string                               `protobuf:"bytes,9,opt,name=next_page_token,json=nextPageToken,proto3" json:"next_page_token,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                             `json:"-"`
	XXX_unrecognized     []byte                               `json:"-"`
	XXX_sizecache        int32                                `json:"-"`
}

func (m *EdgesReply) Reset()         { *m = EdgesReply{} }
func (m *EdgesReply) String() string { return proto.CompactTextString(m) }
func (*EdgesReply) ProtoMessage()    {}
func (*EdgesReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_graph_c34d059ac32f02dd, []int{4}
}
func (m *EdgesReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_EdgesReply.Unmarshal(m, b)
}
func (m *EdgesReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_EdgesReply.Marshal(b, m, deterministic)
}
func (dst *EdgesReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_EdgesReply.Merge(dst, src)
}
func (m *EdgesReply) XXX_Size() int {
	return xxx_messageInfo_EdgesReply.Size(m)
}
func (m *EdgesReply) XXX_DiscardUnknown() {
	xxx_messageInfo_EdgesReply.DiscardUnknown(m)
}

var xxx_messageInfo_EdgesReply proto.InternalMessageInfo

func (m *EdgesReply) GetEdgeSets() map[string]*EdgeSet {
	if m != nil {
		return m.EdgeSets
	}
	return nil
}

func (m *EdgesReply) GetNodes() map[string]*common_go_proto.NodeInfo {
	if m != nil {
		return m.Nodes
	}
	return nil
}

func (m *EdgesReply) GetTotalEdgesByKind() map[string]int64 {
	if m != nil {
		return m.TotalEdgesByKind
	}
	return nil
}

func (m *EdgesReply) GetNextPageToken() string {
	if m != nil {
		return m.NextPageToken
	}
	return ""
}

func init() {
	proto.RegisterType((*NodesRequest)(nil), "kythe.proto.NodesRequest")
	proto.RegisterType((*NodesReply)(nil), "kythe.proto.NodesReply")
	proto.RegisterMapType((map[string]*common_go_proto.NodeInfo)(nil), "kythe.proto.NodesReply.NodesEntry")
	proto.RegisterType((*EdgesRequest)(nil), "kythe.proto.EdgesRequest")
	proto.RegisterType((*EdgeSet)(nil), "kythe.proto.EdgeSet")
	proto.RegisterMapType((map[string]*EdgeSet_Group)(nil), "kythe.proto.EdgeSet.GroupsEntry")
	proto.RegisterType((*EdgeSet_Group)(nil), "kythe.proto.EdgeSet.Group")
	proto.RegisterType((*EdgeSet_Group_Edge)(nil), "kythe.proto.EdgeSet.Group.Edge")
	proto.RegisterType((*EdgesReply)(nil), "kythe.proto.EdgesReply")
	proto.RegisterMapType((map[string]*EdgeSet)(nil), "kythe.proto.EdgesReply.EdgeSetsEntry")
	proto.RegisterMapType((map[string]*common_go_proto.NodeInfo)(nil), "kythe.proto.EdgesReply.NodesEntry")
	proto.RegisterMapType((map[string]int64)(nil), "kythe.proto.EdgesReply.TotalEdgesByKindEntry")
}

func init() { proto.RegisterFile("kythe/proto/graph.proto", fileDescriptor_graph_c34d059ac32f02dd) }

var fileDescriptor_graph_c34d059ac32f02dd = []byte{
	// 598 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xb4, 0x54, 0x51, 0x8f, 0xd2, 0x40,
	0x10, 0xbe, 0x16, 0xca, 0xc1, 0x00, 0x4a, 0xd6, 0xd3, 0xab, 0x55, 0x23, 0xa9, 0xd1, 0x10, 0x13,
	0x39, 0xc3, 0xbd, 0x10, 0x13, 0x7d, 0xc0, 0x90, 0x8b, 0x9a, 0x18, 0x2d, 0xe8, 0x83, 0x31, 0x69,
	0x7a, 0x74, 0xaf, 0xd7, 0xc0, 0x75, 0xb1, 0x5d, 0x2e, 0xf6, 0x9e, 0xfc, 0x01, 0x46, 0xff, 0x81,
	0xff, 0xc5, 0x7f, 0x66, 0x76, 0x76, 0x91, 0x05, 0x4a, 0x7c, 0xf2, 0xa9, 0x3b, 0xb3, 0x33, 0xdf,
	0xce, 0x37, 0xf3, 0x75, 0xe0, 0x70, 0x9a, 0xf3, 0x73, 0x7a, 0x34, 0x4f, 0x19, 0x67, 0x47, 0x51,
	0x1a, 0xcc, 0xcf, 0xbb, 0x78, 0x26, 0x75, 0xbc, 0x90, 0x86, 0x63, 0xeb, 0x51, 0x13, 0x76, 0x71,
	0xc1, 0x12, 0x79, 0xe3, 0xbe, 0x80, 0xc6, 0x5b, 0x16, 0xd2, 0xcc, 0xa3, 0x5f, 0x16, 0x34, 0xe3,
	0xe4, 0x16, 0x54, 0x78, 0x3c, 0x99, 0x52, 0x6e, 0x1b, 0xed, 0x52, 0xa7, 0xe6, 0x29, 0x4b, 0xf8,
	0xcf, 0xe2, 0x19, 0xa7, 0xa9, 0x6d, 0x4a, 0xbf, 0xb4, 0xdc, 0x5f, 0x06, 0x80, 0x02, 0x98, 0xcf,
	0x72, 0xd2, 0x07, 0x2b, 0x11, 0x16, 0x66, 0xd7, 0x7b, 0x6e, 0x57, 0xab, 0xa2, 0xbb, 0x8a, 0x93,
	0xc7, 0x61, 0xc2, 0xd3, 0xdc, 0x93, 0x09, 0xce, 0x47, 0x85, 0x83, 0x4e, 0xd2, 0x82, 0xd2, 0x94,
	0xe6, 0xb6, 0xd1, 0x36, 0x3a, 0x35, 0x4f, 0x1c, 0x49, 0x0f, 0xac, 0xcb, 0x60, 0xb6, 0xa0, 0xb6,
	0xd9, 0x36, 0x3a, 0xf5, 0xde, 0xdd, 0x35, 0x64, 0x45, 0x49, 0x00, 0xbc, 0x4a, 0xce, 0x98, 0x27,
	0x43, 0x9f, 0x99, 0x7d, 0xc3, 0xfd, 0x61, 0x40, 0x63, 0x18, 0x46, 0xff, 0x66, 0x48, 0xa0, 0x3c,
	0x8d, 0x93, 0x50, 0xf1, 0xc3, 0xb3, 0xc6, 0xba, 0xa4, 0xb3, 0x26, 0x77, 0xa0, 0x36, 0x0f, 0x22,
	0xea, 0x67, 0xf1, 0x15, 0xb5, 0xab, 0x6d, 0xa3, 0x63, 0x79, 0x55, 0xe1, 0x18, 0xc5, 0x57, 0x94,
	0xdc, 0x03, 0xc0, 0x4b, 0xce, 0xa6, 0x34, 0xb1, 0x6b, 0x48, 0x01, 0xc3, 0xc7, 0xc2, 0xe1, 0xfe,
	0x36, 0x61, 0x5f, 0x14, 0x34, 0xa2, 0x9c, 0xf4, 0xa1, 0x12, 0xa5, 0x6c, 0x31, 0xcf, 0xf0, 0xd5,
	0x7a, 0xaf, 0xbd, 0xc6, 0x4a, 0x45, 0x75, 0x4f, 0x30, 0x44, 0x76, 0x4b, 0xc5, 0x3b, 0x3f, 0x0d,
	0xb0, 0xd0, 0x4f, 0x8e, 0xa1, 0x4c, 0xc3, 0x88, 0x2a, 0x84, 0xfb, 0xbb, 0x11, 0xd0, 0xf2, 0x30,
	0xd8, 0x19, 0x42, 0x59, 0x58, 0xe4, 0x01, 0x34, 0x79, 0x90, 0x46, 0x94, 0xfb, 0x7f, 0x7b, 0x22,
	0xca, 0x6d, 0x48, 0xe7, 0x58, 0x76, 0xc6, 0x86, 0x7d, 0x96, 0x86, 0x71, 0x12, 0xcc, 0xb0, 0xf9,
	0x96, 0xb7, 0x34, 0x5f, 0x97, 0xab, 0x46, 0xcb, 0x94, 0xbd, 0x72, 0x3e, 0x40, 0x5d, 0x2b, 0xb4,
	0x60, 0x82, 0x4f, 0xd7, 0x27, 0xe8, 0xec, 0xae, 0x54, 0x9b, 0x9f, 0x7a, 0xa2, 0x99, 0xb1, 0x45,
	0x3a, 0xa1, 0xaa, 0x4a, 0xf7, 0x5b, 0x19, 0x40, 0x0d, 0x55, 0xa8, 0x6e, 0x00, 0x35, 0xc1, 0xca,
	0xcf, 0x28, 0x5f, 0x2a, 0xef, 0xe1, 0x16, 0xba, 0x52, 0x9e, 0x7a, 0x48, 0xb5, 0xb3, 0x4a, 0x95,
	0xb9, 0x52, 0xae, 0x59, 0xa0, 0x5c, 0x2d, 0x7f, 0x4b, 0xb9, 0xe4, 0x33, 0xdc, 0xe0, 0x8c, 0x07,
	0x33, 0x5f, 0x60, 0x65, 0xfe, 0x69, 0xee, 0xa3, 0x8e, 0x2c, 0xc4, 0x79, 0xb2, 0x0b, 0x67, 0x2c,
	0x52, 0xd0, 0x1e, 0xe4, 0x6f, 0xe2, 0x24, 0x94, 0x90, 0x2d, 0xbe, 0xe1, 0x26, 0x8f, 0xe0, 0x7a,
	0x42, 0xbf, 0x72, 0x7f, 0x4b, 0x52, 0x4d, 0xe1, 0x7e, 0xb7, 0x94, 0x95, 0xf3, 0x1e, 0x9a, 0x6b,
	0xd4, 0x0a, 0x06, 0xf0, 0x78, 0x7d, 0x00, 0x07, 0x45, 0x03, 0xd0, 0x5a, 0xff, 0xbf, 0x7e, 0x49,
	0xe7, 0x25, 0xdc, 0x2c, 0x64, 0x5f, 0xf0, 0xc4, 0x81, 0xfe, 0x44, 0x49, 0x03, 0xe9, 0x7d, 0x37,
	0xa0, 0x71, 0x22, 0xf6, 0xdd, 0x88, 0xa6, 0x97, 0xf1, 0x84, 0x92, 0xe7, 0x60, 0x61, 0xb5, 0xe4,
	0x76, 0xd1, 0xd2, 0xc1, 0x7f, 0xdf, 0x39, 0xdc, 0xb1, 0x8f, 0xdc, 0x3d, 0x91, 0x8e, 0xf5, 0x6c,
	0xa4, 0xeb, 0xab, 0x63, 0x23, 0x7d, 0x35, 0x4c, 0x77, 0x6f, 0xd0, 0xfa, 0x74, 0x0d, 0xb7, 0xaf,
	0x1f, 0x31, 0x1f, 0xaf, 0x4f, 0x2b, 0xf8, 0x39, 0xfe, 0x13, 0x00, 0x00, 0xff, 0xff, 0x85, 0xde,
	0xdf, 0xdd, 0xa2, 0x05, 0x00, 0x00,
}
