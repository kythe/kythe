/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package delimited

import (
	"fmt"
	"io"
)

// A Source represents a sequence of records.
type Source interface {
	// Next returns the next record in the sequence, or io.EOF when no further
	// records are available. The slice returned by Next is only required to be
	// valid until a subsequent call to Next.
	Next() ([]byte, error)
}

// A Sink represents a receiver of records.
type Sink interface {
	// Put delivers a record to the sink.
	Put([]byte) error
}

// Copy sequentially copies each record read from src to sink until src.Next()
// returns io.EOF or another error occurs.
func Copy(sink Sink, src Source) error {
	for {
		record, err := src.Next()
		if err == io.EOF {
			return nil
		} else if err != nil {
			return fmt.Errorf("copy: read error: %v", err)
		}
		if err := sink.Put(record); err != nil {
			return fmt.Errorf("copy: write error: %v", err)
		}
	}
}
