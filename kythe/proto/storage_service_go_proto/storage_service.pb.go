// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.30.0
// 	protoc        v4.22.2
// source: kythe/proto/storage_service.proto

package storage_service_go_proto

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	storage_go_proto "kythe.io/kythe/proto/storage_go_proto"
	reflect "reflect"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

var File_kythe_proto_storage_service_proto protoreflect.FileDescriptor

var file_kythe_proto_storage_service_proto_rawDesc = []byte{
	0x0a, 0x21, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x73, 0x74,
	0x6f, 0x72, 0x61, 0x67, 0x65, 0x5f, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x12, 0x0b, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x1a, 0x19, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x73, 0x74,
	0x6f, 0x72, 0x61, 0x67, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x32, 0xbf, 0x01, 0x0a, 0x0a,
	0x47, 0x72, 0x61, 0x70, 0x68, 0x53, 0x74, 0x6f, 0x72, 0x65, 0x12, 0x38, 0x0a, 0x04, 0x52, 0x65,
	0x61, 0x64, 0x12, 0x18, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x2e, 0x52, 0x65, 0x61, 0x64, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x12, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x45, 0x6e, 0x74, 0x72, 0x79,
	0x22, 0x00, 0x30, 0x01, 0x12, 0x38, 0x0a, 0x04, 0x53, 0x63, 0x61, 0x6e, 0x12, 0x18, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x53, 0x63, 0x61, 0x6e, 0x52,
	0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x12, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70,
	0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x22, 0x00, 0x30, 0x01, 0x12, 0x3d,
	0x0a, 0x05, 0x57, 0x72, 0x69, 0x74, 0x65, 0x12, 0x19, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x57, 0x72, 0x69, 0x74, 0x65, 0x52, 0x65, 0x71, 0x75, 0x65,
	0x73, 0x74, 0x1a, 0x17, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x2e, 0x57, 0x72, 0x69, 0x74, 0x65, 0x52, 0x65, 0x70, 0x6c, 0x79, 0x22, 0x00, 0x32, 0x8e, 0x01,
	0x0a, 0x11, 0x53, 0x68, 0x61, 0x72, 0x64, 0x65, 0x64, 0x47, 0x72, 0x61, 0x70, 0x68, 0x53, 0x74,
	0x6f, 0x72, 0x65, 0x12, 0x3d, 0x0a, 0x05, 0x43, 0x6f, 0x75, 0x6e, 0x74, 0x12, 0x19, 0x2e, 0x6b,
	0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x6f, 0x75, 0x6e, 0x74,
	0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x17, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x43, 0x6f, 0x75, 0x6e, 0x74, 0x52, 0x65, 0x70, 0x6c, 0x79,
	0x22, 0x00, 0x12, 0x3a, 0x0a, 0x05, 0x53, 0x68, 0x61, 0x72, 0x64, 0x12, 0x19, 0x2e, 0x6b, 0x79,
	0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x53, 0x68, 0x61, 0x72, 0x64, 0x52,
	0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x12, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70,
	0x72, 0x6f, 0x74, 0x6f, 0x2e, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x22, 0x00, 0x30, 0x01, 0x42, 0x50,
	0x0a, 0x1f, 0x63, 0x6f, 0x6d, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x76,
	0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2e, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x5a, 0x2d, 0x6b, 0x79, 0x74, 0x68, 0x65, 0x2e, 0x69, 0x6f, 0x2f, 0x6b, 0x79, 0x74, 0x68,
	0x65, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x5f,
	0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x5f, 0x67, 0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var file_kythe_proto_storage_service_proto_goTypes = []interface{}{
	(*storage_go_proto.ReadRequest)(nil),  // 0: kythe.proto.ReadRequest
	(*storage_go_proto.ScanRequest)(nil),  // 1: kythe.proto.ScanRequest
	(*storage_go_proto.WriteRequest)(nil), // 2: kythe.proto.WriteRequest
	(*storage_go_proto.CountRequest)(nil), // 3: kythe.proto.CountRequest
	(*storage_go_proto.ShardRequest)(nil), // 4: kythe.proto.ShardRequest
	(*storage_go_proto.Entry)(nil),        // 5: kythe.proto.Entry
	(*storage_go_proto.WriteReply)(nil),   // 6: kythe.proto.WriteReply
	(*storage_go_proto.CountReply)(nil),   // 7: kythe.proto.CountReply
}
var file_kythe_proto_storage_service_proto_depIdxs = []int32{
	0, // 0: kythe.proto.GraphStore.Read:input_type -> kythe.proto.ReadRequest
	1, // 1: kythe.proto.GraphStore.Scan:input_type -> kythe.proto.ScanRequest
	2, // 2: kythe.proto.GraphStore.Write:input_type -> kythe.proto.WriteRequest
	3, // 3: kythe.proto.ShardedGraphStore.Count:input_type -> kythe.proto.CountRequest
	4, // 4: kythe.proto.ShardedGraphStore.Shard:input_type -> kythe.proto.ShardRequest
	5, // 5: kythe.proto.GraphStore.Read:output_type -> kythe.proto.Entry
	5, // 6: kythe.proto.GraphStore.Scan:output_type -> kythe.proto.Entry
	6, // 7: kythe.proto.GraphStore.Write:output_type -> kythe.proto.WriteReply
	7, // 8: kythe.proto.ShardedGraphStore.Count:output_type -> kythe.proto.CountReply
	5, // 9: kythe.proto.ShardedGraphStore.Shard:output_type -> kythe.proto.Entry
	5, // [5:10] is the sub-list for method output_type
	0, // [0:5] is the sub-list for method input_type
	0, // [0:0] is the sub-list for extension type_name
	0, // [0:0] is the sub-list for extension extendee
	0, // [0:0] is the sub-list for field type_name
}

func init() { file_kythe_proto_storage_service_proto_init() }
func file_kythe_proto_storage_service_proto_init() {
	if File_kythe_proto_storage_service_proto != nil {
		return
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_kythe_proto_storage_service_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   0,
			NumExtensions: 0,
			NumServices:   2,
		},
		GoTypes:           file_kythe_proto_storage_service_proto_goTypes,
		DependencyIndexes: file_kythe_proto_storage_service_proto_depIdxs,
	}.Build()
	File_kythe_proto_storage_service_proto = out.File
	file_kythe_proto_storage_service_proto_rawDesc = nil
	file_kythe_proto_storage_service_proto_goTypes = nil
	file_kythe_proto_storage_service_proto_depIdxs = nil
}
