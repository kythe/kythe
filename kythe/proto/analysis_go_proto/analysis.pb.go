// Code generated by protoc-gen-go. DO NOT EDIT.
// source: kythe/proto/analysis.proto

package analysis_go_proto

import (
	fmt "fmt"
	proto "github.com/golang/protobuf/proto"
	any "github.com/golang/protobuf/ptypes/any"
	storage_go_proto "kythe.io/kythe/proto/storage_go_proto"
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

type AnalysisResult_Status int32

const (
	AnalysisResult_COMPLETE        AnalysisResult_Status = 0
	AnalysisResult_INCOMPLETE      AnalysisResult_Status = 1
	AnalysisResult_INVALID_REQUEST AnalysisResult_Status = 2
)

var AnalysisResult_Status_name = map[int32]string{
	0: "COMPLETE",
	1: "INCOMPLETE",
	2: "INVALID_REQUEST",
}

var AnalysisResult_Status_value = map[string]int32{
	"COMPLETE":        0,
	"INCOMPLETE":      1,
	"INVALID_REQUEST": 2,
}

func (x AnalysisResult_Status) String() string {
	return proto.EnumName(AnalysisResult_Status_name, int32(x))
}

func (AnalysisResult_Status) EnumDescriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{2, 0}
}

type AnalysisRequest struct {
	Compilation          *CompilationUnit `protobuf:"bytes,1,opt,name=compilation,proto3" json:"compilation,omitempty"`
	FileDataService      string           `protobuf:"bytes,2,opt,name=file_data_service,json=fileDataService,proto3" json:"file_data_service,omitempty"`
	Revision             string           `protobuf:"bytes,3,opt,name=revision,proto3" json:"revision,omitempty"`
	BuildId              string           `protobuf:"bytes,4,opt,name=build_id,json=buildId,proto3" json:"build_id,omitempty"`
	XXX_NoUnkeyedLiteral struct{}         `json:"-"`
	XXX_unrecognized     []byte           `json:"-"`
	XXX_sizecache        int32            `json:"-"`
}

func (m *AnalysisRequest) Reset()         { *m = AnalysisRequest{} }
func (m *AnalysisRequest) String() string { return proto.CompactTextString(m) }
func (*AnalysisRequest) ProtoMessage()    {}
func (*AnalysisRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{0}
}

func (m *AnalysisRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AnalysisRequest.Unmarshal(m, b)
}
func (m *AnalysisRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AnalysisRequest.Marshal(b, m, deterministic)
}
func (m *AnalysisRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AnalysisRequest.Merge(m, src)
}
func (m *AnalysisRequest) XXX_Size() int {
	return xxx_messageInfo_AnalysisRequest.Size(m)
}
func (m *AnalysisRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_AnalysisRequest.DiscardUnknown(m)
}

var xxx_messageInfo_AnalysisRequest proto.InternalMessageInfo

func (m *AnalysisRequest) GetCompilation() *CompilationUnit {
	if m != nil {
		return m.Compilation
	}
	return nil
}

func (m *AnalysisRequest) GetFileDataService() string {
	if m != nil {
		return m.FileDataService
	}
	return ""
}

func (m *AnalysisRequest) GetRevision() string {
	if m != nil {
		return m.Revision
	}
	return ""
}

func (m *AnalysisRequest) GetBuildId() string {
	if m != nil {
		return m.BuildId
	}
	return ""
}

type AnalysisOutput struct {
	Value                []byte          `protobuf:"bytes,1,opt,name=value,proto3" json:"value,omitempty"`
	FinalResult          *AnalysisResult `protobuf:"bytes,10,opt,name=final_result,json=finalResult,proto3" json:"final_result,omitempty"`
	XXX_NoUnkeyedLiteral struct{}        `json:"-"`
	XXX_unrecognized     []byte          `json:"-"`
	XXX_sizecache        int32           `json:"-"`
}

func (m *AnalysisOutput) Reset()         { *m = AnalysisOutput{} }
func (m *AnalysisOutput) String() string { return proto.CompactTextString(m) }
func (*AnalysisOutput) ProtoMessage()    {}
func (*AnalysisOutput) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{1}
}

func (m *AnalysisOutput) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AnalysisOutput.Unmarshal(m, b)
}
func (m *AnalysisOutput) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AnalysisOutput.Marshal(b, m, deterministic)
}
func (m *AnalysisOutput) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AnalysisOutput.Merge(m, src)
}
func (m *AnalysisOutput) XXX_Size() int {
	return xxx_messageInfo_AnalysisOutput.Size(m)
}
func (m *AnalysisOutput) XXX_DiscardUnknown() {
	xxx_messageInfo_AnalysisOutput.DiscardUnknown(m)
}

var xxx_messageInfo_AnalysisOutput proto.InternalMessageInfo

func (m *AnalysisOutput) GetValue() []byte {
	if m != nil {
		return m.Value
	}
	return nil
}

func (m *AnalysisOutput) GetFinalResult() *AnalysisResult {
	if m != nil {
		return m.FinalResult
	}
	return nil
}

type AnalysisResult struct {
	Status               AnalysisResult_Status `protobuf:"varint,1,opt,name=status,proto3,enum=kythe.proto.AnalysisResult_Status" json:"status,omitempty"`
	Summary              string                `protobuf:"bytes,2,opt,name=summary,proto3" json:"summary,omitempty"`
	XXX_NoUnkeyedLiteral struct{}              `json:"-"`
	XXX_unrecognized     []byte                `json:"-"`
	XXX_sizecache        int32                 `json:"-"`
}

func (m *AnalysisResult) Reset()         { *m = AnalysisResult{} }
func (m *AnalysisResult) String() string { return proto.CompactTextString(m) }
func (*AnalysisResult) ProtoMessage()    {}
func (*AnalysisResult) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{2}
}

