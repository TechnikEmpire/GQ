# GQ
GQ is a CSS Selector Engine for [Gumbo Parser](https://github.com/google/gumbo-parser) written in C++11. Using Gumbo Parser as a backend, GQ can parse input HTML and allow users to select and modify elements in the parsed document with CSS Selectors and the provided simple, but powerful mutation API.

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

std::vector<std::string> collectionOfRawSelectorStrings {...};
std::vector<gq::SharedGQSelector> compiledSelectors();
compiledSelectors.reserve(collectionOfRawSelectorStrings.size());

for(auto& s : collectionOfRawSelectorStrings)
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
Time taken to parse 2764600 selectors: 2492.68 ms.
Processed at a rate of 0.00090164 milliseconds per selector or 1109.09 selectors per millisecond.
Benchmarking document parsing.
Time taken to parse 100 documents: 13102.2 ms.
Processed at a rate of 131.022 milliseconds per document.
Benchmarking selection speed.
Time taken to run 2764600 selectors against the document: 8783.99 ms producing 42900 total matches.
Processed at a rate of 0.00317731 milliseconds per selector or 314.732 selectors per millisecond.
Benchmarking mutation.
Time taken to run 27646 selectors against the document while serializing with mutations 100 times: 9169.81 ms.
Time per cycle 91.6981 ms.
Processed at a rate of 0.00331687 milliseconds per selector or 301.489 selectors per millisecond.
```

So from these results, a document could be loaded, parsed, and have 27646 precompiled selectors run on it in about **218.8619** milliseconds, a little shy of a quarter of a second. Based on the latest benchmarks (that actually included mutation during serialization) it's about ***222.7201*** msec to load, parse the document, run 27646 selectors and serialize the output with modifications based on those selectors back to a html string.
  
That's almost stretching the "user doesn't notice" goal, but thankfully nowhere near 30K selectors would ever actually need to be run at once. In a more realistic use case of the EasyList, less than half that number of selectors would run on any given html input.

Speed doesn't mean much if the matching code is broken. As such, over 40 tests currently exist that ensure correct functionality of various types of selectors. ~~I have yet to write tests for nested and combined selectors.~~

##Configuration  
It can be pretty handy to see verbose output from GQ for debugging selectors, engine issues, and just plain seeing what's going on under the hood. If you build GQ in Debug and add `GQ_VERBOSE_DEBUG_NFO` to your preprocessor definitions, GQ will generate ***lots*** of console output, detailing nearly every single significant event at various stages of the program. Be advised that this will be a lot of text, so piping it to a file or something similar is recommended.

##TODO
 - ~~Mutation API.~~
 - ~~Tests for combined and nested selectors.~~
 - ~~Reduce candidate collections BEFORE attempting to match in the event that the selector is a GQBinarySelector with the
 intersection operator. Can reduce sets by only keeping candidates that match the traits from both the left and right
 hand sides of the GQBinarySelector, which would drastically reduce candidates and thus drastically increase matching speed.~~ This was tried and abandoned, it's actually faster to just let it chew through all candidates.
 - ~~Modify `GQSelector::Match()` and related methods to return the final matched node. Required for child selectors and such.~~
 - ~~Work around for including root node without having to switch to the abysmal `weak_ptr` in `GQTreeMap`.~~

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
