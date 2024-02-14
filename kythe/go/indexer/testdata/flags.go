package flags

import (
	"flag"
	"fmt"
)

//- vname(_, Corpus, Root, Path, _).node/kind package

var (
	//- @flagVar defines/binding FlagVar
	//- FlagVar.node/kind variable
	//- @"\"flag_name\"" defines/binding Flag=vname("flag flag_name", Corpus, Root, Path, _)
	//- Flag.node/kind google/gflag
	//- Flag generates FlagVar
	flagVar = flag.String("flag_name", "default_value", "Flag description")

	//- FlagDoc documents Flag
	//- FlagDoc.node/kind doc
	//- FlagDoc.text "Flag description"
)

func init() {
	//- @localFlag defines/binding LocalFlag
	//- LocalFlag.node/kind variable
	//- @"\"bool_flag\"" defines/binding BoolFlag=vname("flag bool_flag", _, _, _, _)
	//- BoolFlag.node/kind google/gflag
	//- BoolFlag generates LocalFlag
	localFlag := flag.Bool("bool_flag", false, "Bool flag")

	args := flag.Args()
	fmt.Println(*flagVar, *localFlag, args)
}
