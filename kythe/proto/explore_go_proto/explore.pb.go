// Code generated by protoc-gen-go. DO NOT EDIT.
// source: kythe/proto/explore.proto

package explore_go_proto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import common_go_proto "kythe.io/kythe/proto/common_go_proto"
import storage_go_proto "kythe.io/kythe/proto/storage_go_proto"
import xref_go_proto "kythe.io/kythe/proto/xref_go_proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type NodeData struct {
	Kind                 string                        `protobuf:"bytes,1,opt,name=kind" json:"kind,omitempty"`
	Subkind              string                        `protobuf:"bytes,2,opt,name=subkind" json:"subkind,omitempty"`
	Locations            []*xref_go_proto.Location     `protobuf:"bytes,3,rep,name=locations" json:"locations,omitempty"`
	DefinitionAnchor     string                        `protobuf:"bytes,4,opt,name=definition_anchor,json=definitionAnchor" json:"definition_anchor,omitempty"`
	Code                 *common_go_proto.MarkedSource `protobuf:"bytes,5,opt,name=code" json:"code,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                      `json:"-"`
	XXX_unrecognized     []byte                        `json:"-"`
	XXX_sizecache        int32                         `json:"-"`
}

func (m *NodeData) Reset()         { *m = NodeData{} }
func (m *NodeData) String() string { return proto.CompactTextString(m) }
func (*NodeData) ProtoMessage()    {}
func (*NodeData) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{0}
}
func (m *NodeData) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_NodeData.Unmarshal(m, b)
}
func (m *NodeData) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_NodeData.Marshal(b, m, deterministic)
}
func (dst *NodeData) XXX_Merge(src proto.Message) {
	xxx_messageInfo_NodeData.Merge(dst, src)
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
	NodeData             *NodeData `protobuf:"bytes,1,opt,name=node_data,json=nodeData" json:"node_data,omitempty"`
	Predecessors         *Tickets  `protobuf:"bytes,2,opt,name=predecessors" json:"predecessors,omitempty"`
	Successors           *Tickets  `protobuf:"bytes,3,opt,name=successors" json:"successors,omitempty"`
	XXX_NoUnkeyedLiteral struct{}  `json:"-"`
	XXX_unrecognized     []byte    `json:"-"`
	XXX_sizecache        int32     `json:"-"`
}

func (m *GraphNode) Reset()         { *m = GraphNode{} }
func (m *GraphNode) String() string { return proto.CompactTextString(m) }
func (*GraphNode) ProtoMessage()    {}
func (*GraphNode) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{1}
}
func (m *GraphNode) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_GraphNode.Unmarshal(m, b)
}
func (m *GraphNode) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_GraphNode.Marshal(b, m, deterministic)
}
func (dst *GraphNode) XXX_Merge(src proto.Message) {
	xxx_messageInfo_GraphNode.Merge(dst, src)
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

func (m *GraphNode) GetPredecessors() *Tickets {
	if m != nil {
		return m.Predecessors
	}
	return nil
}

func (m *GraphNode) GetSuccessors() *Tickets {
	if m != nil {
		return m.Successors
	}
	return nil
}

type Graph struct {
	Nodes                map[string]*GraphNode `protobuf:"bytes,1,rep,name=nodes" json:"nodes,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	XXX_NoUnkeyedLiteral struct{}              `json:"-"`
	XXX_unrecognized     []byte                `json:"-"`
	XXX_sizecache        int32                 `json:"-"`
}

