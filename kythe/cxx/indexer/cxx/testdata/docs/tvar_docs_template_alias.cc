// We index documentation for type alias templates.

//- @+3"/// Alias." documents Alias
//- @+2"/// Alias." documents Abs

/// Alias.
template<typename U>
using T = int;

//- Alias.node/kind talias
//- Abs.node/kind abs
