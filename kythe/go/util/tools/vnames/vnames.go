// Binary vnames is a utility for handling VNames.
//
// Examples:
//   # Convert VName rewrite rules from json to proto.
//   vnames convert-rules --from json --to proto < rules.json > rules.pb.bin
//
//   # Apply VName rewrite rules to stdin.
//   print -l path1 path2 path3 | vnames apply-rules --rules rules.json
package main

import (
	"context"
	"flag"
	"os"

	"github.com/google/subcommands"
)

func main() {
	flag.Parse()

	subcommands.Register(&applyRulesCmd{Info: applyRulesInfo}, "rules")
	subcommands.Register(&convertRulesCmd{Info: convertRulesInfo}, "rules")

	subcommands.Register(subcommands.FlagsCommand(), "info")
	subcommands.Register(subcommands.HelpCommand(), "info")

	ctx := context.Background()
	os.Exit(int(subcommands.Execute(ctx)))
}
