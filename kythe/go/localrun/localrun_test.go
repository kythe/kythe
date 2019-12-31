package localrun

import (
	"testing"
)

func TestAllLanguages(t *testing.T) {
	for l, _ := range AllLanguages() {
		if l.Valid() != true {
			t.Errorf("Provided an invalid language: %v", l)
		}
	}
}
