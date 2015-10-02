/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Package text contains utilities dealing with the encoding of source text.
package text

import (
	"errors"

	"golang.org/x/text/encoding"
	"golang.org/x/text/encoding/htmlindex"
	"golang.org/x/text/transform"
)

// ErrUnsupportedEncoding is returned if an encoding is unsupported.
var ErrUnsupportedEncoding = errors.New("unsupported encoding")

// ToUTF8 converts the given encoded text to a UTF-8 string.
func ToUTF8(encodingName string, b []byte) (string, error) {
	if encodingName == "" {
		return transformBytes(encoding.Replacement.NewEncoder(), b)
	}

	e, err := htmlindex.Get(encodingName)
	if err != nil {
		return "", ErrUnsupportedEncoding
	}
	var t transform.Transformer
	if e == encoding.Replacement {
		t = encoding.Replacement.NewEncoder()
	} else {
		t = e.NewDecoder()
	}
	return transformBytes(t, b)
}

func transformBytes(e transform.Transformer, text []byte) (string, error) {
	res, _, err := transform.Bytes(e, text)
	return string(res), err
}
