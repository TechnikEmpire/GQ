# GQ
CSS Selector Engine for [Gumbo Parser](https://github.com/google/gumbo-parser)

Fork of https://github.com/lazytiger/gumbo-query. I opted to have this be an unofficial fork because I ~~intend on radically altering~~ have radically altered the library in a way that ~~I don't expect the original repository to pull from~~ is irreconcilable with the original source.

##Usage

You can either construct a GQDocument around an existing GumboOutput pointer, at which point the GQDocument will assume managing the lifetime of the GQDocument, or you can supply a raw string of HTML for GQDocument to parse and also maintain.
```c++
std::string someHtmlString = "...";
std::string someSelectorString = "...";
auto testDocument = gq::GQDocument::Create();
testDocument->Parse(someHtmlString);

try
{
    auto results = testDocument->Find(someSelectorString);
    auto numResults = results.GetNodeCount();
}
catch(std::runtime_error& e)
{
    // Necessary because naturally, the parser can throw.
}
```

As you can see, you can run raw selector strings into the `::Find(...)` method, but each time, the selector string will be "compiled" into a GQSharedSelector and destroyed. You can alternatively "precompile" and save built selectors, and as such avoid wrapping every `::Find(...)` call in a try/catch.

```c++
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

##Speed
One of the primary goals with this engine was to maximize speed. For my purposes, I wanted to ensure I could run an insane amount of selectors without any visible delay to the user. Running the TestParser test benchmarks running every single selector in [EasyList](https://easylist.adblockplus.org/en/) (spare a handful which were removed because they're improperly formatted) against a standard high profile website's landing page HTML. The current results on my [dev laptop](https://www.asus.com/ca-en/ROG-Republic-Of-Gamers/ASUS_ROG_G750JM/) are:

```
Processed 27646 selectors. Had handled errors? false
Benchmarking parsing speed.
Time taken to parse 2764600 selectors: 2550.37 ms.
Processed at a rate of 0.00092251 milliseconds per selector or 1084 selectors per millisecond.
Benchmarking document parsing.
Time to build document: 152.981 milliseconds.
Benchmarking selection speed.
Time taken to run 2764600 selectors against the document: 9756.02 ms producing 42800 total matches.
Processed at a rate of 0.00352891 milliseconds per selector or 283.374 selectors per millisecond.
```

So from these results, a document could be loaded, parsed, and have 27646 precompiled selectors run on it about **436.355** milliseconds, just shy of half a second. Add another 50 msec or so tops to say remove matched nodes and reserialize the document with changes, it's about half a second total. That's stretching the "user doesn't notice" goal, but thankfully nowhere near 30K selectors would never actually need to be run at once.

Speed doesn't mean much if the matching code is broken. As such, over 30 tests currently exist that ensure correct functionality of various types of selectors. I have yet to write tests for nested and combined selectors.

##TODO
 - Mutability API.
 - Tests for combined and nested selectors.
 - Modify `GQSelector::Match()` and related methods to return the final matched node. Required for child selectors and such.

##Original Goals  
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