func (m *AnalysisResult) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AnalysisResult.Unmarshal(m, b)
}
func (m *AnalysisResult) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AnalysisResult.Marshal(b, m, deterministic)
}
func (m *AnalysisResult) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AnalysisResult.Merge(m, src)
}
func (m *AnalysisResult) XXX_Size() int {
	return xxx_messageInfo_AnalysisResult.Size(m)
}
func (m *AnalysisResult) XXX_DiscardUnknown() {
	xxx_messageInfo_AnalysisResult.DiscardUnknown(m)
}

var xxx_messageInfo_AnalysisResult proto.InternalMessageInfo

func (m *AnalysisResult) GetStatus() AnalysisResult_Status {
	if m != nil {
		return m.Status
	}
	return AnalysisResult_COMPLETE
}

func (m *AnalysisResult) GetSummary() string {
	if m != nil {
		return m.Summary
	}
	return ""
}

type CompilationUnit struct {
	VName                *storage_go_proto.VName      `protobuf:"bytes,1,opt,name=v_name,json=vName,proto3" json:"v_name,omitempty"`
	RequiredInput        []*CompilationUnit_FileInput `protobuf:"bytes,3,rep,name=required_input,json=requiredInput,proto3" json:"required_input,omitempty"`
	HasCompileErrors     bool                         `protobuf:"varint,4,opt,name=has_compile_errors,json=hasCompileErrors,proto3" json:"has_compile_errors,omitempty"`
	Argument             []string                     `protobuf:"bytes,5,rep,name=argument,proto3" json:"argument,omitempty"`
	SourceFile           []string                     `protobuf:"bytes,6,rep,name=source_file,json=sourceFile,proto3" json:"source_file,omitempty"`
	OutputKey            string                       `protobuf:"bytes,7,opt,name=output_key,json=outputKey,proto3" json:"output_key,omitempty"`
	WorkingDirectory     string                       `protobuf:"bytes,8,opt,name=working_directory,json=workingDirectory,proto3" json:"working_directory,omitempty"`
	EntryContext         string                       `protobuf:"bytes,9,opt,name=entry_context,json=entryContext,proto3" json:"entry_context,omitempty"`
	Environment          []*CompilationUnit_Env       `protobuf:"bytes,10,rep,name=environment,proto3" json:"environment,omitempty"`
	Details              []*any.Any                   `protobuf:"bytes,11,rep,name=details,proto3" json:"details,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                     `json:"-"`
	XXX_unrecognized     []byte                       `json:"-"`
	XXX_sizecache        int32                        `json:"-"`
}

func (m *CompilationUnit) Reset()         { *m = CompilationUnit{} }
func (m *CompilationUnit) String() string { return proto.CompactTextString(m) }
func (*CompilationUnit) ProtoMessage()    {}
func (*CompilationUnit) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{3}
}

func (m *CompilationUnit) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CompilationUnit.Unmarshal(m, b)
}
func (m *CompilationUnit) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CompilationUnit.Marshal(b, m, deterministic)
}
func (m *CompilationUnit) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CompilationUnit.Merge(m, src)
}
func (m *CompilationUnit) XXX_Size() int {
	return xxx_messageInfo_CompilationUnit.Size(m)
}
func (m *CompilationUnit) XXX_DiscardUnknown() {
	xxx_messageInfo_CompilationUnit.DiscardUnknown(m)
}

var xxx_messageInfo_CompilationUnit proto.InternalMessageInfo

func (m *CompilationUnit) GetVName() *storage_go_proto.VName {
	if m != nil {
		return m.VName
	}
	return nil
}

func (m *CompilationUnit) GetRequiredInput() []*CompilationUnit_FileInput {
	if m != nil {
		return m.RequiredInput
	}
	return nil
}

func (m *CompilationUnit) GetHasCompileErrors() bool {
	if m != nil {
		return m.HasCompileErrors
	}
	return false
}

func (m *CompilationUnit) GetArgument() []string {
	if m != nil {
		return m.Argument
	}
	return nil
}

func (m *CompilationUnit) GetSourceFile() []string {
	if m != nil {
		return m.SourceFile
	}
	return nil
}

func (m *CompilationUnit) GetOutputKey() string {
	if m != nil {
		return m.OutputKey
	}
	return ""
}

func (m *CompilationUnit) GetWorkingDirectory() string {
	if m != nil {
		return m.WorkingDirectory
	}
	return ""
}

func (m *CompilationUnit) GetEntryContext() string {
	if m != nil {
		return m.EntryContext
	}
	return ""
}

func (m *CompilationUnit) GetEnvironment() []*CompilationUnit_Env {
	if m != nil {
		return m.Environment
	}
	return nil
}

func (m *CompilationUnit) GetDetails() []*any.Any {
	if m != nil {
		return m.Details
	}
	return nil
}

type CompilationUnit_FileInput struct {
	VName                *storage_go_proto.VName `protobuf:"bytes,1,opt,name=v_name,json=vName,proto3" json:"v_name,omitempty"`
	Info                 *FileInfo               `protobuf:"bytes,2,opt,name=info,proto3" json:"info,omitempty"`
	Details              []*any.Any              `protobuf:"bytes,4,rep,name=details,proto3" json:"details,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                `json:"-"`
	XXX_unrecognized     []byte                  `json:"-"`
	XXX_sizecache        int32                   `json:"-"`
}

func (m *CompilationUnit_FileInput) Reset()         { *m = CompilationUnit_FileInput{} }
func (m *CompilationUnit_FileInput) String() string { return proto.CompactTextString(m) }
func (*CompilationUnit_FileInput) ProtoMessage()    {}
func (*CompilationUnit_FileInput) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{3, 0}
}

func (m *CompilationUnit_FileInput) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CompilationUnit_FileInput.Unmarshal(m, b)
}
func (m *CompilationUnit_FileInput) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CompilationUnit_FileInput.Marshal(b, m, deterministic)
}
func (m *CompilationUnit_FileInput) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CompilationUnit_FileInput.Merge(m, src)
}
func (m *CompilationUnit_FileInput) XXX_Size() int {
	return xxx_messageInfo_CompilationUnit_FileInput.Size(m)
}
func (m *CompilationUnit_FileInput) XXX_DiscardUnknown() {
	xxx_messageInfo_CompilationUnit_FileInput.DiscardUnknown(m)
}

