// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.30.0
// 	protoc        v4.22.2
// source: kythe/proto/java.proto

package java_go_proto

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

type JarDetails struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Jar []*JarDetails_Jar `protobuf:"bytes,1,rep,name=jar,proto3" json:"jar,omitempty"`
}

func (x *JarDetails) Reset() {
	*x = JarDetails{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_java_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *JarDetails) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*JarDetails) ProtoMessage() {}

func (x *JarDetails) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_java_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use JarDetails.ProtoReflect.Descriptor instead.
func (*JarDetails) Descriptor() ([]byte, []int) {
	return file_kythe_proto_java_proto_rawDescGZIP(), []int{0}
}

func (x *JarDetails) GetJar() []*JarDetails_Jar {
	if x != nil {
		return x.Jar
	}
	return nil
}

type JarEntryDetails struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	JarContainer int32 `protobuf:"varint,1,opt,name=jar_container,json=jarContainer,proto3" json:"jar_container,omitempty"`
}

func (x *JarEntryDetails) Reset() {
	*x = JarEntryDetails{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_java_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *JarEntryDetails) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*JarEntryDetails) ProtoMessage() {}

func (x *JarEntryDetails) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_java_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use JarEntryDetails.ProtoReflect.Descriptor instead.
func (*JarEntryDetails) Descriptor() ([]byte, []int) {
	return file_kythe_proto_java_proto_rawDescGZIP(), []int{1}
}

func (x *JarEntryDetails) GetJarContainer() int32 {
	if x != nil {
		return x.JarContainer
	}
	return 0
}

type JavaDetails struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Classpath      []string `protobuf:"bytes,1,rep,name=classpath,proto3" json:"classpath,omitempty"`
	Sourcepath     []string `protobuf:"bytes,2,rep,name=sourcepath,proto3" json:"sourcepath,omitempty"`
	Bootclasspath  []string `protobuf:"bytes,3,rep,name=bootclasspath,proto3" json:"bootclasspath,omitempty"`
	ExtraJavacopts []string `protobuf:"bytes,10,rep,name=extra_javacopts,json=extraJavacopts,proto3" json:"extra_javacopts,omitempty"`
}

func (x *JavaDetails) Reset() {
	*x = JavaDetails{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_java_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *JavaDetails) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*JavaDetails) ProtoMessage() {}

func (x *JavaDetails) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_java_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use JavaDetails.ProtoReflect.Descriptor instead.
func (*JavaDetails) Descriptor() ([]byte, []int) {
	return file_kythe_proto_java_proto_rawDescGZIP(), []int{2}
}

func (x *JavaDetails) GetClasspath() []string {
	if x != nil {
		return x.Classpath
	}
	return nil
}

func (x *JavaDetails) GetSourcepath() []string {
	if x != nil {
		return x.Sourcepath
	}
	return nil
}

func (x *JavaDetails) GetBootclasspath() []string {
	if x != nil {
		return x.Bootclasspath
	}
	return nil
}

func (x *JavaDetails) GetExtraJavacopts() []string {
	if x != nil {
		return x.ExtraJavacopts
	}
	return nil
}

type JarDetails_Jar struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	VName *storage_go_proto.VName `protobuf:"bytes,1,opt,name=v_name,json=vName,proto3" json:"v_name,omitempty"`
}

func (x *JarDetails_Jar) Reset() {
	*x = JarDetails_Jar{}
	if protoimpl.UnsafeEnabled {
		mi := &file_kythe_proto_java_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *JarDetails_Jar) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*JarDetails_Jar) ProtoMessage() {}