func (m *Graph) Reset()         { *m = Graph{} }
func (m *Graph) String() string { return proto.CompactTextString(m) }
func (*Graph) ProtoMessage()    {}
func (*Graph) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{2}
}
func (m *Graph) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Graph.Unmarshal(m, b)
}
func (m *Graph) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Graph.Marshal(b, m, deterministic)
}
func (dst *Graph) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Graph.Merge(dst, src)
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
	IncludedLanguages    []string                  `protobuf:"bytes,1,rep,name=included_languages,json=includedLanguages" json:"included_languages,omitempty"`
	IncludedFiles        []*storage_go_proto.VName `protobuf:"bytes,2,rep,name=included_files,json=includedFiles" json:"included_files,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                  `json:"-"`
	XXX_unrecognized     []byte                    `json:"-"`
	XXX_sizecache        int32                     `json:"-"`
}

func (m *NodeFilter) Reset()         { *m = NodeFilter{} }
func (m *NodeFilter) String() string { return proto.CompactTextString(m) }
func (*NodeFilter) ProtoMessage()    {}
func (*NodeFilter) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{3}
}
func (m *NodeFilter) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_NodeFilter.Unmarshal(m, b)
}
func (m *NodeFilter) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_NodeFilter.Marshal(b, m, deterministic)
}
func (dst *NodeFilter) XXX_Merge(src proto.Message) {
	xxx_messageInfo_NodeFilter.Merge(dst, src)
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
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *Tickets) Reset()         { *m = Tickets{} }
func (m *Tickets) String() string { return proto.CompactTextString(m) }
func (*Tickets) ProtoMessage()    {}
func (*Tickets) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{4}
}
func (m *Tickets) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_Tickets.Unmarshal(m, b)
}
func (m *Tickets) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_Tickets.Marshal(b, m, deterministic)
}
func (dst *Tickets) XXX_Merge(src proto.Message) {
	xxx_messageInfo_Tickets.Merge(dst, src)
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
	TypeTicket           string      `protobuf:"bytes,1,opt,name=type_ticket,json=typeTicket" json:"type_ticket,omitempty"`
	NodeFilter           *NodeFilter `protobuf:"bytes,2,opt,name=node_filter,json=nodeFilter" json:"node_filter,omitempty"`
	XXX_NoUnkeyedLiteral struct{}    `json:"-"`
	XXX_unrecognized     []byte      `json:"-"`
	XXX_sizecache        int32       `json:"-"`
}

func (m *TypeHierarchyRequest) Reset()         { *m = TypeHierarchyRequest{} }
func (m *TypeHierarchyRequest) String() string { return proto.CompactTextString(m) }
func (*TypeHierarchyRequest) ProtoMessage()    {}
func (*TypeHierarchyRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{5}
}
func (m *TypeHierarchyRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_TypeHierarchyRequest.Unmarshal(m, b)
}
func (m *TypeHierarchyRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_TypeHierarchyRequest.Marshal(b, m, deterministic)
}
func (dst *TypeHierarchyRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_TypeHierarchyRequest.Merge(dst, src)
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
	TypeTicket           string   `protobuf:"bytes,1,opt,name=type_ticket,json=typeTicket" json:"type_ticket,omitempty"`
	Graph                *Graph   `protobuf:"bytes,2,opt,name=graph" json:"graph,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *TypeHierarchyReply) Reset()         { *m = TypeHierarchyReply{} }
func (m *TypeHierarchyReply) String() string { return proto.CompactTextString(m) }
func (*TypeHierarchyReply) ProtoMessage()    {}
func (*TypeHierarchyReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{6}
}
func (m *TypeHierarchyReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_TypeHierarchyReply.Unmarshal(m, b)
}
func (m *TypeHierarchyReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_TypeHierarchyReply.Marshal(b, m, deterministic)
}
func (dst *TypeHierarchyReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_TypeHierarchyReply.Merge(dst, src)
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
	FunctionTicket       []string `protobuf:"bytes,1,rep,name=function_ticket,json=functionTicket" json:"function_ticket,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CallersRequest) Reset()         { *m = CallersRequest{} }
func (m *CallersRequest) String() string { return proto.CompactTextString(m) }
func (*CallersRequest) ProtoMessage()    {}
func (*CallersRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{7}
}
func (m *CallersRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CallersRequest.Unmarshal(m, b)
}
func (m *CallersRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CallersRequest.Marshal(b, m, deterministic)
}
func (dst *CallersRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CallersRequest.Merge(dst, src)
}
func (m *CallersRequest) XXX_Size() int {
	return xxx_messageInfo_CallersRequest.Size(m)
}
func (m *CallersRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_CallersRequest.DiscardUnknown(m)
}

var xxx_messageInfo_CallersRequest proto.InternalMessageInfo

func (m *CallersRequest) GetFunctionTicket() []string {
	if m != nil {
		return m.FunctionTicket
	}
	return nil
}

type CallersReply struct {
	FunctionToGraph      map[string]*Graph `protobuf:"bytes,1,rep,name=function_to_graph,json=functionToGraph" json:"function_to_graph,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	XXX_NoUnkeyedLiteral struct{}          `json:"-"`
	XXX_unrecognized     []byte            `json:"-"`
	XXX_sizecache        int32             `json:"-"`
}

