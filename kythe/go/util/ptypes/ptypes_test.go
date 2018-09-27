/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ptypes

import (
	"testing"

	"github.com/golang/protobuf/proto"

	anypb "github.com/golang/protobuf/ptypes/any"
)

// A dummy implementation of proto.Message for testing.
type dummy struct {
	S string `protobuf:"bytes,1,opt,name=s"`

	name string
}

func (d *dummy) Reset()                      { d.S = "" }
func (d *dummy) String() string              { return "dummy proto" }
func (d *dummy) XXX_MessageName() string     { return d.name }
func (d *dummy) Marshal() ([]byte, error)    { return []byte("S=" + d.S), nil }
func (d *dummy) Unmarshal(data []byte) error { d.S = string(data); return nil }
func (*dummy) ProtoMessage()                 {}

func TestMarshalAny(t *testing.T) {
	tests := []struct {
		input      proto.Message
		want, data string
	}{
		{&dummy{S: "foo", name: "some.Message"}, "type.googleapis.com/some.Message", "S=foo"},
		{&dummy{S: "bar", name: "kythe.Foo"}, "kythe.io/proto/kythe.Foo", "S=bar"},
	}
	for _, test := range tests {
		msg, err := MarshalAny(test.input)
		if err != nil {
			t.Errorf("MarshalAny %+v failed: %v", test.input, err)
			continue
		}
		if got := msg.TypeUrl; got != test.want {
			t.Errorf("MarshalAny %+v URL: got %q, want %q", test.input, got, test.want)
		}
		if got := string(msg.Value); got != test.data {
			t.Errorf("MarshalAny %+v value: got %q, want %q", test.input, got, test.data)
		}
	}
}

func TestUnmarshalAny(t *testing.T) {
	tests := []struct {
		url, data string
		want      *dummy
	}{
		{"type.googleapis.com/fuzzy", "wuzzy", &dummy{name: "fuzzy", S: "wuzzy"}},
		{"kythe.io/proto/kythe.Blah", "evil dog", &dummy{name: "kythe.Blah", S: "evil dog"}},
	}
	for _, test := range tests {
		got := &dummy{name: test.want.name}
		if err := UnmarshalAny(&anypb.Any{
			TypeUrl: test.url,
			Value:   []byte(test.data),
		}, got); err != nil {
			t.Errorf("Unmarshaling URL %q, data %q: error %v", test.url, test.data, err)
		}
		if *got != *test.want {
			t.Errorf("Unmarshaling failed: got %+v, want %+v", got, test.want)
		}
	}
}
