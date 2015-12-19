# GQ
CSS Selector Engine for [Gumbo Parser](https://github.com/google/gumbo-parser)

Fork of https://github.com/lazytiger/gumbo-query. I opted to have this be an unofficial fork because I ~~intend on radically altering~~ have radically altered the library in a way that ~~I don't expect the original repository to pull from~~ is irreconcilable with the original source.

##Usage

You can either construct a GQDocument around an existing GumboOutput pointer, at which point the GQDocument will assume managing the lifetime of the GQDocument, or you can supply a raw string of HTML for GQDocument to parse and also maintain.
```
std::string someHtmlString = "...";
std::string someSelectorString = "...";
auto testDocument = gq::GQDocument::Create();
testDocument->Parse(someHtmlString);

try
{
    auto results = testDocument->Find(someSelectorString);
}
catch(std::runtime_error& e)
{
    // Necessary because naturally, the parser can throw.
}
auto numResults = results.GetNodeCount();
```

As you can see, you can run raw selector strings into the `::Find(...)` method, but each time, the selector string will be "compiled" into a GQSharedSelector and destroyed. You can alternatively "precompile" and save built selectors, and as such avoid wrapping every `::Find(...)` call in a try/catch.

```
GumboOutput* output = SOMETHING_NOT_NULL;
auto testDocument = gq::GQDocument::Create(output);

gq::GQParser parser;

std::vector<std::string> collectionOfRawSelectoStrings {...};
std::vector<gq::SharedGQSelector> compiledSelectors();
compiledSelectors.reserve(collectionOfRawSelectoStrings.size());

for(auto& s : collectionOfRawSelectoStrings)
{
    try
    {
        auto result = parser.CreateSelector(s);
        compiledSelectors.push_back(result);
    }
    catch(std::runtime_error& e)
    {
        // Necessary because naturally, the parser can throw.
    }
}

size_t numResults = 0;
for(auto& ss : compiledSelectors)
{
    auto results = testDocument->Find(ss);
    numResults += results.GetNodeCount();
}
```

##Goals  
 - Renaming objects and files and nesting them inside directories to avoid existing conflicts with Gumbo Parser during compilation and inclusion.
 - Wrapping things up in proper namespaces.
 - Remove custom rolled automatic reference counting and replace with standard `shared_ptr` types.  
 - Fix broken parsing that was ported from cascadia, but is invalid for use with Gumbo Parser.
 - Make parsing/matching produce the same behavior as jQuery does on the exact same test data.
 - Replace `std::string` with `boost::string_ref` wherever string copies don't truly need to be generated.  
 - Implement a mapping system to dramatically increase matching speed by filtering potential matches by traits.
 - Remove local state tracking from the selector parser.
 - Expose compiled selectors to the public so that they can be retained and recycled against existing and new documents.
 - "Comments. Lots of comments."
 - "Speed. Lots of Speed."
