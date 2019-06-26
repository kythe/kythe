/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Package flags provides type EncodingFlag for use as a flag to specify Encoding.
package flags

import (
	"fmt"
	"strings"

	"kythe.io/kythe/go/platform/kzip"
)

// EncodingFlag encapsulates an Encoding for use as a flag.
type EncodingFlag struct {
	kzip.Encoding
}

// String implements part of the flag.Value interface.
func (e *EncodingFlag) String() string {
	return e.Encoding.String()
}

// Get implements part of the flag.Getter interface.
func (e *EncodingFlag) Get() interface{} {
	return e.Encoding
}

// Set implements part of the flag.Value interface.
func (e *EncodingFlag) Set(v string) error {
	v = strings.ToUpper(v)
	switch {
	case v == "ALL":
		*e = EncodingFlag{kzip.EncodingAll}
		return nil
	case v == "JSON":
		*e = EncodingFlag{kzip.EncodingJSON}
		return nil
	case v == "PROTO":
		*e = EncodingFlag{kzip.EncodingProto}
		return nil
	default:
		return fmt.Errorf("Unknown encoding %s", e)
	}
}
