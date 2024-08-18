#include "absl/synchronization/mutex.h"

// Since WebAssembly is single-threaded we can have a dummy Mutex
//
namespace absl
{
inline namespace lts_20240722
{
void Mutex::Lock()
{
}
void Mutex::Unlock()
{
}
void Mutex::ReaderLock()
{
}
void Mutex::ReaderUnlock()
{
}
} // namespace lts_20240722
} // namespace absl