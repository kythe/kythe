// Verify that a crash introduced in the indexer by overzealous
// type replacement in DeclaratorDecls isn't reintroduced.

//- @_F defines/binding FileStruct
struct _F;
//- @FILE defines/binding FileAlias
typedef struct _F FILE;
typedef __SIZE_TYPE__ size_t;

//- @fread defines/binding FreadFn
//- FreadFn code FCRoot
//- FCRoot child.0 FCSize
//- FCSize.kind "TYPE"
extern size_t fread (void *__restrict __ptr, size_t __size,
                     //- @FILE ref FileAlias
		     size_t __n, FILE *__restrict __stream);
