package golang

import (
	"strings"

	"github.com/bazelbuild/bazel-gazelle/config"
	"github.com/bazelbuild/bazel-gazelle/language"
	"github.com/bazelbuild/bazel-gazelle/rule"

	golang "github.com/bazelbuild/bazel-gazelle/language/go"
)

type goLang struct {
	language.Language
}

func NewLanguage() language.Language {
	return &goLang{golang.NewLanguage()}
}

func (l *goLang) GenerateRules(args language.GenerateArgs) language.GenerateResult {
	res := l.Language.GenerateRules(args)
	moveTestEmbedToLibrary(&res)
	suppressBinEmbeds(&res)
	return res
}

func (l *goLang) Fix(c *config.Config, f *rule.File) {
	// Inhibit underlying fixes for now as they include s/library/embed/ which we want to preserve.
}

func (l *goLang) ApparentLoads(moduleToApparentName func(string) string) []rule.LoadInfo {
	loads := l.Language.(language.ModuleAwareLanguage).ApparentLoads(moduleToApparentName)
	for i := range loads {
		if loads[i].Name == "io_bazel_rules_go//go:def.bzl" {
			loads[i].Name = "//tools:build_rules/shims.bzl"
		}
	}
	return loads
}

func moveTestEmbedToLibrary(res *language.GenerateResult) {
	for _, r := range res.Gen {
		if r.Kind() == "go_test" && len(r.AttrStrings("embed")) == 1 {
			r.SetAttr("library", r.AttrStrings("embed")[0])
			r.DelAttr("embed")
		}
	}
}

func suppressBinEmbeds(res *language.GenerateResult) {
	bi, bin := findEmbedBinRule(res.Gen)
	if bin == nil {
		return
	}
	li, lib := findEmbedLibRule(res.Gen, bin.AttrStrings("embed"))
	if lib == nil {
		return
	}

	// Copy the srcs and deps of the embedded library rule, then delete it.
	bin.DelAttr("embed")
	bin.SetAttr("srcs", lib.AttrStrings("srcs"))
	res.Imports[bi] = res.Imports[li]

	// This will inhibit creating the go_library rule if it doesn't already exist,
	// but will not actually delete it unless in `fix` mode.
	lib.Delete()
}

func findEmbedBinRule(gen []*rule.Rule) (int, *rule.Rule) {
	for i, r := range gen {
		if r.Kind() == "go_binary" && len(r.AttrStrings("embed")) > 0 {
			return i, r
		}
	}
	return -1, nil
}

func findEmbedLibRule(gen []*rule.Rule, embed []string) (int, *rule.Rule) {
	for i, r := range gen {
		if r.Kind() == "go_library" {
			for _, name := range embed {
				if r.Name() == strings.TrimPrefix(name, ":") {
					return i, r
				}
			}
		}
	}
	return -1, nil
}
