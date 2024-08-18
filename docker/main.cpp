// We are using absl::string_view since RE2 already uses that; no need to bring
// in std::string_view and inflate the binary size.
//
#include "absl/strings/string_view.h"
#include <emscripten.h>
#include <re2/re2.h>

// Globals to save a compiled pattern across invocations.
//
absl::string_view latchedPattern;
RE2* latchedInstance;

#ifdef COMPILE_MATCH
extern "C" EMSCRIPTEN_KEEPALIVE bool Match(
    char* testPtr, size_t testLen, char* patternPtr, size_t patternLen)
{
    absl::string_view test(testPtr, testLen);
    absl::string_view pattern(patternPtr, patternLen);

    if (latchedInstance != nullptr && pattern != latchedPattern)
    {
        bool result = RE2::PartialMatch(test, pattern);
        free(patternPtr);
        free(testPtr);
        return result;
    }

    if (latchedInstance == nullptr)
    {
        latchedPattern = pattern;
        latchedInstance = new RE2(pattern, RE2::Quiet);
    }
    else
    {
        free(patternPtr);
    }

    bool result = RE2::PartialMatch(test, *latchedInstance);
    free(testPtr);
    return result;
}
#endif

#ifdef COMPILE_MATCH_EXTRACT
struct MatchExtractResult
{
    bool matched;
    char* submatchPtr;
    size_t submatchLen;
};

// Since we have a multi-value return, we'll return the address of this global
// struct containing the multiple return values.
//
MatchExtractResult matchExtractResult;

// MatchExtract returns the result along with a single submatch
// To return the entire pattern, wrap the whole thing in parentheses
//
extern "C" EMSCRIPTEN_KEEPALIVE void* MatchExtract(
    char* testPtr, size_t testLen, char* patternPtr, size_t patternLen)
{
    absl::string_view test(testPtr, testLen);
    absl::string_view pattern(patternPtr, patternLen);

    if (latchedInstance != nullptr && pattern != latchedPattern)
    {
        absl::string_view extracted;
        if (!RE2::PartialMatch(test, pattern, &extracted))
        {
            free(patternPtr);
            free(testPtr);
            matchExtractResult = {false, nullptr, 0};
            return &matchExtractResult;
        }

        free(patternPtr);

        // Rather than free testPtr, since we own it reuse it for the result
        //
        memcpy(testPtr, extracted.data(), extracted.size());
        matchExtractResult = {true, testPtr, extracted.size()};
        return &matchExtractResult;
    }

    if (latchedInstance == nullptr)
    {
        latchedPattern = pattern;
        latchedInstance = new RE2(pattern, RE2::Quiet);
    }
    else
    {
        free(patternPtr);
    }

    absl::string_view extracted;
    if (!RE2::PartialMatch(test, *latchedInstance, &extracted))
    {
        free(testPtr);
        matchExtractResult = {false, nullptr, 0};
        return &matchExtractResult;
    }

    // Rather than free testPtr, since we own it reuse it for the result
    //
    memcpy(testPtr, extracted.data(), extracted.size());
    matchExtractResult = {true, testPtr, extracted.size()};
    return &matchExtractResult;
}
#endif