var xxx_messageInfo_CompilationUnit_FileInput proto.InternalMessageInfo

func (m *CompilationUnit_FileInput) GetVName() *storage_go_proto.VName {
	if m != nil {
		return m.VName
	}
	return nil
}

func (m *CompilationUnit_FileInput) GetInfo() *FileInfo {
	if m != nil {
		return m.Info
	}
	return nil
}

func (m *CompilationUnit_FileInput) GetDetails() []*any.Any {
	if m != nil {
		return m.Details
	}
	return nil
}

type CompilationUnit_Env struct {
	Name                 string   `protobuf:"bytes,1,opt,name=name,proto3" json:"name,omitempty"`
	Value                string   `protobuf:"bytes,2,opt,name=value,proto3" json:"value,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *CompilationUnit_Env) Reset()         { *m = CompilationUnit_Env{} }
func (m *CompilationUnit_Env) String() string { return proto.CompactTextString(m) }
func (*CompilationUnit_Env) ProtoMessage()    {}
func (*CompilationUnit_Env) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{3, 1}
}

func (m *CompilationUnit_Env) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CompilationUnit_Env.Unmarshal(m, b)
}
func (m *CompilationUnit_Env) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CompilationUnit_Env.Marshal(b, m, deterministic)
}
func (m *CompilationUnit_Env) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CompilationUnit_Env.Merge(m, src)
}
func (m *CompilationUnit_Env) XXX_Size() int {
	return xxx_messageInfo_CompilationUnit_Env.Size(m)
}
func (m *CompilationUnit_Env) XXX_DiscardUnknown() {
	xxx_messageInfo_CompilationUnit_Env.DiscardUnknown(m)
}

var xxx_messageInfo_CompilationUnit_Env proto.InternalMessageInfo

func (m *CompilationUnit_Env) GetName() string {
	if m != nil {
		return m.Name
	}
	return ""
}

func (m *CompilationUnit_Env) GetValue() string {
	if m != nil {
		return m.Value
	}
	return ""
}

type KzipInfo struct {
	Corpora              map[string]*KzipInfo_CorpusInfo `protobuf:"bytes,1,rep,name=corpora,proto3" json:"corpora,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	TotalUnits           int32                           `protobuf:"varint,3,opt,name=total_units,json=totalUnits,proto3" json:"total_units,omitempty"`
	TotalSources         int32                           `protobuf:"varint,4,opt,name=total_sources,json=totalSources,proto3" json:"total_sources,omitempty"`
	TotalRequiredInputs  int32                           `protobuf:"varint,5,opt,name=total_required_inputs,json=totalRequiredInputs,proto3" json:"total_required_inputs,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                        `json:"-"`
	XXX_unrecognized     []byte                          `json:"-"`
	XXX_sizecache        int32                           `json:"-"`
}

func (m *KzipInfo) Reset()         { *m = KzipInfo{} }
func (m *KzipInfo) String() string { return proto.CompactTextString(m) }
func (*KzipInfo) ProtoMessage()    {}
func (*KzipInfo) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{4}
}

func (m *KzipInfo) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_KzipInfo.Unmarshal(m, b)
}
func (m *KzipInfo) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_KzipInfo.Marshal(b, m, deterministic)
}
func (m *KzipInfo) XXX_Merge(src proto.Message) {
	xxx_messageInfo_KzipInfo.Merge(m, src)
}
func (m *KzipInfo) XXX_Size() int {
	return xxx_messageInfo_KzipInfo.Size(m)
}
func (m *KzipInfo) XXX_DiscardUnknown() {
	xxx_messageInfo_KzipInfo.DiscardUnknown(m)
}

var xxx_messageInfo_KzipInfo proto.InternalMessageInfo

func (m *KzipInfo) GetCorpora() map[string]*KzipInfo_CorpusInfo {
	if m != nil {
		return m.Corpora
	}
	return nil
}

func (m *KzipInfo) GetTotalUnits() int32 {
	if m != nil {
		return m.TotalUnits
	}
	return 0
}

func (m *KzipInfo) GetTotalSources() int32 {
	if m != nil {
		return m.TotalSources
	}
	return 0
}

func (m *KzipInfo) GetTotalRequiredInputs() int32 {
	if m != nil {
		return m.TotalRequiredInputs
	}
	return 0
}

type KzipInfo_CorpusInfo struct {
	CompilationUnits     map[string]*KzipInfo_CorpusInfo_CompilationUnits `protobuf:"bytes,3,rep,name=compilation_units,json=compilationUnits,proto3" json:"compilation_units,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"bytes,2,opt,name=value,proto3"`
	RequiredInputs       map[string]int32                                 `protobuf:"bytes,4,rep,name=required_inputs,json=requiredInputs,proto3" json:"required_inputs,omitempty" protobuf_key:"bytes,1,opt,name=key,proto3" protobuf_val:"varint,2,opt,name=value,proto3"`
	XXX_NoUnkeyedLiteral struct{}                                         `json:"-"`
	XXX_unrecognized     []byte                                           `json:"-"`
	XXX_sizecache        int32                                            `json:"-"`
}

func (m *KzipInfo_CorpusInfo) Reset()         { *m = KzipInfo_CorpusInfo{} }
func (m *KzipInfo_CorpusInfo) String() string { return proto.CompactTextString(m) }
func (*KzipInfo_CorpusInfo) ProtoMessage()    {}
func (*KzipInfo_CorpusInfo) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{4, 0}
}

func (m *KzipInfo_CorpusInfo) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_KzipInfo_CorpusInfo.Unmarshal(m, b)
}
func (m *KzipInfo_CorpusInfo) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_KzipInfo_CorpusInfo.Marshal(b, m, deterministic)
}
func (m *KzipInfo_CorpusInfo) XXX_Merge(src proto.Message) {
	xxx_messageInfo_KzipInfo_CorpusInfo.Merge(m, src)
}
func (m *KzipInfo_CorpusInfo) XXX_Size() int {
	return xxx_messageInfo_KzipInfo_CorpusInfo.Size(m)
}
func (m *KzipInfo_CorpusInfo) XXX_DiscardUnknown() {
	xxx_messageInfo_KzipInfo_CorpusInfo.DiscardUnknown(m)
}

