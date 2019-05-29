// Code generated by protoc-gen-go. DO NOT EDIT.
// source: kythe/proto/explore.proto

package explore_go_proto

import (
	fmt "fmt"
	proto "github.com/golang/protobuf/proto"
	common_go_proto "kythe.io/kythe/proto/common_go_proto"
	storage_go_proto "kythe.io/kythe/proto/storage_go_proto"
	xref_go_proto "kythe.io/kythe/proto/xref_go_proto"
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

type NodeData struct {
	Kind                 string                        `protobuf:"bytes,1,opt,name=kind,proto3" json:"kind,omitempty"`
	Subkind              string                        `protobuf:"bytes,2,opt,name=subkind,proto3" json:"subkind,omitempty"`
	Locations            []*xref_go_proto.Location     `protobuf:"bytes,3,rep,name=locations,proto3" json:"locations,omitempty"`
	DefinitionAnchor     string                        `protobuf:"bytes,4,opt,name=definition_anchor,json=definitionAnchor,proto3" json:"definition_anchor,omitempty"`
	Code                 *common_go_proto.MarkedSource `protobuf:"bytes,5,opt,name=code,proto3" json:"code,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                      `json:"-"`
	XXX_unrecognized     []byte                        `json:"-"`
	XXX_sizecache        int32                         `json:"-"`
}

func (m *NodeData) Reset()         { *m = NodeData{} }
func (m *NodeData) String() string { return proto.CompactTextString(m) }
func (*NodeData) ProtoMessage()    {}
func (*NodeData) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{0}
}

func (m *NodeData) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_NodeData.Unmarshal(m, b)
}
func (m *NodeData) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_NodeData.Marshal(b, m, deterministic)
}
func (m *NodeData) XXX_Merge(src proto.Message) {
	xxx_messageInfo_NodeData.Merge(m, src)
}
func (m *NodeData) XXX_Size() int {
	return xxx_messageInfo_NodeData.Size(m)
}
func (m *NodeData) XXX_DiscardUnknown() {
	xxx_messageInfo_NodeData.DiscardUnknown(m)
}

var xxx_messageInfo_NodeData proto.InternalMessageInfo

func (m *NodeData) GetKind() string {
	if m != nil {
		return m.Kind
	}
	return ""
}

func (m *NodeData) GetSubkind() string {
	if m != nil {
		return m.Subkind
	}
	return ""
}

func (m *NodeData) GetLocations() []*xref_go_proto.Location {
	if m != nil {
		return m.Locations
	}
	return nil
}

func (m *NodeData) GetDefinitionAnchor() string {
	if m != nil {
		return m.DefinitionAnchor
	}
	return ""
}

func (m *NodeData) GetCode() *common_go_proto.MarkedSource {
	if m != nil {
		return m.Code
	}
	return nil
}

type GraphNode struct {
	NodeData             *NodeData `protobuf:"bytes,1,opt,name=node_data,json=nodeData,proto3" json:"node_data,omitempty"`
	Predecessors         []string  `protobuf:"bytes,2,rep,name=predecessors,proto3" json:"predecessors,omitempty"`
	Successors           []string  `protobuf:"bytes,3,rep,name=successors,proto3" json:"successors,omitempty"`
	XXX_NoUnkeyedLiteral struct{}  `json:"-"`
	XXX_unrecognized     []byte    `json:"-"`
	XXX_sizecache        int32     `json:"-"`
}

func (m *GraphNode) Reset()         { *m = GraphNode{} }
func (m *GraphNode) String() string { return proto.CompactTextString(m) }
func (*GraphNode) ProtoMessage()    {}
func (*GraphNode) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{1}
}

func (m *GraphNode) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_GraphNode.Unmarshal(m, b)
}
func (m *GraphNode) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_GraphNode.Marshal(b, m, deterministic)
}
func (m *GraphNode) XXX_Merge(src proto.Message) {
	xxx_messageInfo_GraphNode.Merge(m, src)
}
func (m *GraphNode) XXX_Size() int {
	return xxx_messageInfo_GraphNode.Size(m)
}
func (m *GraphNode) XXX_DiscardUnknown() {
	xxx_messageInfo_GraphNode.DiscardUnknown(m)
}

var xxx_messageInfo_GraphNode proto.InternalMessageInfo

func (m *GraphNode) GetNodeData() *NodeData {
	if m != nil {
		return m.NodeData
	}
	return nil
}

func (m *GraphNode) GetPredecessors() []string {
	if m != nil {
		return m.Predecessors
	}
	return nil
}

func (m *GraphNode) GetSuccessors() []string {
	if m != nil {
		return m.Successors
	}
	return nil
}

type Graph struct {
	Nodes                map[string]*GraphNode `protobuf:"bytes,1,rep,name=nodes,proto3" json:"nodes,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral struct{}              `json:"-"`
	XXX_unrecognized     []byte                `json:"-"`
	XXX_sizecache        int32                 `json:"-"`
}

func (m *Graph) Reset()         { *m = Graph{} }
func (m *Graph) String() string { return proto.CompactTextString(m) }
func (*Graph) ProtoMessage()    {}
func (*Graph) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{2}
}

