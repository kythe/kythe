// This file is generated by rust-protobuf 2.27.1. Do not edit
// @generated

// https://github.com/rust-lang/rust-clippy/issues/702
#![allow(unknown_lints)]
#![allow(clippy::all)]

#![allow(unused_attributes)]
#![cfg_attr(rustfmt, rustfmt::skip)]

#![allow(box_pointers)]
#![allow(dead_code)]
#![allow(missing_docs)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(trivial_casts)]
#![allow(unused_imports)]
#![allow(unused_results)]
//! Generated file from `google/protobuf/timestamp.proto`

/// Generated files are compatible only with the same version
/// of protobuf runtime.
// const _PROTOBUF_VERSION_CHECK: () = ::protobuf::VERSION_2_27_1;

#[derive(PartialEq,Clone,Default)]
pub struct Timestamp {
    // message fields
    pub seconds: i64,
    pub nanos: i32,
    // special fields
    pub unknown_fields: ::protobuf::UnknownFields,
    pub cached_size: ::protobuf::CachedSize,
}

impl<'a> ::std::default::Default for &'a Timestamp {
    fn default() -> &'a Timestamp {
        <Timestamp as ::protobuf::Message>::default_instance()
    }
}

impl Timestamp {
    pub fn new() -> Timestamp {
        ::std::default::Default::default()
    }

    // int64 seconds = 1;


    pub fn get_seconds(&self) -> i64 {
        self.seconds
    }
    pub fn clear_seconds(&mut self) {
        self.seconds = 0;
    }

    // Param is passed by value, moved
    pub fn set_seconds(&mut self, v: i64) {
        self.seconds = v;
    }

    // int32 nanos = 2;


    pub fn get_nanos(&self) -> i32 {
        self.nanos
    }
    pub fn clear_nanos(&mut self) {
        self.nanos = 0;
    }

    // Param is passed by value, moved
    pub fn set_nanos(&mut self, v: i32) {
        self.nanos = v;
    }
}

impl ::protobuf::Message for Timestamp {
    fn is_initialized(&self) -> bool {
        true
    }

    fn merge_from(&mut self, is: &mut ::protobuf::CodedInputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        while !is.eof()? {
            let (field_number, wire_type) = is.read_tag_unpack()?;
            match field_number {
                1 => {
                    if wire_type != ::protobuf::wire_format::WireTypeVarint {
                        return ::std::result::Result::Err(::protobuf::rt::unexpected_wire_type(wire_type));
                    }
                    let tmp = is.read_int64()?;
                    self.seconds = tmp;
                },
                2 => {
                    if wire_type != ::protobuf::wire_format::WireTypeVarint {
                        return ::std::result::Result::Err(::protobuf::rt::unexpected_wire_type(wire_type));
                    }
                    let tmp = is.read_int32()?;
                    self.nanos = tmp;
                },
                _ => {
                    ::protobuf::rt::read_unknown_or_skip_group(field_number, wire_type, is, self.mut_unknown_fields())?;
                },
            };
        }
        ::std::result::Result::Ok(())
    }

    // Compute sizes of nested messages
    #[allow(unused_variables)]
    fn compute_size(&self) -> u32 {
        let mut my_size = 0;
        if self.seconds != 0 {
            my_size += ::protobuf::rt::value_size(1, self.seconds, ::protobuf::wire_format::WireTypeVarint);
        }
        if self.nanos != 0 {
            my_size += ::protobuf::rt::value_size(2, self.nanos, ::protobuf::wire_format::WireTypeVarint);
        }
        my_size += ::protobuf::rt::unknown_fields_size(self.get_unknown_fields());
        self.cached_size.set(my_size);
        my_size
    }

    fn write_to_with_cached_sizes(&self, os: &mut ::protobuf::CodedOutputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        if self.seconds != 0 {
            os.write_int64(1, self.seconds)?;
        }
        if self.nanos != 0 {
            os.write_int32(2, self.nanos)?;
        }
        os.write_unknown_fields(self.get_unknown_fields())?;
        ::std::result::Result::Ok(())
    }

