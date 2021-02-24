#include "GeoGebraCAS.h"
#include <stdexcept>
#include <giac.h>

using namespace std;
using namespace giac;

#ifndef _WIN32
#define EXPORT __attribute__((visibility("default")))
__attribute__((constructor))
static void initializer(void) { }
__attribute__((destructor))
static void finalizer(void) { }
#else
#define EXPORT
#endif

context ct;

extern "C" {
    EXPORT void initializeCAS() { return; }
    EXPORT string evaluateCAS(string command) {
        gen e(string(command), &ct);
        try {
            return giac::print(giac::eval(e, &ct), &ct);
            } catch (std::runtime_error & err) {
            cerr << err.what() << endl;
            }
        }
    }