func (m *Graph) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Graph.Unmarshal(m, b)
}
func (m *Graph) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Graph.Marshal(b, m, deterministic)
}
func (m *Graph) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Graph.Merge(m, src)
}
func (m *Graph) XXX_Size() int {
	return xxx_messageInfo_Graph.Size(m)
}
func (m *Graph) XXX_DiscardUnknown() {
	xxx_messageInfo_Graph.DiscardUnknown(m)
}

var xxx_messageInfo_Graph proto.InternalMessageInfo

func (m *Graph) GetNodes() map[string]*GraphNode {
	if m != nil {
		return m.Nodes
	}
	return nil
}

type NodeFilter struct {
	IncludedLanguages    []string                  `protobuf:"bytes,1,rep,name=included_languages,json=includedLanguages,proto3" json:"included_languages,omitempty"`
	IncludedFiles        []*storage_go_proto.VName `protobuf:"bytes,2,rep,name=included_files,json=includedFiles,proto3" json:"included_files,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                  `json:"-"`
	XXX_unrecognized     []byte                    `json:"-"`
	XXX_sizecache        int32                     `json:"-"`
}

func (m *NodeFilter) Reset()         { *m = NodeFilter{} }
func (m *NodeFilter) String() string { return proto.CompactTextString(m) }
func (*NodeFilter) ProtoMessage()    {}
func (*NodeFilter) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{3}
}

func (m *NodeFilter) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_NodeFilter.Unmarshal(m, b)
}
func (m *NodeFilter) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_NodeFilter.Marshal(b, m, deterministic)
}
func (m *NodeFilter) XXX_Merge(src proto.Message) {
	xxx_messageInfo_NodeFilter.Merge(m, src)
}
func (m *NodeFilter) XXX_Size() int {
	return xxx_messageInfo_NodeFilter.Size(m)
}
func (m *NodeFilter) XXX_DiscardUnknown() {
	xxx_messageInfo_NodeFilter.DiscardUnknown(m)
}

var xxx_messageInfo_NodeFilter proto.InternalMessageInfo

func (m *NodeFilter) GetIncludedLanguages() []string {
	if m != nil {
		return m.IncludedLanguages
	}
	return nil
}

func (m *NodeFilter) GetIncludedFiles() []*storage_go_proto.VName {
	if m != nil {
		return m.IncludedFiles
	}
	return nil
}

type Tickets struct {
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets,proto3" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *Tickets) Reset()         { *m = Tickets{} }
func (m *Tickets) String() string { return proto.CompactTextString(m) }
func (*Tickets) ProtoMessage()    {}
func (*Tickets) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{4}
}

func (m *Tickets) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Tickets.Unmarshal(m, b)
}
func (m *Tickets) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Tickets.Marshal(b, m, deterministic)
}
func (m *Tickets) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Tickets.Merge(m, src)
}
func (m *Tickets) XXX_Size() int {
	return xxx_messageInfo_Tickets.Size(m)
}
func (m *Tickets) XXX_DiscardUnknown() {
	xxx_messageInfo_Tickets.DiscardUnknown(m)
}

var xxx_messageInfo_Tickets proto.InternalMessageInfo

func (m *Tickets) GetTickets() []string {
	if m != nil {
		return m.Tickets
	}
	return nil
}

type TypeHierarchyRequest struct {
	TypeTicket           string      `protobuf:"bytes,1,opt,name=type_ticket,json=typeTicket,proto3" json:"type_ticket,omitempty"`
	NodeFilter           *NodeFilter `protobuf:"bytes,2,opt,name=node_filter,json=nodeFilter,proto3" json:"node_filter,omitempty"`
	XXX_NoUnkeyedLiteral struct{}    `json:"-"`
	XXX_unrecognized     []byte      `json:"-"`
	XXX_sizecache        int32       `json:"-"`
}

func (m *TypeHierarchyRequest) Reset()         { *m = TypeHierarchyRequest{} }
func (m *TypeHierarchyRequest) String() string { return proto.CompactTextString(m) }
func (*TypeHierarchyRequest) ProtoMessage()    {}
func (*TypeHierarchyRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{5}
}

func (m *TypeHierarchyRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_TypeHierarchyRequest.Unmarshal(m, b)
}
func (m *TypeHierarchyRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_TypeHierarchyRequest.Marshal(b, m, deterministic)
}
func (m *TypeHierarchyRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_TypeHierarchyRequest.Merge(m, src)
}
func (m *TypeHierarchyRequest) XXX_Size() int {
	return xxx_messageInfo_TypeHierarchyRequest.Size(m)
}
func (m *TypeHierarchyRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_TypeHierarchyRequest.DiscardUnknown(m)
}

var xxx_messageInfo_TypeHierarchyRequest proto.InternalMessageInfo

func (m *TypeHierarchyRequest) GetTypeTicket() string {
	if m != nil {
		return m.TypeTicket
	}
	return ""
}

func (m *TypeHierarchyRequest) GetNodeFilter() *NodeFilter {
	if m != nil {
		return m.NodeFilter
	}
	return nil
}

type TypeHierarchyReply struct {
	TypeTicket           string   `protobuf:"bytes,1,opt,name=type_ticket,json=typeTicket,proto3" json:"type_ticket,omitempty"`
	Graph                *Graph   `protobuf:"bytes,2,opt,name=graph,proto3" json:"graph,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *TypeHierarchyReply) Reset()         { *m = TypeHierarchyReply{} }
func (m *TypeHierarchyReply) String() string { return proto.CompactTextString(m) }
func (*TypeHierarchyReply) ProtoMessage()    {}
func (*TypeHierarchyReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{6}
}

func (m *TypeHierarchyReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_TypeHierarchyReply.Unmarshal(m, b)
}
func (m *TypeHierarchyReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_TypeHierarchyReply.Marshal(b, m, deterministic)
}
func (m *TypeHierarchyReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_TypeHierarchyReply.Merge(m, src)
}
func (m *TypeHierarchyReply) XXX_Size() int {
	return xxx_messageInfo_TypeHierarchyReply.Size(m)
}
func (m *TypeHierarchyReply) XXX_DiscardUnknown() {
	xxx_messageInfo_TypeHierarchyReply.DiscardUnknown(m)
}

var xxx_messageInfo_TypeHierarchyReply proto.InternalMessageInfo

func (m *TypeHierarchyReply) GetTypeTicket() string {
	if m != nil {
		return m.TypeTicket
	}
	return ""
}

func (m *TypeHierarchyReply) GetGraph() *Graph {
	if m != nil {
		return m.Graph
	}
	return nil
}

type CallersRequest struct {
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets,proto3" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CallersRequest) Reset()         { *m = CallersRequest{} }
func (m *CallersRequest) String() string { return proto.CompactTextString(m) }
func (*CallersRequest) ProtoMessage()    {}
func (*CallersRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{7}
}

func (m *CallersRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CallersRequest.Unmarshal(m, b)
}
func (m *CallersRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CallersRequest.Marshal(b, m, deterministic)
}
func (m *CallersRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CallersRequest.Merge(m, src)
}
func (m *CallersRequest) XXX_Size() int {
	return xxx_messageInfo_CallersRequest.Size(m)
}
func (m *CallersRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_CallersRequest.DiscardUnknown(m)
}

var xxx_messageInfo_CallersRequest proto.InternalMessageInfo

func (m *CallersRequest) GetTickets() []string {
	if m != nil {
		return m.Tickets
	}
	return nil
}

type CallersReply struct {
	Graph                *Graph   `protobuf:"bytes,1,opt,name=graph,proto3" json:"graph,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CallersReply) Reset()         { *m = CallersReply{} }
func (m *CallersReply) String() string { return proto.CompactTextString(m) }
func (*CallersReply) ProtoMessage()    {}
func (*CallersReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{8}
}

func (m *CallersReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CallersReply.Unmarshal(m, b)
}
func (m *CallersReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CallersReply.Marshal(b, m, deterministic)
}
func (m *CallersReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CallersReply.Merge(m, src)
}
func (m *CallersReply) XXX_Size() int {
	return xxx_messageInfo_CallersReply.Size(m)
}
func (m *CallersReply) XXX_DiscardUnknown() {
	xxx_messageInfo_CallersReply.DiscardUnknown(m)
}

var xxx_messageInfo_CallersReply proto.InternalMessageInfo

func (m *CallersReply) GetGraph() *Graph {
	if m != nil {
		return m.Graph
	}
	return nil
}

type CalleesRequest struct {
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets,proto3" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CalleesRequest) Reset()         { *m = CalleesRequest{} }
func (m *CalleesRequest) String() string { return proto.CompactTextString(m) }
func (*CalleesRequest) ProtoMessage()    {}
func (*CalleesRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{9}
}

func (m *CalleesRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CalleesRequest.Unmarshal(m, b)
}
func (m *CalleesRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CalleesRequest.Marshal(b, m, deterministic)
}
func (m *CalleesRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CalleesRequest.Merge(m, src)
}
func (m *CalleesRequest) XXX_Size() int {
	return xxx_messageInfo_CalleesRequest.Size(m)
}
func (m *CalleesRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_CalleesRequest.DiscardUnknown(m)
}

var xxx_messageInfo_CalleesRequest proto.InternalMessageInfo

func (m *CalleesRequest) GetTickets() []string {
	if m != nil {
		return m.Tickets
	}
	return nil
}

type CalleesReply struct {
	Graph                *Graph   `protobuf:"bytes,1,opt,name=graph,proto3" json:"graph,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CalleesReply) Reset()         { *m = CalleesReply{} }
func (m *CalleesReply) String() string { return proto.CompactTextString(m) }
func (*CalleesReply) ProtoMessage()    {}
func (*CalleesReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{10}
}

func (m *CalleesReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CalleesReply.Unmarshal(m, b)
}
func (m *CalleesReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CalleesReply.Marshal(b, m, deterministic)
}
func (m *CalleesReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CalleesReply.Merge(m, src)
}
func (m *CalleesReply) XXX_Size() int {
	return xxx_messageInfo_CalleesReply.Size(m)
}
func (m *CalleesReply) XXX_DiscardUnknown() {
	xxx_messageInfo_CalleesReply.DiscardUnknown(m)
}

var xxx_messageInfo_CalleesReply proto.InternalMessageInfo

func (m *CalleesReply) GetGraph() *Graph {
	if m != nil {
		return m.Graph
	}
	return nil
}

type ParametersRequest struct {
	FunctionTickets      []string `protobuf:"bytes,1,rep,name=function_tickets,json=functionTickets,proto3" json:"function_tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ParametersRequest) Reset()         { *m = ParametersRequest{} }
func (m *ParametersRequest) String() string { return proto.CompactTextString(m) }
func (*ParametersRequest) ProtoMessage()    {}
func (*ParametersRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{11}
}

func (m *ParametersRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParametersRequest.Unmarshal(m, b)
}
func (m *ParametersRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParametersRequest.Marshal(b, m, deterministic)
}
func (m *ParametersRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParametersRequest.Merge(m, src)
}
func (m *ParametersRequest) XXX_Size() int {
	return xxx_messageInfo_ParametersRequest.Size(m)
}
func (m *ParametersRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_ParametersRequest.DiscardUnknown(m)
}

var xxx_messageInfo_ParametersRequest proto.InternalMessageInfo

func (m *ParametersRequest) GetFunctionTickets() []string {
	if m != nil {
		return m.FunctionTickets
	}
	return nil
}

type ParametersReply struct {
	FunctionToParameters  map[string]*Tickets  `protobuf:"bytes,1,rep,name=function_to_parameters,json=functionToParameters,proto3" json:"function_to_parameters,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	FunctionToReturnValue map[string]string    `protobuf:"bytes,2,rep,name=function_to_return_value,json=functionToReturnValue,proto3" json:"function_to_return_value,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	NodeData              map[string]*NodeData `protobuf:"bytes,3,rep,name=node_data,json=nodeData,proto3" json:"node_data,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral  struct{}             `json:"-"`
	XXX_unrecognized      []byte               `json:"-"`
	XXX_sizecache         int32                `json:"-"`
}

func (m *ParametersReply) Reset()         { *m = ParametersReply{} }
func (m *ParametersReply) String() string { return proto.CompactTextString(m) }
func (*ParametersReply) ProtoMessage()    {}
func (*ParametersReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{12}
}

func (m *ParametersReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParametersReply.Unmarshal(m, b)
}
func (m *ParametersReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParametersReply.Marshal(b, m, deterministic)
}
func (m *ParametersReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParametersReply.Merge(m, src)
}
func (m *ParametersReply) XXX_Size() int {
	return xxx_messageInfo_ParametersReply.Size(m)
}
func (m *ParametersReply) XXX_DiscardUnknown() {
	xxx_messageInfo_ParametersReply.DiscardUnknown(m)
}

var xxx_messageInfo_ParametersReply proto.InternalMessageInfo

func (m *ParametersReply) GetFunctionToParameters() map[string]*Tickets {
	if m != nil {
		return m.FunctionToParameters
	}
	return nil
}

func (m *ParametersReply) GetFunctionToReturnValue() map[string]string {
	if m != nil {
		return m.FunctionToReturnValue
	}
	return nil
}

func (m *ParametersReply) GetNodeData() map[string]*NodeData {
	if m != nil {
		return m.NodeData
	}
	return nil
}

type ParentsRequest struct {
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets,proto3" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ParentsRequest) Reset()         { *m = ParentsRequest{} }
func (m *ParentsRequest) String() string { return proto.CompactTextString(m) }
func (*ParentsRequest) ProtoMessage()    {}
func (*ParentsRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{13}
}

func (m *ParentsRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParentsRequest.Unmarshal(m, b)
}
func (m *ParentsRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParentsRequest.Marshal(b, m, deterministic)
}
func (m *ParentsRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParentsRequest.Merge(m, src)
}
func (m *ParentsRequest) XXX_Size() int {
	return xxx_messageInfo_ParentsRequest.Size(m)
}
func (m *ParentsRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_ParentsRequest.DiscardUnknown(m)
}

var xxx_messageInfo_ParentsRequest proto.InternalMessageInfo

func (m *ParentsRequest) GetTickets() []string {
	if m != nil {
		return m.Tickets
	}
	return nil
}

type ParentsReply struct {
	InputToParents       map[string]*Tickets `protobuf:"bytes,1,rep,name=input_to_parents,json=inputToParents,proto3" json:"input_to_parents,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral struct{}            `json:"-"`
	XXX_unrecognized     []byte              `json:"-"`
	XXX_sizecache        int32               `json:"-"`
}

func (m *ParentsReply) Reset()         { *m = ParentsReply{} }
func (m *ParentsReply) String() string { return proto.CompactTextString(m) }
func (*ParentsReply) ProtoMessage()    {}
func (*ParentsReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{14}
}

func (m *ParentsReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParentsReply.Unmarshal(m, b)
}
func (m *ParentsReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParentsReply.Marshal(b, m, deterministic)
}
func (m *ParentsReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParentsReply.Merge(m, src)
}
func (m *ParentsReply) XXX_Size() int {
	return xxx_messageInfo_ParentsReply.Size(m)
}
func (m *ParentsReply) XXX_DiscardUnknown() {
	xxx_messageInfo_ParentsReply.DiscardUnknown(m)
}

var xxx_messageInfo_ParentsReply proto.InternalMessageInfo

func (m *ParentsReply) GetInputToParents() map[string]*Tickets {
	if m != nil {
		return m.InputToParents
	}
	return nil
}

type ChildrenRequest struct {
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets,proto3" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ChildrenRequest) Reset()         { *m = ChildrenRequest{} }
func (m *ChildrenRequest) String() string { return proto.CompactTextString(m) }
func (*ChildrenRequest) ProtoMessage()    {}
func (*ChildrenRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{15}
}

func (m *ChildrenRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ChildrenRequest.Unmarshal(m, b)
}
func (m *ChildrenRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ChildrenRequest.Marshal(b, m, deterministic)
}
func (m *ChildrenRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ChildrenRequest.Merge(m, src)
}
func (m *ChildrenRequest) XXX_Size() int {
	return xxx_messageInfo_ChildrenRequest.Size(m)
}
func (m *ChildrenRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_ChildrenRequest.DiscardUnknown(m)
}

var xxx_messageInfo_ChildrenRequest proto.InternalMessageInfo

func (m *ChildrenRequest) GetTickets() []string {
	if m != nil {
		return m.Tickets
	}
	return nil
}

type ChildrenReply struct {
	InputToChildren      map[string]*Tickets `protobuf:"bytes,1,rep,name=input_to_children,json=inputToChildren,proto3" json:"input_to_children,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral struct{}            `json:"-"`
	XXX_unrecognized     []byte              `json:"-"`
	XXX_sizecache        int32               `json:"-"`
}

func (m *ChildrenReply) Reset()         { *m = ChildrenReply{} }
func (m *ChildrenReply) String() string { return proto.CompactTextString(m) }
func (*ChildrenReply) ProtoMessage()    {}
func (*ChildrenReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_ee5b2ef3873ea484, []int{16}
}

func (m *ChildrenReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ChildrenReply.Unmarshal(m, b)
}
func (m *ChildrenReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ChildrenReply.Marshal(b, m, deterministic)
}
func (m *ChildrenReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ChildrenReply.Merge(m, src)
}
func (m *ChildrenReply) XXX_Size() int {
	return xxx_messageInfo_ChildrenReply.Size(m)
}
func (m *ChildrenReply) XXX_DiscardUnknown() {
	xxx_messageInfo_ChildrenReply.DiscardUnknown(m)
}

var xxx_messageInfo_ChildrenReply proto.InternalMessageInfo

func (m *ChildrenReply) GetInputToChildren() map[string]*Tickets {
	if m != nil {
		return m.InputToChildren
	}
	return nil
}

func init() {
	proto.RegisterType((*NodeData)(nil), "kythe.proto.NodeData")
	proto.RegisterType((*GraphNode)(nil), "kythe.proto.GraphNode")
	proto.RegisterType((*Graph)(nil), "kythe.proto.Graph")
	proto.RegisterMapType((map[string]*GraphNode)(nil), "kythe.proto.Graph.NodesEntry")
	proto.RegisterType((*NodeFilter)(nil), "kythe.proto.NodeFilter")
	proto.RegisterType((*Tickets)(nil), "kythe.proto.Tickets")
	proto.RegisterType((*TypeHierarchyRequest)(nil), "kythe.proto.TypeHierarchyRequest")
	proto.RegisterType((*TypeHierarchyReply)(nil), "kythe.proto.TypeHierarchyReply")
	proto.RegisterType((*CallersRequest)(nil), "kythe.proto.CallersRequest")
	proto.RegisterType((*CallersReply)(nil), "kythe.proto.CallersReply")
	proto.RegisterType((*CalleesRequest)(nil), "kythe.proto.CalleesRequest")
	proto.RegisterType((*CalleesReply)(nil), "kythe.proto.CalleesReply")
	proto.RegisterType((*ParametersRequest)(nil), "kythe.proto.ParametersRequest")
	proto.RegisterType((*ParametersReply)(nil), "kythe.proto.ParametersReply")
	proto.RegisterMapType((map[string]*Tickets)(nil), "kythe.proto.ParametersReply.FunctionToParametersEntry")
	proto.RegisterMapType((map[string]string)(nil), "kythe.proto.ParametersReply.FunctionToReturnValueEntry")
	proto.RegisterMapType((map[string]*NodeData)(nil), "kythe.proto.ParametersReply.NodeDataEntry")
	proto.RegisterType((*ParentsRequest)(nil), "kythe.proto.ParentsRequest")
	proto.RegisterType((*ParentsReply)(nil), "kythe.proto.ParentsReply")
	proto.RegisterMapType((map[string]*Tickets)(nil), "kythe.proto.ParentsReply.InputToParentsEntry")
	proto.RegisterType((*ChildrenRequest)(nil), "kythe.proto.ChildrenRequest")
	proto.RegisterType((*ChildrenReply)(nil), "kythe.proto.ChildrenReply")
	proto.RegisterMapType((map[string]*Tickets)(nil), "kythe.proto.ChildrenReply.InputToChildrenEntry")
}

func init() { proto.RegisterFile("kythe/proto/explore.proto", fileDescriptor_ee5b2ef3873ea484) }

var fileDescriptor_ee5b2ef3873ea484 = []byte{
	// 958 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xa4, 0x56, 0xcf, 0x73, 0xdb, 0x54,
	0x10, 0x8e, 0xea, 0x98, 0xc4, 0xeb, 0xc4, 0x71, 0x1e, 0x6e, 0x70, 0x44, 0x69, 0x8c, 0xb8, 0x98,
	0x84, 0x3a, 0x33, 0x0e, 0x3f, 0x02, 0x07, 0x66, 0x20, 0x34, 0x2d, 0x33, 0xa1, 0x93, 0x51, 0x4b,
	0xcb, 0xc0, 0x30, 0x1a, 0x55, 0x5a, 0xdb, 0x1a, 0xcb, 0x7a, 0xea, 0x93, 0x94, 0xa9, 0xcf, 0xdc,
	0xf9, 0x77, 0xb8, 0x72, 0xe2, 0xc4, 0x81, 0x3f, 0x89, 0x79, 0x3f, 0x24, 0xeb, 0x39, 0x72, 0x9a,
	0x4e, 0x4f, 0x91, 0x76, 0xbf, 0xfd, 0xbe, 0xf7, 0xed, 0x3e, 0x6d, 0x0c, 0xfb, 0xd3, 0x79, 0x3a,
	0xc1, 0xe3, 0x98, 0xd1, 0x94, 0x1e, 0xe3, 0xeb, 0x38, 0xa4, 0x0c, 0x07, 0xe2, 0x8d, 0x34, 0x45,
	0x4a, 0xbe, 0x98, 0xdd, 0x32, 0xce, 0xa3, 0xb3, 0x19, 0x8d, 0x54, 0x46, 0x63, 0x48, 0x52, 0xca,
	0xdc, 0x71, 0x5e, 0xb4, 0x57, 0x4e, 0xbd, 0x66, 0x38, 0x92, 0x71, 0xeb, 0x3f, 0x03, 0x36, 0x9f,
	0x50, 0x1f, 0x7f, 0x70, 0x53, 0x97, 0x10, 0x58, 0x9f, 0x06, 0x91, 0xdf, 0x35, 0x7a, 0x46, 0xbf,
	0x61, 0x8b, 0x67, 0xd2, 0x85, 0x8d, 0x24, 0x7b, 0x29, 0xc2, 0x77, 0x44, 0x38, 0x7f, 0x25, 0x27,
	0xd0, 0x08, 0xa9, 0xe7, 0xa6, 0x01, 0x8d, 0x92, 0x6e, 0xad, 0x57, 0xeb, 0x37, 0x87, 0x77, 0x07,
	0xa5, 0x83, 0x0e, 0x2e, 0x54, 0xd6, 0x5e, 0xe0, 0xc8, 0x11, 0xec, 0xfa, 0x38, 0x0a, 0xa2, 0x80,
	0xbf, 0x3a, 0x6e, 0xe4, 0x4d, 0x28, 0xeb, 0xae, 0x0b, 0xe2, 0xf6, 0x22, 0xf1, 0x9d, 0x88, 0x93,
	0xcf, 0x61, 0xdd, 0xa3, 0x3e, 0x76, 0xeb, 0x3d, 0xa3, 0xdf, 0x1c, 0xf6, 0x34, 0x72, 0x65, 0xfc,
	0x27, 0x97, 0x4d, 0xd1, 0x7f, 0x4a, 0x33, 0xe6, 0xa1, 0x2d, 0xd0, 0xd6, 0x1f, 0x06, 0x34, 0x1e,
	0x31, 0x37, 0x9e, 0x70, 0x5f, 0x64, 0x08, 0x8d, 0x88, 0xfa, 0xe8, 0xf8, 0x6e, 0xea, 0x0a, 0x63,
	0xcb, 0xa7, 0xcc, 0xdd, 0xdb, 0x9b, 0x51, 0xde, 0x07, 0x0b, 0xb6, 0x62, 0x86, 0x3e, 0x7a, 0x98,
	0x24, 0x94, 0x25, 0xdd, 0x3b, 0xbd, 0x5a, 0xbf, 0x61, 0x6b, 0x31, 0x72, 0x1f, 0x20, 0xc9, 0xbc,
	0x1c, 0x51, 0x13, 0x88, 0x52, 0xc4, 0xfa, 0xd3, 0x80, 0xba, 0x38, 0x05, 0x39, 0x81, 0x3a, 0x67,
	0x4e, 0xba, 0x86, 0xe8, 0xd1, 0x47, 0x9a, 0xba, 0x80, 0x88, 0x33, 0x24, 0x0f, 0xa3, 0x94, 0xcd,
	0x6d, 0x89, 0x35, 0x2f, 0x01, 0x16, 0x41, 0xd2, 0x86, 0xda, 0x14, 0xe7, 0x6a, 0x2e, 0xfc, 0x91,
	0x7c, 0x06, 0xf5, 0x2b, 0x37, 0xcc, 0x50, 0x0c, 0xa5, 0x39, 0xdc, 0xbb, 0x4e, 0xca, 0xcb, 0x6d,
	0x09, 0xfa, 0xe6, 0xce, 0xa9, 0x61, 0x5d, 0x49, 0xc6, 0xf3, 0x20, 0x4c, 0x91, 0x91, 0x07, 0x40,
	0x82, 0xc8, 0x0b, 0x33, 0x1f, 0x7d, 0x27, 0x74, 0xa3, 0x71, 0xe6, 0x8e, 0xd5, 0x09, 0x1b, 0xf6,
	0x6e, 0x9e, 0xb9, 0xc8, 0x13, 0xe4, 0x6b, 0x68, 0x15, 0xf0, 0x51, 0x10, 0xa2, 0xec, 0x49, 0x73,
	0x48, 0x34, 0xdd, 0xe7, 0x4f, 0xdc, 0x19, 0xda, 0xdb, 0x39, 0xf2, 0x9c, 0x03, 0xad, 0x4f, 0x60,
	0xe3, 0x59, 0xe0, 0x4d, 0x31, 0x4d, 0xf8, 0x5d, 0x4a, 0xe5, 0xa3, 0x52, 0xca, 0x5f, 0xad, 0x57,
	0xd0, 0x79, 0x36, 0x8f, 0xf1, 0x71, 0x80, 0xcc, 0x65, 0xde, 0x64, 0x6e, 0xe3, 0xab, 0x0c, 0x93,
	0x94, 0x1c, 0x40, 0x33, 0x9d, 0xc7, 0xe8, 0x48, 0x9c, 0x6a, 0x00, 0xf0, 0x90, 0xe4, 0x24, 0xa7,
	0xd0, 0x14, 0xe3, 0x1d, 0x09, 0x5b, 0xaa, 0x1b, 0x1f, 0x5c, 0x1b, 0xb0, 0x74, 0x6d, 0x43, 0x54,
	0x3c, 0x5b, 0x0e, 0x90, 0x25, 0xc9, 0x38, 0x9c, 0xbf, 0x59, 0xb0, 0x0f, 0xf5, 0x31, 0x6f, 0xaf,
	0x92, 0x22, 0xd7, 0x1b, 0x6f, 0x4b, 0x80, 0x75, 0x08, 0xad, 0x33, 0x37, 0x0c, 0x91, 0x25, 0xb9,
	0x9b, 0xd5, 0xfe, 0x4f, 0x61, 0xab, 0xc0, 0xf2, 0x63, 0x14, 0x2a, 0xc6, 0x6d, 0x55, 0xf0, 0x2d,
	0x54, 0xf0, 0xad, 0x55, 0xbe, 0x85, 0xdd, 0x4b, 0x97, 0xb9, 0x33, 0x4c, 0x4b, 0x76, 0x3e, 0x85,
	0xf6, 0x28, 0x8b, 0x3c, 0xf1, 0x25, 0xeb, 0x8a, 0x3b, 0x79, 0x5c, 0x4d, 0xde, 0xfa, 0x6b, 0x1d,
	0x76, 0xca, 0x04, 0x5c, 0x3d, 0x84, 0xbd, 0x45, 0x39, 0x75, 0xe2, 0x22, 0xad, 0x3e, 0x94, 0x2f,
	0xb5, 0xe3, 0x2c, 0x55, 0x0f, 0xce, 0x73, 0x05, 0xba, 0xc8, 0xc8, 0x2f, 0xa8, 0x33, 0xaa, 0x48,
	0x91, 0x18, 0xba, 0x65, 0x35, 0x86, 0x69, 0xc6, 0x22, 0x27, 0xff, 0x86, 0xb8, 0xde, 0x57, 0xb7,
	0xd4, 0xb3, 0x45, 0xe9, 0x73, 0x5e, 0x29, 0x05, 0xef, 0x8e, 0xaa, 0x72, 0xe4, 0x51, 0x79, 0xf3,
	0xc8, 0xfd, 0x78, 0x78, 0xa3, 0x44, 0xbe, 0x89, 0x24, 0x6b, 0xb1, 0x8e, 0xcc, 0xdf, 0x61, 0x7f,
	0xa5, 0xdb, 0x8a, 0xd5, 0x70, 0xa8, 0xaf, 0x86, 0x8e, 0xa6, 0xa9, 0x06, 0x52, 0x5a, 0x0c, 0xe6,
	0x63, 0x30, 0x57, 0x9b, 0xab, 0xe0, 0xef, 0x94, 0xf9, 0x1b, 0x65, 0x26, 0x1b, 0xb6, 0x35, 0x0f,
	0x15, 0xc5, 0x47, 0xfa, 0xe1, 0x56, 0xac, 0xe2, 0xd2, 0xda, 0x3a, 0x84, 0xd6, 0xa5, 0xcb, 0x30,
	0x4a, 0x6f, 0x71, 0xbf, 0xff, 0x36, 0x60, 0xab, 0x00, 0xf3, 0x2b, 0xf6, 0x02, 0xda, 0x41, 0x14,
	0x67, 0xa9, 0xba, 0x5f, 0x3c, 0xa1, 0x2e, 0xd7, 0x83, 0xe5, 0x49, 0x14, 0x45, 0x83, 0x1f, 0x79,
	0x85, 0x68, 0x34, 0x8f, 0xc9, 0x61, 0xb4, 0x02, 0x2d, 0x68, 0xbe, 0x80, 0xf7, 0x2b, 0x60, 0xef,
	0x3e, 0x0c, 0xeb, 0x08, 0x76, 0xce, 0x26, 0x41, 0xe8, 0x33, 0x8c, 0xde, 0xec, 0xf7, 0x1f, 0x03,
	0xb6, 0x17, 0x68, 0x6e, 0xf8, 0x37, 0xd8, 0x2d, 0x0c, 0x7b, 0x2a, 0xa3, 0x1c, 0x1f, 0x6b, 0xd2,
	0x5a, 0x59, 0x6e, 0x39, 0x0f, 0x4a, 0xcf, 0x3b, 0x81, 0x1e, 0x35, 0x7f, 0x81, 0x4e, 0x15, 0xf0,
	0xdd, 0x5d, 0x0f, 0xff, 0xad, 0x41, 0xeb, 0xa1, 0xfc, 0xc5, 0xf3, 0x14, 0xd9, 0x55, 0xe0, 0x21,
	0x39, 0x83, 0x0d, 0xb5, 0x11, 0xc9, 0x87, 0xfa, 0xc9, 0xb5, 0x9d, 0x6a, 0xee, 0x57, 0x27, 0xe3,
	0x70, 0x6e, 0xad, 0x15, 0x24, 0x58, 0x49, 0x82, 0x37, 0x91, 0x60, 0x99, 0x44, 0x0d, 0x79, 0x89,
	0x44, 0xbf, 0x97, 0x4b, 0x24, 0xe5, 0x2b, 0x65, 0xad, 0x91, 0x73, 0xd8, 0xcc, 0x9b, 0x46, 0xee,
	0xad, 0x98, 0x84, 0xa4, 0x31, 0x57, 0xcf, 0xc9, 0x5a, 0x23, 0x3f, 0xc3, 0xb6, 0xf6, 0x5f, 0x8b,
	0x7c, 0xac, 0xf7, 0xb6, 0xe2, 0x9f, 0xa8, 0x79, 0x70, 0x13, 0x44, 0xd2, 0x5e, 0x00, 0x94, 0x76,
	0xe5, 0xfd, 0x95, 0x6b, 0x4a, 0x12, 0xde, 0xbb, 0x69, 0x8d, 0x59, 0x6b, 0xdf, 0x7f, 0x01, 0x07,
	0x1e, 0x9d, 0x0d, 0xc6, 0x94, 0x8e, 0x43, 0x1c, 0xf8, 0x78, 0x95, 0x52, 0x1a, 0x26, 0xe5, 0xa2,
	0x4b, 0xe3, 0xd7, 0xb6, 0xfa, 0x89, 0xeb, 0x8c, 0xa9, 0x23, 0x62, 0x2f, 0xdf, 0x13, 0x7f, 0x4e,
	0xfe, 0x0f, 0x00, 0x00, 0xff, 0xff, 0x44, 0xae, 0xfe, 0x27, 0x09, 0x0b, 0x00, 0x00,
}