var xxx_messageInfo_KzipInfo_CorpusInfo proto.InternalMessageInfo

func (m *KzipInfo_CorpusInfo) GetCompilationUnits() map[string]*KzipInfo_CorpusInfo_CompilationUnits {
	if m != nil {
		return m.CompilationUnits
	}
	return nil
}

func (m *KzipInfo_CorpusInfo) GetRequiredInputs() map[string]int32 {
	if m != nil {
		return m.RequiredInputs
	}
	return nil
}

type KzipInfo_CorpusInfo_CompilationUnits struct {
	Count                int32    `protobuf:"varint,1,opt,name=count,proto3" json:"count,omitempty"`
	Sources              int32    `protobuf:"varint,2,opt,name=sources,proto3" json:"sources,omitempty"`
	RequiredInputs       int32    `protobuf:"varint,3,opt,name=required_inputs,json=requiredInputs,proto3" json:"required_inputs,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *KzipInfo_CorpusInfo_CompilationUnits) Reset()         { *m = KzipInfo_CorpusInfo_CompilationUnits{} }
func (m *KzipInfo_CorpusInfo_CompilationUnits) String() string { return proto.CompactTextString(m) }
func (*KzipInfo_CorpusInfo_CompilationUnits) ProtoMessage()    {}
func (*KzipInfo_CorpusInfo_CompilationUnits) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{4, 1, 0}
}

func (m *KzipInfo_CorpusInfo_CompilationUnits) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_KzipInfo_CorpusInfo_CompilationUnits.Unmarshal(m, b)
}
func (m *KzipInfo_CorpusInfo_CompilationUnits) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_KzipInfo_CorpusInfo_CompilationUnits.Marshal(b, m, deterministic)
}
func (m *KzipInfo_CorpusInfo_CompilationUnits) XXX_Merge(src proto.Message) {
	xxx_messageInfo_KzipInfo_CorpusInfo_CompilationUnits.Merge(m, src)
}
func (m *KzipInfo_CorpusInfo_CompilationUnits) XXX_Size() int {
	return xxx_messageInfo_KzipInfo_CorpusInfo_CompilationUnits.Size(m)
}
func (m *KzipInfo_CorpusInfo_CompilationUnits) XXX_DiscardUnknown() {
	xxx_messageInfo_KzipInfo_CorpusInfo_CompilationUnits.DiscardUnknown(m)
}

var xxx_messageInfo_KzipInfo_CorpusInfo_CompilationUnits proto.InternalMessageInfo

func (m *KzipInfo_CorpusInfo_CompilationUnits) GetCount() int32 {
	if m != nil {
		return m.Count
	}
	return 0
}

func (m *KzipInfo_CorpusInfo_CompilationUnits) GetSources() int32 {
	if m != nil {
		return m.Sources
	}
	return 0
}

func (m *KzipInfo_CorpusInfo_CompilationUnits) GetRequiredInputs() int32 {
	if m != nil {
		return m.RequiredInputs
	}
	return 0
}

type FilesRequest struct {
	Files                []*FileInfo `protobuf:"bytes,1,rep,name=files,proto3" json:"files,omitempty"`
	XXX_NoUnkeyedLiteral struct{}    `json:"-"`
	XXX_unrecognized     []byte      `json:"-"`
	XXX_sizecache        int32       `json:"-"`
}

func (m *FilesRequest) Reset()         { *m = FilesRequest{} }
func (m *FilesRequest) String() string { return proto.CompactTextString(m) }
func (*FilesRequest) ProtoMessage()    {}
func (*FilesRequest) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{5}
}

func (m *FilesRequest) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_FilesRequest.Unmarshal(m, b)
}
func (m *FilesRequest) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_FilesRequest.Marshal(b, m, deterministic)
}
func (m *FilesRequest) XXX_Merge(src proto.Message) {
	xxx_messageInfo_FilesRequest.Merge(m, src)
}
func (m *FilesRequest) XXX_Size() int {
	return xxx_messageInfo_FilesRequest.Size(m)
}
func (m *FilesRequest) XXX_DiscardUnknown() {
	xxx_messageInfo_FilesRequest.DiscardUnknown(m)
}

var xxx_messageInfo_FilesRequest proto.InternalMessageInfo

func (m *FilesRequest) GetFiles() []*FileInfo {
	if m != nil {
		return m.Files
	}
	return nil
}

type FileInfo struct {
	Path                 string   `protobuf:"bytes,1,opt,name=path,proto3" json:"path,omitempty"`
	Digest               string   `protobuf:"bytes,2,opt,name=digest,proto3" json:"digest,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *FileInfo) Reset()         { *m = FileInfo{} }
func (m *FileInfo) String() string { return proto.CompactTextString(m) }
func (*FileInfo) ProtoMessage()    {}
func (*FileInfo) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{6}
}

func (m *FileInfo) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_FileInfo.Unmarshal(m, b)
}
func (m *FileInfo) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_FileInfo.Marshal(b, m, deterministic)
}
func (m *FileInfo) XXX_Merge(src proto.Message) {
	xxx_messageInfo_FileInfo.Merge(m, src)
}
func (m *FileInfo) XXX_Size() int {
	return xxx_messageInfo_FileInfo.Size(m)
}
func (m *FileInfo) XXX_DiscardUnknown() {
	xxx_messageInfo_FileInfo.DiscardUnknown(m)
}

var xxx_messageInfo_FileInfo proto.InternalMessageInfo

func (m *FileInfo) GetPath() string {
	if m != nil {
		return m.Path
	}
	return ""
}

func (m *FileInfo) GetDigest() string {
	if m != nil {
		return m.Digest
	}
	return ""
}

type FileData struct {
	Content              []byte    `protobuf:"bytes,1,opt,name=content,proto3" json:"content,omitempty"`
	Info                 *FileInfo `protobuf:"bytes,2,opt,name=info,proto3" json:"info,omitempty"`
	Missing              bool      `protobuf:"varint,3,opt,name=missing,proto3" json:"missing,omitempty"`
	XXX_NoUnkeyedLiteral struct{}  `json:"-"`
	XXX_unrecognized     []byte    `json:"-"`
	XXX_sizecache        int32     `json:"-"`
}