func (m *CallersReply) Reset()         { *m = CallersReply{} }
func (m *CallersReply) String() string { return proto.CompactTextString(m) }
func (*CallersReply) ProtoMessage()    {}
func (*CallersReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{8}
}
func (m *CallersReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CallersReply.Unmarshal(m, b)
}
func (m *CallersReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CallersReply.Marshal(b, m, deterministic)
}
func (dst *CallersReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CallersReply.Merge(dst, src)
}
func (m *CallersReply) XXX_Size() int {
	return xxx_messageInfo_CallersReply.Size(m)
}
func (m *CallersReply) XXX_DiscardUnknown() {
	xxx_messageInfo_CallersReply.DiscardUnknown(m)
}

var xxx_messageInfo_CallersReply proto.InternalMessageInfo

func (m *CallersReply) GetFunctionToGraph() map[string]*Graph {
	if m != nil {
		return m.FunctionToGraph
	}
	return nil
}

type CalleesRequest struct {
	FunctionTicket       string   `protobuf:"bytes,1,opt,name=function_ticket,json=functionTicket" json:"function_ticket,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CalleesRequest) Reset()         { *m = CalleesRequest{} }
func (m *CalleesRequest) String() string { return proto.CompactTextString(m) }
func (*CalleesRequest) ProtoMessage()    {}
func (*CalleesRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{9}
}
func (m *CalleesRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CalleesRequest.Unmarshal(m, b)
}
func (m *CalleesRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CalleesRequest.Marshal(b, m, deterministic)
}
func (dst *CalleesRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CalleesRequest.Merge(dst, src)
}
func (m *CalleesRequest) XXX_Size() int {
	return xxx_messageInfo_CalleesRequest.Size(m)
}
func (m *CalleesRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_CalleesRequest.DiscardUnknown(m)
}

var xxx_messageInfo_CalleesRequest proto.InternalMessageInfo

func (m *CalleesRequest) GetFunctionTicket() string {
	if m != nil {
		return m.FunctionTicket
	}
	return ""
}

type CalleesReply struct {
	FunctionToGraph      map[string]*Graph `protobuf:"bytes,1,rep,name=function_to_graph,json=functionToGraph" json:"function_to_graph,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	XXX_NoUnkeyedLiteral struct{}          `json:"-"`
	XXX_unrecognized     []byte            `json:"-"`
	XXX_sizecache        int32             `json:"-"`
}

func (m *CalleesReply) Reset()         { *m = CalleesReply{} }
func (m *CalleesReply) String() string { return proto.CompactTextString(m) }
func (*CalleesReply) ProtoMessage()    {}
func (*CalleesReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{10}
}
func (m *CalleesReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CalleesReply.Unmarshal(m, b)
}
func (m *CalleesReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CalleesReply.Marshal(b, m, deterministic)
}
func (dst *CalleesReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CalleesReply.Merge(dst, src)
}
func (m *CalleesReply) XXX_Size() int {
	return xxx_messageInfo_CalleesReply.Size(m)
}
func (m *CalleesReply) XXX_DiscardUnknown() {
	xxx_messageInfo_CalleesReply.DiscardUnknown(m)
}

var xxx_messageInfo_CalleesReply proto.InternalMessageInfo

func (m *CalleesReply) GetFunctionToGraph() map[string]*Graph {
	if m != nil {
		return m.FunctionToGraph
	}
	return nil
}

