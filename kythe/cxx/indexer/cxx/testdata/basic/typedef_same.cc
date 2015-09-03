// The indexer handles redundant typedefs-to-typedefs reasonably.
//- @T defines/binding TypedefIT
typedef int T;
//- @S defines/binding TypedefIS
typedef int S;
//- @S defines/binding TypedefTS
typedef T S;
//- TypedefIT aliases Int
//- TypedefIS aliases Int
//- TypedefTS aliases TypedefIT