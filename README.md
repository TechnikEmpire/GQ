# GQ
CSS Selector Engine for [Gumbo Parser](https://github.com/google/gumbo-parser)

Fork of https://github.com/lazytiger/gumbo-query. I opted to have this be an unofficial fork because I ~~intend on radically altering~~ have radically altered the library in a way that ~~I don't expect the original repository to pull from~~ is irreconcilable with the original source.

##Goals  
 - Renaming objects and files and nesting them inside directories to avoid existing conflicts with Gumbo Parser during compilation and inclusion.
 - Wrapping things up in proper namespaces.
 - Reduce allocations of any kind as much as possible.
 - Remove custom rolled automatic reference counting and replace with standard `shared_ptr` types.  
 - Fix broken parsing that was ported from cascadia, but is invalid for use with Gumbo Parser.
 - Make parsing/matching produce the same behavior as jQuery does on the exact same test data.
 - Replace `std::string` with `boost::string_ref` wherever string copies don't truly need to be generated.  
 - Expose internal Gumbo Parser structures and modify to allow construction of `GQDocument` (formerly `CDocument`) from existing `GumboOutput*`.
 - Implement optimizations for trivial selectors.
 - Remove local state tracking from the selector parser.
 - Expose compiled selectors to the public so that they can be retained and recycled against existing and new documents.
 - "Comments. Lots of comments."

