// The indexer handles redundant typedefs-to-typedefs reasonably.
//- @T defines TypedefIT
typedef int T;
//- @S defines TypedefIS
typedef int S;
//- @S defines TypedefTS
typedef T S;
//- TypedefIT aliases Int
//- TypedefIS aliases Int
//- TypedefTS aliases TypedefIT