type ParametersRequest struct {
	FunctionTickets      []string `protobuf:"bytes,1,rep,name=function_tickets,json=functionTickets" json:"function_tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ParametersRequest) Reset()         { *m = ParametersRequest{} }
func (m *ParametersRequest) String() string { return proto.CompactTextString(m) }
func (*ParametersRequest) ProtoMessage()    {}
func (*ParametersRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{11}
}
func (m *ParametersRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParametersRequest.Unmarshal(m, b)
}
func (m *ParametersRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParametersRequest.Marshal(b, m, deterministic)
}
func (dst *ParametersRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParametersRequest.Merge(dst, src)
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
	FunctionToParameters  map[string]*Tickets  `protobuf:"bytes,1,rep,name=function_to_parameters,json=functionToParameters" json:"function_to_parameters,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	FunctionToReturnValue map[string]string    `protobuf:"bytes,2,rep,name=function_to_return_value,json=functionToReturnValue" json:"function_to_return_value,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	NodeData              map[string]*NodeData `protobuf:"bytes,3,rep,name=node_data,json=nodeData" json:"node_data,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	XXX_NoUnkeyedLiteral  struct{}             `json:"-"`
	XXX_unrecognized      []byte               `json:"-"`
	XXX_sizecache         int32                `json:"-"`
}

func (m *ParametersReply) Reset()         { *m = ParametersReply{} }
func (m *ParametersReply) String() string { return proto.CompactTextString(m) }
func (*ParametersReply) ProtoMessage()    {}
func (*ParametersReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{12}
}
func (m *ParametersReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParametersReply.Unmarshal(m, b)
}
func (m *ParametersReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParametersReply.Marshal(b, m, deterministic)
}
func (dst *ParametersReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParametersReply.Merge(dst, src)
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
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ParentsRequest) Reset()         { *m = ParentsRequest{} }
func (m *ParentsRequest) String() string { return proto.CompactTextString(m) }
func (*ParentsRequest) ProtoMessage()    {}
func (*ParentsRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{13}
}
func (m *ParentsRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParentsRequest.Unmarshal(m, b)
}
func (m *ParentsRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParentsRequest.Marshal(b, m, deterministic)
}
func (dst *ParentsRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParentsRequest.Merge(dst, src)
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
	InputToParents       map[string]*Tickets  `protobuf:"bytes,1,rep,name=input_to_parents,json=inputToParents" json:"input_to_parents,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	NodeData             map[string]*NodeData `protobuf:"bytes,2,rep,name=node_data,json=nodeData" json:"node_data,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	XXX_NoUnkeyedLiteral struct{}             `json:"-"`
	XXX_unrecognized     []byte               `json:"-"`
	XXX_sizecache        int32                `json:"-"`
}

