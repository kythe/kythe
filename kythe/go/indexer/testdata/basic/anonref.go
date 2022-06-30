package anonref

import (
	anonymous "kythe/go/indexer/anonymous_test"
)

// - @Struct ref Struct
// - @V ref V
var _ = anonymous.Struct.V
