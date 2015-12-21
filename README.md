# GQ
CSS Selector Engine for [Gumbo Parser](https://github.com/google/gumbo-parser). Using Gumbo Parser as a backend, GQ can parse input HTML and allow users to select and modify elements in the parsed document with CSS Selectors and the provided simple, but powerful mutation API.

This project is a fork of [gumbo-query](https://github.com/lazytiger/gumbo-query). I opted to have this be an unofficial fork because I intended on performing nearly a complete rewrite, which I did, and as such this source is completely irreconcilable with the original gumbo-query source.

##Usage

You can either construct a GQDocument around an existing GumboOutput pointer, at which point the GQDocument will assume managing the lifetime of the GumboOutput, or you can supply a raw string of HTML for GQDocument to parse and also maintain.
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

These snippets are just meant to demonstrate the most basic of usage. Thanks to the mutation api, it's possible to have fine grain control over elements matched by selectors. Look at the [mutation sample](https://github.com/TechnikEmpire/GQ/blob/master/ide/msvc/GumboQueryExamples/Mutation/Mutation/Mutation.cpp) for a complete example of using this feature.

The contract placed on the end user is very light. Keep GQDocument alive for as long as you're storing or accessing any GQNode object, directly or indirectly. That's basically it.

##Speed
One of the primary goals with this engine was to maximize speed. For my purposes, I wanted to ensure I could run an insane amount of selectors without any visible delay to the user. Running the TestParser test benchmarks parsing and using every single selector in [EasyList](https://easylist.adblockplus.org/en/) (spare a handful which were removed because they're improperly formatted) against a standard high profile website's landing page HTML. The current results on my [dev laptop](https://www.asus.com/ca-en/ROG-Republic-Of-Gamers/ASUS_ROG_G750JM/) are:

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

So from these results, a document could be loaded, parsed, and have 27646 precompiled selectors run on it in about **250.5412** milliseconds, or about a quarter of a second. Based on the latest benchmarks (that actually included mutation during serialization) it's about ***268.019*** msec to load, parse the document, run 27646 selectors and serialize the output with modifications back to a modified html string.
  
That's almost stretching the "user doesn't notice" goal, but thankfully nowhere near 30K selectors would ever actually need to be run at once. In a more realistic use case of the EasyList, less than half that number of selectors would run on any given html input.

Speed doesn't mean much if the matching code is broken. As such, over 40 tests currently exist that ensure correct functionality of various types of selectors. ~~I have yet to write tests for nested and combined selectors.~~

##Configuration  
It can be pretty handy to see verbose output from GQ for debugging selectors, engine issues, and just plain seeing what's going on under the hood. If you build GQ in Debug and add `GQ_VERBOSE_DEBUG_NFO` to your preprocessor definitions, GQ will generate ***lots*** of console output, detailing nearly every single significant event at various stages of the program. Be advised that this will be a lot of text, so piping it to a file or something similar is recommended.

##Limitations  
`shared_ptr` is used at the core of the library, including the mapping system. Since `GQDocument` holds the mapping system for any parsed document, it cannot add itself (the root node of the document) to the map, otherwise a circular reference would be made and nothing would ever clean itself up. As such, the root node of any parsed document is not added to the map, so it's not possible to select against the `<html>` tag itself, only all of its descendants. Switching over to `weak_ptr` isn't possible, since I already tried it. The overhead of the imposed `::lock()` member destroys performance, so for now I've opted to simply omit the `<html>` tag itself from the map. I will consider other solutions in the future. 

##TODO
 - ~~Mutation API.~~
 - ~~Tests for combined and nested selectors.~~
 - Reduce candidate collections BEFORE attempting to match in the event that the selector is a GQBinarySelector with the
 intersection operator. Can reduce sets by only keeping candidates that match the traits from both the left and right
 hand sides of the GQBinarySelector, which would drastically reduce candidates and thus drastically increase matching speed.
 - ~~Modify `GQSelector::Match()` and related methods to return the final matched node. Required for child selectors and such.~~
 - Work around for including root node without having to switch to the abysmal `weak_ptr` in `GQTreeMap`.

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