    fn get_cached_size(&self) -> u32 {
        self.cached_size.get()
    }

    fn get_unknown_fields(&self) -> &::protobuf::UnknownFields {
        &self.unknown_fields
    }

    fn mut_unknown_fields(&mut self) -> &mut ::protobuf::UnknownFields {
        &mut self.unknown_fields
    }

    fn as_any(&self) -> &dyn (::std::any::Any) {
        self as &dyn (::std::any::Any)
    }
    fn as_any_mut(&mut self) -> &mut dyn (::std::any::Any) {
        self as &mut dyn (::std::any::Any)
    }
    fn into_any(self: ::std::boxed::Box<Self>) -> ::std::boxed::Box<dyn (::std::any::Any)> {
        self
    }

    fn descriptor(&self) -> &'static ::protobuf::reflect::MessageDescriptor {
        Self::descriptor_static()
    }

    fn new() -> Timestamp {
        Timestamp::new()
    }

    fn descriptor_static() -> &'static ::protobuf::reflect::MessageDescriptor {
        static descriptor: ::protobuf::rt::LazyV2<::protobuf::reflect::MessageDescriptor> = ::protobuf::rt::LazyV2::INIT;
        descriptor.get(|| {
            let mut fields = ::std::vec::Vec::new();
            fields.push(::protobuf::reflect::accessor::make_simple_field_accessor::<_, ::protobuf::types::ProtobufTypeInt64>(
                "seconds",
                |m: &Timestamp| { &m.seconds },
                |m: &mut Timestamp| { &mut m.seconds },
            ));
            fields.push(::protobuf::reflect::accessor::make_simple_field_accessor::<_, ::protobuf::types::ProtobufTypeInt32>(
                "nanos",
                |m: &Timestamp| { &m.nanos },
                |m: &mut Timestamp| { &mut m.nanos },
            ));
            ::protobuf::reflect::MessageDescriptor::new_pb_name::<Timestamp>(
                "Timestamp",
                fields,
                file_descriptor_proto()
            )
        })
    }

    fn default_instance() -> &'static Timestamp {
        static instance: ::protobuf::rt::LazyV2<Timestamp> = ::protobuf::rt::LazyV2::INIT;
        instance.get(Timestamp::new)
    }
}

impl ::protobuf::Clear for Timestamp {
    fn clear(&mut self) {
        self.seconds = 0;
        self.nanos = 0;
        self.unknown_fields.clear();
    }
}

impl ::std::fmt::Debug for Timestamp {
    fn fmt(&self, f: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
        ::protobuf::text_format::fmt(self, f)
    }
}

impl ::protobuf::reflect::ProtobufValue for Timestamp {
    fn as_ref(&self) -> ::protobuf::reflect::ReflectValueRef {
        ::protobuf::reflect::ReflectValueRef::Message(self)
    }
}

static file_descriptor_proto_data: &'static [u8] = b"\
    \n\x1fgoogle/protobuf/timestamp.proto\x12\x0fgoogle.protobuf\";\n\tTimes\
    tamp\x12\x18\n\x07seconds\x18\x01\x20\x01(\x03R\x07seconds\x12\x14\n\x05\
    nanos\x18\x02\x20\x01(\x05R\x05nanosB\x85\x01\n\x13com.google.protobufB\
    \x0eTimestampProtoP\x01Z2google.golang.org/protobuf/types/known/timestam\
    ppb\xf8\x01\x01\xa2\x02\x03GPB\xaa\x02\x1eGoogle.Protobuf.WellKnownTypes\
    b\x06proto3\
";

static file_descriptor_proto_lazy: ::protobuf::rt::LazyV2<::protobuf::descriptor::FileDescriptorProto> = ::protobuf::rt::LazyV2::INIT;

fn parse_descriptor_proto() -> ::protobuf::descriptor::FileDescriptorProto {
    ::protobuf::Message::parse_from_bytes(file_descriptor_proto_data).unwrap()
}

pub fn file_descriptor_proto() -> &'static ::protobuf::descriptor::FileDescriptorProto {
    file_descriptor_proto_lazy.get(|| {
        parse_descriptor_proto()
    })
}