func (m *ParentsReply) Reset()         { *m = ParentsReply{} }
func (m *ParentsReply) String() string { return proto.CompactTextString(m) }
func (*ParentsReply) ProtoMessage()    {}
func (*ParentsReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{14}
}
func (m *ParentsReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ParentsReply.Unmarshal(m, b)
}
func (m *ParentsReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ParentsReply.Marshal(b, m, deterministic)
}
func (dst *ParentsReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ParentsReply.Merge(dst, src)
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

func (m *ParentsReply) GetNodeData() map[string]*NodeData {
	if m != nil {
		return m.NodeData
	}
	return nil
}

type ChildrenRequest struct {
	Tickets              []string `protobuf:"bytes,1,rep,name=tickets" json:"tickets,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *ChildrenRequest) Reset()         { *m = ChildrenRequest{} }
func (m *ChildrenRequest) String() string { return proto.CompactTextString(m) }
func (*ChildrenRequest) ProtoMessage()    {}
func (*ChildrenRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{15}
}
func (m *ChildrenRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ChildrenRequest.Unmarshal(m, b)
}
func (m *ChildrenRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ChildrenRequest.Marshal(b, m, deterministic)
}
func (dst *ChildrenRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ChildrenRequest.Merge(dst, src)
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
	InputToChildren      map[string]*Tickets  `protobuf:"bytes,1,rep,name=input_to_children,json=inputToChildren" json:"input_to_children,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	NodeData             map[string]*NodeData `protobuf:"bytes,2,rep,name=node_data,json=nodeData" json:"node_data,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
	XXX_NoUnkeyedLiteral struct{}             `json:"-"`
	XXX_unrecognized     []byte               `json:"-"`
	XXX_sizecache        int32                `json:"-"`
}

func (m *ChildrenReply) Reset()         { *m = ChildrenReply{} }
func (m *ChildrenReply) String() string { return proto.CompactTextString(m) }
func (*ChildrenReply) ProtoMessage()    {}
func (*ChildrenReply) Descriptor() ([]byte, []int) {
	return fileDescriptor_explore_546c1228df1a8d6d, []int{16}
}
func (m *ChildrenReply) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_ChildrenReply.Unmarshal(m, b)
}
func (m *ChildrenReply) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_ChildrenReply.Marshal(b, m, deterministic)
}
func (dst *ChildrenReply) XXX_Merge(src proto.Message) {
	xxx_messageInfo_ChildrenReply.Merge(dst, src)
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

func (m *ChildrenReply) GetNodeData() map[string]*NodeData {
	if m != nil {
		return m.NodeData
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
	proto.RegisterMapType((map[string]*Graph)(nil), "kythe.proto.CallersReply.FunctionToGraphEntry")
	proto.RegisterType((*CalleesRequest)(nil), "kythe.proto.CalleesRequest")
	proto.RegisterType((*CalleesReply)(nil), "kythe.proto.CalleesReply")
	proto.RegisterMapType((map[string]*Graph)(nil), "kythe.proto.CalleesReply.FunctionToGraphEntry")
	proto.RegisterType((*ParametersRequest)(nil), "kythe.proto.ParametersRequest")
	proto.RegisterType((*ParametersReply)(nil), "kythe.proto.ParametersReply")
	proto.RegisterMapType((map[string]*Tickets)(nil), "kythe.proto.ParametersReply.FunctionToParametersEntry")
	proto.RegisterMapType((map[string]string)(nil), "kythe.proto.ParametersReply.FunctionToReturnValueEntry")
	proto.RegisterMapType((map[string]*NodeData)(nil), "kythe.proto.ParametersReply.NodeDataEntry")
	proto.RegisterType((*ParentsRequest)(nil), "kythe.proto.ParentsRequest")
	proto.RegisterType((*ParentsReply)(nil), "kythe.proto.ParentsReply")
	proto.RegisterMapType((map[string]*Tickets)(nil), "kythe.proto.ParentsReply.InputToParentsEntry")
	proto.RegisterMapType((map[string]*NodeData)(nil), "kythe.proto.ParentsReply.NodeDataEntry")
	proto.RegisterType((*ChildrenRequest)(nil), "kythe.proto.ChildrenRequest")
	proto.RegisterType((*ChildrenReply)(nil), "kythe.proto.ChildrenReply")
	proto.RegisterMapType((map[string]*Tickets)(nil), "kythe.proto.ChildrenReply.InputToChildrenEntry")
	proto.RegisterMapType((map[string]*NodeData)(nil), "kythe.proto.ChildrenReply.NodeDataEntry")
}

func init() { proto.RegisterFile("kythe/proto/explore.proto", fileDescriptor_explore_546c1228df1a8d6d) }

var fileDescriptor_explore_546c1228df1a8d6d = []byte{
	// 1033 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xcc, 0x56, 0xcd, 0x6e, 0xdb, 0x46,
	0x10, 0x0e, 0x2d, 0xab, 0xb6, 0x46, 0xb6, 0x6c, 0x6d, 0x15, 0x57, 0x56, 0xd2, 0xda, 0x65, 0x0e,
	0x51, 0xed, 0x46, 0x06, 0xe4, 0xa0, 0x75, 0x7a, 0x28, 0xd0, 0x3a, 0x76, 0x52, 0xc0, 0x0d, 0x0c,
	0x26, 0x75, 0x8a, 0x16, 0x85, 0xc0, 0x90, 0x23, 0x89, 0x10, 0xc5, 0x65, 0x96, 0xa4, 0x11, 0xbd,
	0x44, 0x5f, 0xa5, 0xc7, 0xde, 0x7b, 0xee, 0xa1, 0xe7, 0xde, 0xfa, 0x26, 0xc5, 0xee, 0x72, 0xa9,
	0xa5, 0x4c, 0x2a, 0x45, 0xd1, 0x02, 0xb9, 0x71, 0xe7, 0xe7, 0x9b, 0xf9, 0x66, 0x66, 0x67, 0x09,
	0xbb, 0x93, 0x59, 0x3c, 0xc6, 0xa3, 0x90, 0xd1, 0x98, 0x1e, 0xe1, 0x9b, 0xd0, 0xa7, 0x0c, 0x7b,
	0xe2, 0x44, 0xea, 0x42, 0x25, 0x0f, 0x9d, 0xb6, 0x6e, 0xe7, 0xd0, 0xe9, 0x94, 0x06, 0xa9, 0x26,
	0x87, 0x10, 0xc5, 0x94, 0xd9, 0x23, 0xe5, 0xb4, 0xa3, 0xab, 0xde, 0x30, 0x1c, 0x4a, 0xb9, 0xf9,
	0x87, 0x01, 0xeb, 0xcf, 0xa8, 0x8b, 0x8f, 0xed, 0xd8, 0x26, 0x04, 0x56, 0x27, 0x5e, 0xe0, 0xb6,
	0x8d, 0x7d, 0xa3, 0x5b, 0xb3, 0xc4, 0x37, 0x69, 0xc3, 0x5a, 0x94, 0xbc, 0x12, 0xe2, 0x15, 0x21,
	0x56, 0x47, 0x72, 0x0c, 0x35, 0x9f, 0x3a, 0x76, 0xec, 0xd1, 0x20, 0x6a, 0x57, 0xf6, 0x2b, 0xdd,
	0x7a, 0xff, 0x76, 0x4f, 0x4b, 0xb4, 0x77, 0x91, 0x6a, 0xad, 0xb9, 0x1d, 0x39, 0x84, 0xa6, 0x8b,
	0x43, 0x2f, 0xf0, 0xf8, 0x71, 0x60, 0x07, 0xce, 0x98, 0xb2, 0xf6, 0xaa, 0x00, 0xde, 0x9e, 0x2b,
	0xbe, 0x12, 0x72, 0xf2, 0x10, 0x56, 0x1d, 0xea, 0x62, 0xbb, 0xba, 0x6f, 0x74, 0xeb, 0xfd, 0xfd,
	0x1c, 0x78, 0x4a, 0xfc, 0x5b, 0x9b, 0x4d, 0xd0, 0x7d, 0x4e, 0x13, 0xe6, 0xa0, 0x25, 0xac, 0xcd,
	0x5f, 0x0c, 0xa8, 0x3d, 0x61, 0x76, 0x38, 0xe6, 0xbc, 0x48, 0x1f, 0x6a, 0x01, 0x75, 0x71, 0xe0,
	0xda, 0xb1, 0x2d, 0x88, 0x2d, 0x66, 0xa9, 0xd8, 0x5b, 0xeb, 0x81, 0xaa, 0xc3, 0x09, 0x6c, 0x84,
	0x0c, 0x5d, 0x74, 0x30, 0x8a, 0x28, 0x8b, 0x04, 0xf1, 0x7a, 0xbf, 0x95, 0x73, 0x7b, 0xe1, 0x39,
	0x13, 0x8c, 0x23, 0x2b, 0x67, 0x49, 0x1e, 0x02, 0x44, 0x89, 0xa3, 0xfc, 0x2a, 0x4b, 0xfc, 0x34,
	0x3b, 0xf3, 0x67, 0x03, 0xaa, 0x22, 0x63, 0x72, 0x0c, 0x55, 0x9e, 0x45, 0xd4, 0x36, 0x44, 0x3d,
	0x3f, 0xcc, 0xb9, 0x0a, 0x13, 0x91, 0x6f, 0x74, 0x16, 0xc4, 0x6c, 0x66, 0x49, 0xdb, 0xce, 0x25,
	0xc0, 0x5c, 0x48, 0xb6, 0xa1, 0x32, 0xc1, 0x59, 0xda, 0x43, 0xfe, 0x49, 0x3e, 0x85, 0xea, 0xb5,
	0xed, 0x27, 0x98, 0xf2, 0xd8, 0xb9, 0x09, 0xca, 0xdd, 0x2d, 0x69, 0xf4, 0xc5, 0xca, 0x89, 0x61,
	0x5e, 0x4b, 0xc4, 0x73, 0xcf, 0x8f, 0x91, 0x91, 0x07, 0x40, 0xbc, 0xc0, 0xf1, 0x13, 0x17, 0xdd,
	0x81, 0x6f, 0x07, 0xa3, 0xc4, 0x1e, 0xa5, 0x19, 0xd6, 0xac, 0xa6, 0xd2, 0x5c, 0x28, 0x05, 0x79,
	0x04, 0x8d, 0xcc, 0x7c, 0xe8, 0xf9, 0xc8, 0xeb, 0xc7, 0xc9, 0x90, 0x5c, 0xdc, 0xab, 0x67, 0xf6,
	0x14, 0xad, 0x4d, 0x65, 0x79, 0xce, 0x0d, 0xcd, 0x7b, 0xb0, 0x96, 0xd6, 0x87, 0xcf, 0x5d, 0x2c,
	0x3f, 0xd3, 0x48, 0xea, 0x68, 0xbe, 0x86, 0xd6, 0x8b, 0x59, 0x88, 0x4f, 0x3d, 0x64, 0x36, 0x73,
	0xc6, 0x33, 0x0b, 0x5f, 0x27, 0x18, 0xc5, 0x64, 0x0f, 0xea, 0xf1, 0x2c, 0xc4, 0x81, 0xb4, 0x4b,
	0x0b, 0x00, 0x5c, 0x24, 0x31, 0xc9, 0x09, 0xd4, 0xc5, 0x28, 0x0c, 0x05, 0xad, 0xb4, 0x1a, 0x1f,
	0xdc, 0x18, 0x06, 0xc9, 0xda, 0x82, 0x20, 0xfb, 0x36, 0x07, 0x40, 0x16, 0x42, 0x86, 0xfe, 0xec,
	0xed, 0x01, 0xbb, 0x50, 0x1d, 0xf1, 0xf2, 0xa6, 0xa1, 0xc8, 0xcd, 0xc2, 0x5b, 0xd2, 0xc0, 0x7c,
	0x04, 0x8d, 0x53, 0xdb, 0xf7, 0x91, 0x45, 0x8a, 0xcd, 0x7d, 0xd8, 0x1a, 0x26, 0x81, 0x23, 0xae,
	0x49, 0x16, 0x80, 0xd7, 0xa1, 0xa1, 0xc4, 0x32, 0x88, 0xf9, 0x9b, 0x01, 0x1b, 0x99, 0x2f, 0x4f,
	0xeb, 0x07, 0x68, 0xce, 0x3d, 0xe9, 0x40, 0x66, 0x20, 0xe7, 0xa9, 0x97, 0xcb, 0x40, 0xf7, 0xea,
	0x9d, 0x2b, 0x54, 0x2a, 0x12, 0x93, 0x03, 0x96, 0xa5, 0x90, 0x4a, 0x3b, 0x57, 0xd0, 0x2a, 0x32,
	0x2c, 0x18, 0xba, 0x6e, 0x7e, 0xe8, 0x0a, 0xb9, 0xcf, 0x07, 0x4e, 0xf1, 0xc7, 0xe5, 0xfc, 0x8d,
	0x65, 0xfc, 0xf1, 0x5f, 0xf1, 0xc7, 0x77, 0x82, 0xff, 0x97, 0xd0, 0xbc, 0xb4, 0x99, 0x3d, 0xc5,
	0x58, 0x1b, 0x81, 0x4f, 0x60, 0x7b, 0xa1, 0x04, 0xea, 0x2e, 0x6c, 0xe5, 0x6b, 0x10, 0x99, 0xbf,
	0xae, 0xc2, 0x96, 0x0e, 0xc0, 0xeb, 0xe0, 0xc3, 0x8e, 0x5e, 0x87, 0x30, 0x53, 0xa7, 0xc5, 0xf8,
	0x2c, 0x97, 0xd2, 0x82, 0xb7, 0x56, 0x8f, 0xb9, 0x46, 0x16, 0xa5, 0x35, 0x2c, 0x50, 0x91, 0x10,
	0xda, 0x7a, 0x34, 0x86, 0x71, 0xc2, 0x82, 0x81, 0x2a, 0x01, 0x8f, 0xf7, 0xf9, 0x3f, 0x8c, 0x67,
	0x09, 0xd7, 0x2b, 0xee, 0x29, 0x03, 0xde, 0x1e, 0x16, 0xe9, 0xc8, 0x13, 0x7d, 0xb3, 0xcb, 0xf7,
	0xe7, 0x60, 0x69, 0x08, 0xb5, 0xe9, 0x25, 0x6a, 0xb6, 0xee, 0x3b, 0x3f, 0xc1, 0x6e, 0x29, 0xdb,
	0x82, 0xce, 0x1e, 0xe4, 0x3b, 0x5b, 0xbc, 0xde, 0xe7, 0xbd, 0xed, 0x3c, 0x85, 0x4e, 0x39, 0xb9,
	0x02, 0xfc, 0x96, 0x8e, 0x5f, 0xd3, 0x91, 0x2c, 0xd8, 0xcc, 0x71, 0x28, 0x70, 0x3e, 0xcc, 0x27,
	0x57, 0xf2, 0xd4, 0x69, 0x93, 0x77, 0x00, 0x8d, 0x4b, 0x9b, 0x61, 0x10, 0x67, 0x63, 0x57, 0xbe,
	0x79, 0xff, 0x5c, 0x81, 0x8d, 0xcc, 0x98, 0x8f, 0xd8, 0x4b, 0xd8, 0xf6, 0x82, 0x30, 0x89, 0xd3,
	0xf9, 0xe2, 0x8a, 0x74, 0xb8, 0x1e, 0x2c, 0x76, 0x22, 0x73, 0xea, 0x7d, 0xc3, 0x3d, 0x44, 0xa1,
	0xb9, 0x4c, 0x36, 0xa3, 0xe1, 0xe5, 0x84, 0xe4, 0xb1, 0xde, 0x5b, 0x39, 0x3e, 0xf7, 0xcb, 0x11,
	0xcb, 0x1a, 0xfb, 0x12, 0xde, 0x2f, 0x08, 0xf6, 0x1f, 0xb4, 0xf4, 0xff, 0x68, 0xc4, 0x21, 0x6c,
	0x9d, 0x8e, 0x3d, 0xdf, 0x65, 0x18, 0xbc, 0xbd, 0x13, 0x7f, 0xad, 0xc0, 0xe6, 0xdc, 0x9a, 0xb7,
	0xe2, 0x47, 0x68, 0x66, 0xad, 0x70, 0x52, 0x4d, 0xda, 0x8b, 0xa3, 0xfc, 0xd6, 0xd3, 0xdd, 0x54,
	0x33, 0x94, 0x30, 0x5d, 0x7b, 0x5e, 0x5e, 0x4a, 0xce, 0x6e, 0xb6, 0xa3, 0xbb, 0x04, 0xb4, 0xac,
	0x1f, 0xdf, 0x43, 0xab, 0x28, 0xde, 0xbb, 0xd9, 0x90, 0xfe, 0xef, 0x15, 0x68, 0x9c, 0xc9, 0xdf,
	0xf0, 0xe7, 0xc8, 0xae, 0x3d, 0x07, 0xc9, 0x77, 0xb0, 0x99, 0xfb, 0x0f, 0x20, 0x1f, 0xe7, 0x13,
	0x2b, 0xf8, 0x2d, 0xe9, 0xec, 0x2d, 0x33, 0x09, 0xfd, 0x99, 0x79, 0x8b, 0x9c, 0xc2, 0x5a, 0xfa,
	0x16, 0x93, 0x3b, 0xc5, 0x2f, 0xb4, 0x84, 0xda, 0x2d, 0x7d, 0xbe, 0x35, 0x10, 0x2c, 0x04, 0xc1,
	0x65, 0x20, 0x98, 0x81, 0x5c, 0x00, 0x68, 0x3b, 0xfd, 0xa3, 0xd2, 0x75, 0x2a, 0xa1, 0xee, 0x2e,
	0x5b, 0xb7, 0x32, 0x25, 0x75, 0xa1, 0xef, 0x14, 0xdf, 0xde, 0xa2, 0x94, 0xf4, 0xab, 0x6d, 0xde,
	0x22, 0xe7, 0xb0, 0x9e, 0xcd, 0xe1, 0xdd, 0x92, 0xa1, 0x93, 0x30, 0x9d, 0xf2, 0x91, 0x34, 0x6f,
	0x7d, 0x7d, 0x0f, 0xf6, 0x1c, 0x3a, 0xed, 0x8d, 0x28, 0x1d, 0xf9, 0xd8, 0x73, 0xf1, 0x3a, 0xa6,
	0xd4, 0x8f, 0x74, 0x97, 0x4b, 0xe3, 0xd5, 0x7b, 0xe2, 0xe3, 0xf8, 0xef, 0x00, 0x00, 0x00, 0xff,
	0xff, 0xe5, 0x40, 0x81, 0xa4, 0x8c, 0x0d, 0x00, 0x00,
}
