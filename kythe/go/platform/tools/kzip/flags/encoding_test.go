/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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
package flags

import (
	"testing"

	"kythe.io/kythe/go/platform/kzip"
)

func TestJsonEncodingFlag(t *testing.T) {
	var encodingFlag EncodingFlag
	err := encodingFlag.Set("json")

	if err != nil {
		t.Errorf("Unexpected error setting encoding flag value: %v", err)
	}

	flagValue := encodingFlag.Get()
	if flagValue != kzip.EncodingJSON {
		t.Errorf("Value of flag did not match kzip.EncodingJSON(%d): %s", kzip.EncodingJSON, flagValue)
	}

	flagStringValue := encodingFlag.String()
	if flagStringValue != "JSON" {
		t.Errorf("String value of flag did not match \"JSON\": %s", flagStringValue)
	}
}

func TestProtoEncodingFlag(t *testing.T) {
	var encodingFlag EncodingFlag
	err := encodingFlag.Set("proto")

	if err != nil {
		t.Errorf("Unexpected error setting encoding flag value: %v", err)
	}

	flagValue := encodingFlag.Get()
	if flagValue != kzip.EncodingProto {
		t.Errorf("Value of flag did not match kzip.EncodingProto(%d): %s", kzip.EncodingProto, flagValue)
	}

	flagStringValue := encodingFlag.String()
	if flagStringValue != "Proto" {
		t.Errorf("String value of flag did not match \"PROTO\": %s", flagStringValue)
	}
}

func TestInvalidEncodingFlag(t *testing.T) {
	var encodingFlag EncodingFlag
	err := encodingFlag.Set("invalid")

	if err == nil {
		t.Errorf("Expected error but none was returned")
	}
}
