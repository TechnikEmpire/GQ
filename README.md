# GumboQuery
CSS Selector Engine for Gumbo Parser

Fork of https://github.com/lazytiger/gumbo-query. I opted to have this be an unofficial fork because I intend on radically altering the library in a way that I don't expect the original repository to pull from.

##Goals  
 - Renaming objects and files and nesting them inside directories to avoid existing conflicts with Gumbo Parser during compilation and inclusion.
 - Wrapping things up in proper namespaces.
 - Reduce allocations of any kind as much as possible.
 - Remove automatic reference counting.  
 - Replace `std::string` with `boost::string_ref` wherever string copies don't truly need to be generated.  
 - Expose internal Gumbo Parser structures and modify API so that construction is possible where things like `CDocument` are non-owning of the raw Gumbo Parser structures.
 - Implement optimizations for trivial selectors.
 - Expose compiled selectors to the public so that they can be retained and recycled against existing and new documents.
 - Comment the hell out of everything. The goal is to have more green than anything else.
