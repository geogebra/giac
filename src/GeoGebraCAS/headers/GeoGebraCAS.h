#ifdef _WIN32
#ifdef GEOGEBRACAS_EXPORTS
#define GEOGEBRACAS_API __declspec(dllexport) 
#else
#define GEOGEBRACAS_API __declspec(dllimport) 
#endif
#else
#define GEOGEBRACAS_API
#endif

using namespace std;
#include <string>

extern "C" {
    GEOGEBRACAS_API void initializeCAS();
    GEOGEBRACAS_API string evaluateCAS(string command);
    };
