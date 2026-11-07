// wasm32-wasi exception stubs.
//
// zig's wasm32-wasi libc++ ships without a C++ exception runtime, so code that
// contains `throw` fails to link. These two functions make every throw print
// the exception's what() and abort, which is what an uncaught exception does
// anyway. Catch blocks therefore never run under WASI; errors still end the
// program with a readable message. Only linked into the wasm build.
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <typeinfo>

extern "C" void* __cxa_allocate_exception(size_t size) noexcept { return std::malloc(size); }

extern "C" [[noreturn]] void __cxa_throw(void* object, std::type_info* type, void (*)(void*)) {
    // Every exception thrown by this code base derives from std::exception.
    std::fprintf(stderr, "fatal: C++ exception (%s): %s\n", type ? type->name() : "?",
                 static_cast<std::exception*>(object)->what());
    std::abort();
}