func (m *FileData) Reset()         { *m = FileData{} }
func (m *FileData) String() string { return proto.CompactTextString(m) }
func (*FileData) ProtoMessage()    {}
func (*FileData) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{7}
}

func (m *FileData) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_FileData.Unmarshal(m, b)
}
func (m *FileData) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_FileData.Marshal(b, m, deterministic)
}
func (m *FileData) XXX_Merge(src proto.Message) {
	xxx_messageInfo_FileData.Merge(m, src)
}
func (m *FileData) XXX_Size() int {
	return xxx_messageInfo_FileData.Size(m)
}
func (m *FileData) XXX_DiscardUnknown() {
	xxx_messageInfo_FileData.DiscardUnknown(m)
}

var xxx_messageInfo_FileData proto.InternalMessageInfo

func (m *FileData) GetContent() []byte {
	if m != nil {
		return m.Content
	}
	return nil
}

func (m *FileData) GetInfo() *FileInfo {
	if m != nil {
		return m.Info
	}
	return nil
}

func (m *FileData) GetMissing() bool {
	if m != nil {
		return m.Missing
	}
	return false
}

type CompilationBundle struct {
	Unit                 *CompilationUnit `protobuf:"bytes,1,opt,name=unit,proto3" json:"unit,omitempty"`
	Files                []*FileData      `protobuf:"bytes,2,rep,name=files,proto3" json:"files,omitempty"`
	XXX_NoUnkeyedLiteral struct{}         `json:"-"`
	XXX_unrecognized     []byte           `json:"-"`
	XXX_sizecache        int32            `json:"-"`
}

func (m *CompilationBundle) Reset()         { *m = CompilationBundle{} }
func (m *CompilationBundle) String() string { return proto.CompactTextString(m) }
func (*CompilationBundle) ProtoMessage()    {}
func (*CompilationBundle) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{8}
}

func (m *CompilationBundle) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_CompilationBundle.Unmarshal(m, b)
}
func (m *CompilationBundle) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_CompilationBundle.Marshal(b, m, deterministic)
}
func (m *CompilationBundle) XXX_Merge(src proto.Message) {
	xxx_messageInfo_CompilationBundle.Merge(m, src)
}
func (m *CompilationBundle) XXX_Size() int {
	return xxx_messageInfo_CompilationBundle.Size(m)
}
func (m *CompilationBundle) XXX_DiscardUnknown() {
	xxx_messageInfo_CompilationBundle.DiscardUnknown(m)
}

var xxx_messageInfo_CompilationBundle proto.InternalMessageInfo

func (m *CompilationBundle) GetUnit() *CompilationUnit {
	if m != nil {
		return m.Unit
	}
	return nil
}

func (m *CompilationBundle) GetFiles() []*FileData {
	if m != nil {
		return m.Files
	}
	return nil
}

type IndexedCompilation struct {
	Unit                 *CompilationUnit          `protobuf:"bytes,1,opt,name=unit,proto3" json:"unit,omitempty"`
	Index                *IndexedCompilation_Index `protobuf:"bytes,2,opt,name=index,proto3" json:"index,omitempty"`
	XXX_NoUnkeyedLiteral struct{}                  `json:"-"`
	XXX_unrecognized     []byte                    `json:"-"`
	XXX_sizecache        int32                     `json:"-"`
}

func (m *IndexedCompilation) Reset()         { *m = IndexedCompilation{} }
func (m *IndexedCompilation) String() string { return proto.CompactTextString(m) }
func (*IndexedCompilation) ProtoMessage()    {}
func (*IndexedCompilation) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{9}
}

func (m *IndexedCompilation) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_IndexedCompilation.Unmarshal(m, b)
}
func (m *IndexedCompilation) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_IndexedCompilation.Marshal(b, m, deterministic)
}
func (m *IndexedCompilation) XXX_Merge(src proto.Message) {
	xxx_messageInfo_IndexedCompilation.Merge(m, src)
}
func (m *IndexedCompilation) XXX_Size() int {
	return xxx_messageInfo_IndexedCompilation.Size(m)
}
func (m *IndexedCompilation) XXX_DiscardUnknown() {
	xxx_messageInfo_IndexedCompilation.DiscardUnknown(m)
}

var xxx_messageInfo_IndexedCompilation proto.InternalMessageInfo

func (m *IndexedCompilation) GetUnit() *CompilationUnit {
	if m != nil {
		return m.Unit
	}
	return nil
}

func (m *IndexedCompilation) GetIndex() *IndexedCompilation_Index {
	if m != nil {
		return m.Index
	}
	return nil
}

