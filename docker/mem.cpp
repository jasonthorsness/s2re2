#include <emscripten.h>

extern "C" EMSCRIPTEN_KEEPALIVE void* canonical_abi_realloc(
    void* ptr, size_t /* currentSize */, size_t /* align */, size_t newSize)
{
    void* result = realloc(ptr, newSize);
    if (result == 0)
    {
        abort();
    }
    return result;
}

extern "C" EMSCRIPTEN_KEEPALIVE void canonical_abi_free(
    void* ptr, size_t /* size */, size_t /* align*/)
{
    free(ptr);
}