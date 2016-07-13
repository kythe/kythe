// We index references to typedefs inside sizeof().
//- @alias defines/binding Alias
typedef float alias;
//- @alias ref Alias
int s = sizeof(alias);