type IndexedCompilation_Index struct {
	Revisions            []string `protobuf:"bytes,1,rep,name=revisions,proto3" json:"revisions,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *IndexedCompilation_Index) Reset()         { *m = IndexedCompilation_Index{} }
func (m *IndexedCompilation_Index) String() string { return proto.CompactTextString(m) }
func (*IndexedCompilation_Index) ProtoMessage()    {}
func (*IndexedCompilation_Index) Descriptor() ([]byte, []int) {
	return fileDescriptor_8e4ea7eca60afe48, []int{9, 0}
}

func (m *IndexedCompilation_Index) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_IndexedCompilation_Index.Unmarshal(m, b)
}
func (m *IndexedCompilation_Index) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_IndexedCompilation_Index.Marshal(b, m, deterministic)
}
func (m *IndexedCompilation_Index) XXX_Merge(src proto.Message) {
	xxx_messageInfo_IndexedCompilation_Index.Merge(m, src)
}
func (m *IndexedCompilation_Index) XXX_Size() int {
	return xxx_messageInfo_IndexedCompilation_Index.Size(m)
}
func (m *IndexedCompilation_Index) XXX_DiscardUnknown() {
	xxx_messageInfo_IndexedCompilation_Index.DiscardUnknown(m)
}

var xxx_messageInfo_IndexedCompilation_Index proto.InternalMessageInfo

func (m *IndexedCompilation_Index) GetRevisions() []string {
	if m != nil {
		return m.Revisions
	}
	return nil
}

func init() {
	proto.RegisterEnum("kythe.proto.AnalysisResult_Status", AnalysisResult_Status_name, AnalysisResult_Status_value)
	proto.RegisterType((*AnalysisRequest)(nil), "kythe.proto.AnalysisRequest")
	proto.RegisterType((*AnalysisOutput)(nil), "kythe.proto.AnalysisOutput")
	proto.RegisterType((*AnalysisResult)(nil), "kythe.proto.AnalysisResult")
	proto.RegisterType((*CompilationUnit)(nil), "kythe.proto.CompilationUnit")
	proto.RegisterType((*CompilationUnit_FileInput)(nil), "kythe.proto.CompilationUnit.FileInput")
	proto.RegisterType((*CompilationUnit_Env)(nil), "kythe.proto.CompilationUnit.Env")
	proto.RegisterType((*KzipInfo)(nil), "kythe.proto.KzipInfo")
	proto.RegisterMapType((map[string]*KzipInfo_CorpusInfo)(nil), "kythe.proto.KzipInfo.CorporaEntry")
	proto.RegisterType((*KzipInfo_CorpusInfo)(nil), "kythe.proto.KzipInfo.CorpusInfo")
	proto.RegisterMapType((map[string]*KzipInfo_CorpusInfo_CompilationUnits)(nil), "kythe.proto.KzipInfo.CorpusInfo.CompilationUnitsEntry")
	proto.RegisterMapType((map[string]int32)(nil), "kythe.proto.KzipInfo.CorpusInfo.RequiredInputsEntry")
	proto.RegisterType((*KzipInfo_CorpusInfo_CompilationUnits)(nil), "kythe.proto.KzipInfo.CorpusInfo.CompilationUnits")
	proto.RegisterType((*FilesRequest)(nil), "kythe.proto.FilesRequest")
	proto.RegisterType((*FileInfo)(nil), "kythe.proto.FileInfo")
	proto.RegisterType((*FileData)(nil), "kythe.proto.FileData")
	proto.RegisterType((*CompilationBundle)(nil), "kythe.proto.CompilationBundle")
	proto.RegisterType((*IndexedCompilation)(nil), "kythe.proto.IndexedCompilation")
	proto.RegisterType((*IndexedCompilation_Index)(nil), "kythe.proto.IndexedCompilation.Index")
}

func init() { proto.RegisterFile("kythe/proto/analysis.proto", fileDescriptor_8e4ea7eca60afe48) }

var fileDescriptor_8e4ea7eca60afe48 = []byte{
	// 1113 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x9c, 0x56, 0xcd, 0x73, 0x1b, 0x35,
	0x14, 0x67, 0x6d, 0xaf, 0x3f, 0x9e, 0xdd, 0xc4, 0x51, 0x5b, 0xd8, 0x2e, 0x65, 0x6a, 0xcc, 0x14,
	0x5a, 0xca, 0x38, 0x10, 0x98, 0x00, 0x0d, 0xd3, 0x99, 0x7c, 0x18, 0xc6, 0x69, 0x9b, 0x82, 0xd2,
	0xf6, 0xc0, 0xc0, 0xec, 0x28, 0x5e, 0xd9, 0xd1, 0x64, 0x2d, 0xb9, 0x2b, 0xad, 0xa9, 0xb9, 0x72,
	0xe6, 0xcc, 0x1f, 0xc0, 0x8d, 0xe1, 0xc8, 0x1f, 0xc7, 0x91, 0x91, 0xb4, 0x6b, 0xef, 0xa6, 0x49,
	0xda, 0x70, 0xf2, 0xbe, 0xa7, 0xf7, 0xf9, 0x7b, 0xbf, 0x27, 0x0b, 0xfc, 0x93, 0xb9, 0x3a, 0xa6,
	0xeb, 0xd3, 0x58, 0x28, 0xb1, 0x4e, 0x38, 0x89, 0xe6, 0x92, 0xc9, 0x9e, 0x11, 0x51, 0xd3, 0x9c,
	0x59, 0xc1, 0xbf, 0x31, 0x16, 0x62, 0x1c, 0xa5, 0x96, 0x47, 0xc9, 0x68, 0x9d, 0xf0, 0x79, 0x76,
	0x94, 0x8f, 0x21, 0x95, 0x88, 0xc9, 0x38, 0xf5, 0xea, 0xfe, 0xe3, 0xc0, 0xea, 0x76, 0x1a, 0x15,
	0xd3, 0x17, 0x09, 0x95, 0x0a, 0x3d, 0x80, 0xe6, 0x50, 0x4c, 0xa6, 0x2c, 0x22, 0x8a, 0x09, 0xee,
	0x39, 0x1d, 0xe7, 0x4e, 0x73, 0xe3, 0x66, 0x2f, 0x97, 0xac, 0xb7, 0xbb, 0x3c, 0x7f, 0xc6, 0x99,
	0xc2, 0x79, 0x07, 0xf4, 0x31, 0xac, 0x8d, 0x58, 0x44, 0x83, 0x90, 0x28, 0x12, 0x48, 0x1a, 0xcf,
	0xd8, 0x90, 0x7a, 0xa5, 0x8e, 0x73, 0xa7, 0x81, 0x57, 0xf5, 0xc1, 0x1e, 0x51, 0xe4, 0xd0, 0xaa,
	0x91, 0x0f, 0xf5, 0x98, 0xce, 0x98, 0xd4, 0x89, 0xca, 0xc6, 0x64, 0x21, 0xa3, 0x1b, 0x50, 0x3f,
	0x16, 0x54, 0x4f, 0x62, 0x16, 0x06, 0x3e, 0x0b, 0xbc, 0x92, 0xb9, 0xab, 0x18, 0x79, 0x10, 0x74,
	0x56, 0xf5, 0x93, 0x44, 0x4d, 0x13, 0x85, 0xae, 0x81, 0x3b, 0x23, 0x51, 0x42, 0x4d, 0xb9, 0x2d,
	0x6c, 0x05, 0xf4, 0x00, 0x5a, 0x23, 0xc6, 0x49, 0x14, 0xc4, 0x54, 0x26, 0x91, 0xf2, 0xc0, 0xf4,
	0x3c, 0x30, 0xb9, 0xbc, 0x97, 0xcb, 0x65, 0x99, 0xbe, 0x86, 0xe0, 0xba, 0x31, 0xb0, 0x42, 0xe7,
	0x44, 0x56, 0x85, 0xee, 0x43, 0x55, 0x2a, 0xa2, 0x12, 0x69, 0x32, 0xad, 0x6c, 0x74, 0x2f, 0x08,
	0xd6, 0x3b, 0x34, 0x96, 0x38, 0xf5, 0x40, 0x1e, 0xd4, 0x64, 0x32, 0x99, 0x90, 0x78, 0x9e, 0xe2,
	0x91, 0x89, 0xdd, 0x2d, 0xa8, 0x5a, 0x5b, 0xd4, 0x82, 0xfa, 0xee, 0x93, 0xc7, 0xdf, 0x3f, 0xea,
	0x3f, 0xed, 0xb7, 0xdf, 0x42, 0x2b, 0x00, 0x83, 0x83, 0x85, 0xec, 0xa0, 0xab, 0xb0, 0x3a, 0x38,
	0x78, 0xbe, 0xfd, 0x68, 0xb0, 0x17, 0xe0, 0xfe, 0x0f, 0xcf, 0xfa, 0x87, 0x4f, 0xdb, 0xa5, 0xee,
	0xdf, 0x2e, 0xac, 0x9e, 0x9a, 0x08, 0xba, 0x0b, 0xd5, 0x59, 0xc0, 0xc9, 0x84, 0xa6, 0xf3, 0x43,
	0x85, 0x32, 0x9f, 0x1f, 0x90, 0x09, 0xc5, 0xee, 0x4c, 0xff, 0xa0, 0xc7, 0xb0, 0x12, 0xd3, 0x17,
	0x09, 0x8b, 0x69, 0x18, 0x30, 0x3e, 0x4d, 0x94, 0x57, 0xee, 0x94, 0xef, 0x34, 0x37, 0x3e, 0xbc,
	0x3e, 0xbc, 0xac, 0xe4, 0xdd, 0x6f, 0x59, 0x48, 0x07, 0x1a, 0x8d, 0xaf, 0xa5, 0xd6, 0x46, 0x44,
	0x74, 0x4c, 0x64, 0x60, 0x19, 0x41, 0x03, 0x1a, 0xc7, 0x22, 0x96, 0x66, 0x80, 0x75, 0xdc, 0x3e,
	0xdc, 0x3c, 0x25, 0xd2, 0x3e, 0x44, 0xfb, 0x46, 0xaf, 0x1b, 0x80, 0x44, 0xe3, 0x78, 0x42, 0xb9,
	0x53, 0xd6, 0x04, 0xc8, 0x64, 0x74, 0x0b, 0x9a, 0x52, 0x24, 0xf1, 0x90, 0x06, 0x9a, 0x36, 0x5e,
	0xd5, 0x1c, 0x83, 0x55, 0xe9, 0xf4, 0xe8, 0x3d, 0x00, 0x61, 0xc6, 0x1f, 0x9c, 0xd0, 0xb9, 0x57,
	0x33, 0x90, 0x36, 0xac, 0xe6, 0x21, 0x9d, 0xa3, 0x7b, 0xb0, 0xf6, 0x8b, 0x88, 0x4f, 0x18, 0x1f,
	0x07, 0x21, 0x8b, 0xe9, 0x50, 0x89, 0x78, 0xee, 0xd5, 0x8d, 0x55, 0x3b, 0x3d, 0xd8, 0xcb, 0xf4,
	0xe8, 0x03, 0xb8, 0x42, 0xb9, 0x8a, 0xe7, 0xc1, 0x50, 0x70, 0x45, 0x5f, 0x2a, 0xaf, 0x61, 0x0c,
	0x5b, 0x46, 0xb9, 0x6b, 0x75, 0x68, 0x07, 0x9a, 0x94, 0xcf, 0x58, 0x2c, 0xb8, 0x29, 0x18, 0x0c,
	0x4e, 0x9d, 0x0b, 0x71, 0xea, 0xf3, 0x19, 0xce, 0x3b, 0xa1, 0x1e, 0xd4, 0x42, 0xaa, 0x08, 0x8b,
	0xa4, 0xd7, 0x34, 0xfe, 0xd7, 0x7a, 0x76, 0x75, 0x7b, 0xd9, 0xea, 0xf6, 0xb6, 0xf9, 0x1c, 0x67,
	0x46, 0xfe, 0x1f, 0x0e, 0x34, 0x16, 0x60, 0x5f, 0x66, 0xae, 0x77, 0xa1, 0xc2, 0xf8, 0x48, 0x18,
	0xaa, 0x35, 0x37, 0xae, 0x17, 0x0c, 0x6d, 0xc0, 0x91, 0xc0, 0xc6, 0x24, 0x5f, 0x53, 0xe5, 0x0d,
	0x6a, 0xda, 0xaf, 0xd4, 0xcb, 0xed, 0x8a, 0xbf, 0x0e, 0xe5, 0x3e, 0x9f, 0x21, 0x04, 0x95, 0x45,
	0x41, 0x0d, 0x6c, 0xbe, 0x97, 0xeb, 0x68, 0x79, 0x6e, 0x85, 0xfd, 0x4a, 0xbd, 0xd4, 0x2e, 0x77,
	0x7f, 0xab, 0x41, 0xfd, 0xe1, 0xaf, 0x6c, 0xaa, 0xf3, 0xa3, 0x6f, 0xa0, 0x36, 0x14, 0xf1, 0x54,
	0xc4, 0xc4, 0x73, 0x4c, 0xe6, 0xe2, 0x3e, 0x65, 0x76, 0xbd, 0x5d, 0x6b, 0xd4, 0xd7, 0xd3, 0xc0,
	0xd5, 0xc0, 0xa9, 0x89, 0xee, 0x10, 0x25, 0x14, 0x09, 0xfd, 0x98, 0x33, 0x25, 0xcd, 0x06, 0x71,
	0x95, 0x06, 0x5f, 0x6a, 0x32, 0x5a, 0x83, 0x1c, 0x91, 0x2c, 0x19, 0x5d, 0xdc, 0x36, 0x27, 0x87,
	0x0b, 0x3a, 0x49, 0xb4, 0x05, 0xbe, 0xb5, 0x2e, 0xee, 0x43, 0xea, 0xe5, 0x1a, 0xaf, 0x77, 0x8c,
	0x05, 0xce, 0x53, 0xde, 0x38, 0xfb, 0x5f, 0x83, 0xbb, 0x2b, 0x12, 0xae, 0x34, 0xa5, 0x23, 0xc2,
	0xc7, 0x09, 0x19, 0x67, 0x98, 0x2c, 0x64, 0x8d, 0xcb, 0x50, 0x1b, 0x19, 0x5c, 0x5c, 0x6c, 0x05,
	0xff, 0xf7, 0x0a, 0x80, 0x6e, 0x30, 0x91, 0x06, 0x93, 0x21, 0xac, 0xe5, 0xee, 0xd3, 0x45, 0x6f,
	0x1a, 0x9d, 0xcd, 0xf3, 0xd1, 0xb1, 0xce, 0xa7, 0xf9, 0x27, 0x2d, 0x62, 0xed, 0xe1, 0x29, 0x35,
	0xfa, 0x19, 0x56, 0x8b, 0x5d, 0x66, 0xa3, 0xff, 0xe2, 0xb5, 0x29, 0x0a, 0xcd, 0xa7, 0x09, 0x56,
	0x0a, 0x97, 0x80, 0xf4, 0x15, 0xb4, 0x4f, 0x57, 0xb2, 0x6c, 0xde, 0xc9, 0x35, 0x8f, 0xde, 0x87,
	0x56, 0x61, 0x38, 0x16, 0x99, 0xa6, 0xcc, 0xcd, 0xe5, 0xa3, 0x57, 0x6b, 0xb5, 0xa3, 0x3e, 0x9d,
	0x75, 0x06, 0xd7, 0xcf, 0xec, 0x1f, 0xb5, 0xa1, 0xac, 0xaf, 0x08, 0x3b, 0x0e, 0xfd, 0x89, 0xbe,
	0xcb, 0x33, 0xb4, 0xb9, 0xf1, 0xd9, 0xa5, 0x81, 0x4d, 0x49, 0x7d, 0xbf, 0xf4, 0x95, 0xe3, 0x6f,
	0xc3, 0xd5, 0x33, 0x40, 0x39, 0x23, 0x6b, 0x61, 0x2f, 0xdc, 0x5c, 0x88, 0xfd, 0x4a, 0xdd, 0x69,
	0x97, 0xec, 0x86, 0xf8, 0x3f, 0x41, 0x2b, 0xcf, 0xf7, 0x33, 0xe2, 0x6c, 0x16, 0xab, 0xef, 0xbc,
	0xae, 0xfa, 0x62, 0x26, 0xbd, 0x85, 0x5b, 0xd0, 0x32, 0xe0, 0x66, 0xff, 0xfa, 0xf7, 0xc0, 0xb5,
	0xf8, 0xdb, 0x35, 0x3c, 0xe7, 0xba, 0xb0, 0x36, 0xdd, 0x4d, 0xa8, 0x67, 0x2a, 0xbd, 0xfe, 0x53,
	0xa2, 0x8e, 0xb3, 0xf5, 0xd7, 0xdf, 0xe8, 0x6d, 0xa8, 0x86, 0x6c, 0x4c, 0xa5, 0x4a, 0xf7, 0x3f,
	0x95, 0xba, 0xcc, 0xfa, 0xe9, 0x17, 0x80, 0xfe, 0x33, 0x34, 0x57, 0x6d, 0xca, 0x87, 0x16, 0xce,
	0xc4, 0xcb, 0x5c, 0x5c, 0x1e, 0xd4, 0x26, 0x4c, 0x4a, 0xc6, 0xc7, 0x86, 0x11, 0x75, 0x9c, 0x89,
	0xdd, 0x18, 0xd6, 0x72, 0x13, 0xdb, 0x49, 0x78, 0x18, 0x51, 0xf4, 0x29, 0x54, 0xf4, 0x36, 0xbd,
	0xd1, 0x9b, 0xc6, 0x58, 0x2e, 0x61, 0x29, 0x9d, 0x03, 0x8b, 0xee, 0x25, 0x83, 0xe5, 0x2f, 0x07,
	0xd0, 0x80, 0x87, 0xf4, 0x25, 0x0d, 0x73, 0xd1, 0xfe, 0x47, 0xd6, 0x2d, 0x70, 0x99, 0x8e, 0x93,
	0x42, 0x70, 0xbb, 0xe0, 0xf2, 0x6a, 0x06, 0xab, 0xc2, 0xd6, 0xc7, 0xbf, 0x0d, 0xae, 0x91, 0xd1,
	0x4d, 0x68, 0x64, 0x8f, 0x29, 0x3b, 0xd6, 0x06, 0x5e, 0x2a, 0x76, 0xbe, 0x84, 0x5b, 0x43, 0x31,
	0xd0, 0x99, 0x12, 0x22, 0x94, 0x59, 0x4f, 0x3f, 0x6e, 0xa4, 0x5f, 0x95, 0xfe, 0x58, 0xf8, 0x46,
	0x63, 0x11, 0x18, 0xd5, 0xbf, 0x8e, 0x73, 0x54, 0x35, 0x5f, 0x9f, 0xff, 0x17, 0x00, 0x00, 0xff,
	0xff, 0xbd, 0xac, 0xab, 0x63, 0x9b, 0x0a, 0x00, 0x00,
}
