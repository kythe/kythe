package info

import (
	"testing"

	"github.com/golang/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

func TestMergeKzipInfo(t *testing.T) {
	infos := []*apb.KzipInfo{
		{
			TotalUnits: 1,
			TotalFiles: 2,
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"corpus1": {
					Files: map[string]int32{"python": 2},
					Units: map[string]int32{"python": 1},
				},
			},
		},
		{
			TotalUnits: 1,
			TotalFiles: 2,
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"corpus1": {
					Files: map[string]int32{"go": 2},
					Units: map[string]int32{"python": 1},
				},
			},
		},
	}

	want := &apb.KzipInfo{
		TotalUnits: 2,
		TotalFiles: 4,
		Corpora: map[string]*apb.KzipInfo_CorpusInfo{
			"corpus1": {
				Files: map[string]int32{
					"go":     2,
					"python": 2,
				},
				Units: map[string]int32{"python": 2},
			},
		},
	}

	got := MergeKzipInfo(infos)
	if !proto.Equal(got, want) {
		t.Errorf("got %v, want %v", got, want)
	}
}
