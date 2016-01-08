# GQ
GQ is a CSS Selector Engine for [Gumbo Parser](https://github.com/google/gumbo-parser) written in C++11. Using Gumbo Parser as a backend, GQ can parse input HTML and allow users to select and modify elements in the parsed document with CSS Selectors and the provided simple, but powerful mutation API.

This project is a fork of [gumbo-query](https://github.com/lazytiger/gumbo-query). I opted to have this be an unofficial fork because I intended on performing nearly a complete rewrite, which I did, and as such this source is completely irreconcilable with the original gumbo-query source.

##Usage

You can either construct a Document around an existing GumboOutput pointer, at which point the Document will assume managing the lifetime of the GumboOutput, or you can supply a raw string of HTML for Document to parse and also maintain.
```c++
std::string someHtmlString = "...";
std::string someSelectorString = "...";
auto testDocument = gq::Document::Create();
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

As you can see, you can run raw selector strings into the `::Find(...)` method, but each time, the selector string will be "compiled" into a SharedSelector and destroyed. You can alternatively "precompile" and save built selectors, and as such avoid wrapping every `::Find(...)` call in a try/catch.

```c++
GumboOutput* output = SOMETHING_NOT_NULL;
auto testDocument = gq::Document::Create(output);

gq::Parser parser;

std::vector<std::string> collectionOfRawSelectorStrings {...};
std::vector<gq::SharedSelector> compiledSelectors();
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

The contract placed on the end user is very light. Keep Document alive for as long as you're storing or accessing any Node object, directly or indirectly. That's basically it.

##Speed
One of the primary goals with this engine was to maximize speed. For my purposes, I wanted to ensure I could run an insane amount of selectors without any visible delay to the user. Running the TestParser test benchmarks parsing and using every single selector in [EasyList](https://easylist.adblockplus.org/en/) (spare a handful which were removed because they're improperly formatted) against a standard high profile website's landing page HTML. For example, if I download the source for the landing page of [yahoo.com](https://yahoo.com) and use it in the parser test at the time of this writing, the current results on my [dev laptop](https://www.asus.com/ca-en/ROG-Republic-Of-Gamers/ASUS_ROG_G750JM/) are:

```
Processed 27646 selectors. Had handled errors? false
Benchmarking parsing speed.
Time taken to parse 2764600 selectors: 2443.11 ms.
Processed at a rate of 0.000883713 milliseconds per selector or 1131.59 selectors per millisecond.
Benchmarking document parsing.
Time taken to parse 100 documents: 8054.37 ms.
Processed at a rate of 80.5437 milliseconds per document.
Benchmarking selection speed.
Time taken to run 2764600 selectors against the document: 5709.75 ms producing 23300 total matches.
Processed at a rate of 0.00206531 milliseconds per selector or 484.189 selectors per millisecond.
Benchmarking mutation.
Time taken to run 27646 selectors against the document while serializing with mutations 100 times: 6110.32 ms.
Time per cycle 61.1032 ms.
Processed at a rate of 0.0022102 milliseconds per selector or 452.448 selectors per millisecond.
```

So from these results, a document could be loaded, parsed, and have 27646 precompiled selectors run on it in about **137.6412** milliseconds. If you include reserializing the input to an HTML string with modifications, it's about ***141.6469*** msec to load, parse the document, run 27646 selectors and serialize the output with modifications based on those selectors back to an HTML string.

It should be obvious that the speed can greatly vary depending on the size and complexity of the input HTML. For example, running the same test program against the [cnn.com](http://cnn.com) landing page yields the following results:

```
Processed 27646 selectors. Had handled errors? false
Benchmarking parsing speed.
Time taken to parse 2764600 selectors: 2396.14 ms.
Processed at a rate of 0.000866723 milliseconds per selector or 1153.77 selectors per millisecond.
Benchmarking document parsing.
Time taken to parse 100 documents: 2081.5 ms.
Processed at a rate of 20.815 milliseconds per document.
Benchmarking selection speed.
Time taken to run 2764600 selectors against the document: 3321.3 ms producing 9900 total matches.
Processed at a rate of 0.00120137 milliseconds per selector or 832.386 selectors per millisecond.
Benchmarking mutation.
Time taken to run 27646 selectors against the document while serializing with mutations 100 times: 3478.62 ms.
Time per cycle 34.7862 ms.
Processed at a rate of 0.00125827 milliseconds per selector or 794.741 selectors per millisecond.
```

As you can see, approaching double the speed over the [yahoo.com](https://yahoo.com) website's landing page.  

Speed doesn't mean much if the matching code is broken. As such, over 40 tests currently exist that ensure correct functionality of various types of selectors. ~~I have yet to write tests for nested and combined selectors.~~

##Configuration  
Presently, there are only scripts/project files for building GQ under Windows with Visual Studio 2015. There is no reason why GQ cannot be used under Linux or OSX, I just simply have not gone there yet. It will come soon. There is a minimal amount of setup required for building under Windows with VS, and it's detailed in the [Wiki](https://github.com/TechnikEmpire/GQ/wiki).

##TODO
 - ~~Mutation API.~~
 - ~~Tests for combined and nested selectors.~~
 - ~~Reduce candidate collections BEFORE attempting to match in the event that the selector is a BinarySelector with the
 intersection operator. Can reduce sets by only keeping candidates that match the traits from both the left and right
 hand sides of the BinarySelector, which would drastically reduce candidates and thus drastically increase matching speed.~~ This was tried and abandoned, it's actually faster to just let it chew through all candidates.
 - ~~Modify `Selector::Match()` and related methods to return the final matched node. Required for child selectors and such.~~
 - ~~Work around for including root node without having to switch to the abysmal `weak_ptr` in `TreeMap`.~~
 - Scripts/Project Files for building/using under Linux, OSX.

##Original Goals  
 - Wrapping things up in proper namespaces.
 - Remove custom rolled automatic reference counting, remove any sort of `shared_ptr` and make lifetime management simple.
 - Fix broken parsing that was ported from cascadia, but is invalid for use with Gumbo Parser.
 - Make parsing/matching produce the same behavior as jQuery does on the exact same test data.
 - Replace `std::string` with `boost::string_ref` wherever string copies don't truly need to be generated.  
 - Implement a mapping system to dramatically increase matching speed by filtering potential matches by traits.
 - Remove local state tracking from the selector parser.
 - Expose compiled selectors to the public so that they can be retained and recycled against existing and new documents.
 - "Comments. Lots of comments."
 - "Speed. Lots of Speed."