func (x *JarDetails_Jar) ProtoReflect() protoreflect.Message {
	mi := &file_kythe_proto_java_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use JarDetails_Jar.ProtoReflect.Descriptor instead.
func (*JarDetails_Jar) Descriptor() ([]byte, []int) {
	return file_kythe_proto_java_proto_rawDescGZIP(), []int{0, 0}
}

func (x *JarDetails_Jar) GetVName() *storage_go_proto.VName {
	if x != nil {
		return x.VName
	}
	return nil
}

var File_kythe_proto_java_proto protoreflect.FileDescriptor

var file_kythe_proto_java_proto_rawDesc = []byte{
	0x0a, 0x16, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x6a, 0x61,
	0x76, 0x61, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0b, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x1a, 0x19, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x2f, 0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x22, 0x73, 0x0a, 0x0a, 0x4a, 0x61, 0x72, 0x44, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x12, 0x2d,
	0x0a, 0x03, 0x6a, 0x61, 0x72, 0x18, 0x01, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x1b, 0x2e, 0x6b, 0x79,
	0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x4a, 0x61, 0x72, 0x44, 0x65, 0x74,
	0x61, 0x69, 0x6c, 0x73, 0x2e, 0x4a, 0x61, 0x72, 0x52, 0x03, 0x6a, 0x61, 0x72, 0x1a, 0x36, 0x0a,
	0x03, 0x4a, 0x61, 0x72, 0x12, 0x29, 0x0a, 0x06, 0x76, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x0b, 0x32, 0x12, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x2e, 0x56, 0x4e, 0x61, 0x6d, 0x65, 0x52, 0x05, 0x76, 0x4e, 0x61, 0x6d, 0x65, 0x4a,
	0x04, 0x08, 0x02, 0x10, 0x03, 0x22, 0x36, 0x0a, 0x0f, 0x4a, 0x61, 0x72, 0x45, 0x6e, 0x74, 0x72,
	0x79, 0x44, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x12, 0x23, 0x0a, 0x0d, 0x6a, 0x61, 0x72, 0x5f,
	0x63, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x18, 0x01, 0x20, 0x01, 0x28, 0x05, 0x52,
	0x0c, 0x6a, 0x61, 0x72, 0x43, 0x6f, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x22, 0x9a, 0x01,
	0x0a, 0x0b, 0x4a, 0x61, 0x76, 0x61, 0x44, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x12, 0x1c, 0x0a,
	0x09, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x70, 0x61, 0x74, 0x68, 0x18, 0x01, 0x20, 0x03, 0x28, 0x09,
	0x52, 0x09, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x70, 0x61, 0x74, 0x68, 0x12, 0x1e, 0x0a, 0x0a, 0x73,
	0x6f, 0x75, 0x72, 0x63, 0x65, 0x70, 0x61, 0x74, 0x68, 0x18, 0x02, 0x20, 0x03, 0x28, 0x09, 0x52,
	0x0a, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x70, 0x61, 0x74, 0x68, 0x12, 0x24, 0x0a, 0x0d, 0x62,
	0x6f, 0x6f, 0x74, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x70, 0x61, 0x74, 0x68, 0x18, 0x03, 0x20, 0x03,
	0x28, 0x09, 0x52, 0x0d, 0x62, 0x6f, 0x6f, 0x74, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x70, 0x61, 0x74,
	0x68, 0x12, 0x27, 0x0a, 0x0f, 0x65, 0x78, 0x74, 0x72, 0x61, 0x5f, 0x6a, 0x61, 0x76, 0x61, 0x63,
	0x6f, 0x70, 0x74, 0x73, 0x18, 0x0a, 0x20, 0x03, 0x28, 0x09, 0x52, 0x0e, 0x65, 0x78, 0x74, 0x72,
	0x61, 0x4a, 0x61, 0x76, 0x61, 0x63, 0x6f, 0x70, 0x74, 0x73, 0x42, 0x30, 0x0a, 0x1f, 0x63, 0x6f,
	0x6d, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x76, 0x74, 0x6f, 0x6f, 0x6c,
	0x73, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x5a, 0x0d, 0x6a,
	0x61, 0x76, 0x61, 0x5f, 0x67, 0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_kythe_proto_java_proto_rawDescOnce sync.Once
	file_kythe_proto_java_proto_rawDescData = file_kythe_proto_java_proto_rawDesc
)

func file_kythe_proto_java_proto_rawDescGZIP() []byte {
	file_kythe_proto_java_proto_rawDescOnce.Do(func() {
		file_kythe_proto_java_proto_rawDescData = protoimpl.X.CompressGZIP(file_kythe_proto_java_proto_rawDescData)
	})
	return file_kythe_proto_java_proto_rawDescData
}

var file_kythe_proto_java_proto_msgTypes = make([]protoimpl.MessageInfo, 4)
var file_kythe_proto_java_proto_goTypes = []interface{}{
	(*JarDetails)(nil),             // 0: kythe.proto.JarDetails
	(*JarEntryDetails)(nil),        // 1: kythe.proto.JarEntryDetails
	(*JavaDetails)(nil),            // 2: kythe.proto.JavaDetails
	(*JarDetails_Jar)(nil),         // 3: kythe.proto.JarDetails.Jar
	(*storage_go_proto.VName)(nil), // 4: kythe.proto.VName
}
var file_kythe_proto_java_proto_depIdxs = []int32{
	3, // 0: kythe.proto.JarDetails.jar:type_name -> kythe.proto.JarDetails.Jar
	4, // 1: kythe.proto.JarDetails.Jar.v_name:type_name -> kythe.proto.VName
	2, // [2:2] is the sub-list for method output_type
	2, // [2:2] is the sub-list for method input_type
	2, // [2:2] is the sub-list for extension type_name
	2, // [2:2] is the sub-list for extension extendee
	0, // [0:2] is the sub-list for field type_name
}

func init() { file_kythe_proto_java_proto_init() }
func file_kythe_proto_java_proto_init() {
	if File_kythe_proto_java_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_kythe_proto_java_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*JarDetails); i {
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
		file_kythe_proto_java_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*JarEntryDetails); i {
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
		file_kythe_proto_java_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*JavaDetails); i {
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
		file_kythe_proto_java_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*JarDetails_Jar); i {
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
			RawDescriptor: file_kythe_proto_java_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   4,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_kythe_proto_java_proto_goTypes,
		DependencyIndexes: file_kythe_proto_java_proto_depIdxs,
		MessageInfos:      file_kythe_proto_java_proto_msgTypes,
	}.Build()
	File_kythe_proto_java_proto = out.File
	file_kythe_proto_java_proto_rawDesc = nil
	file_kythe_proto_java_proto_goTypes = nil
	file_kythe_proto_java_proto_depIdxs = nil
}
