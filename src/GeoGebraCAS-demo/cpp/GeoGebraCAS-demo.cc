#include "GeoGebraCAS.h"
#include <iostream>

#ifdef _WIN32
#include "tchar.h"
int _tmain(int argc, _TCHAR* argv[]) {
#else
int main(int argc, char* argv[]) {
#endif
        cout << "Starting GeoGebraCAS-demo" << endl;
        // initializeCAS();
        string ret = evaluateCAS("factor(x^2-1)");
        cout << ret << endl;
        ret = evaluateCAS("evalf(7,13)");
        cout << ret << endl;
        ret = evaluateCAS("evalf(7,15)");
        cout << ret << endl;
        ret = evaluateCAS("expand((a+b)^3)");
        cout << ret << endl;
        ret = evaluateCAS("evalfa(when ( type(((x)^(2))+(1)) == DOM_SYMBOLIC && type(x) == DOM_SYMBOLIC , (assume(x),solve(((x)^(2))+(1),x))[size(assume(x),solve(((x)^(2))+(1),x))-1] , when ( type(((x)^(2))+(1)) == DOM_IDENT && type(x) == DOM_SYMBOLIC && ((x)^(2))+(1) == 'x', (assume(x),solve(((x)^(2))+(1)=0,x))[size(assume(x),solve(((x)^(2))+(1)=0,x))-1] ,when ( size(x) == 1,flatten1((normal([op(solve(((x)^(2))+(1),x))]))),(normal([op(solve(((x)^(2))+(1),x))])) ) ) ))");
        cout << ret << endl;
        ret = evaluateCAS("mean([1,2,3,4,1,2,3])");
        cout << ret << endl;
        cout << "Finishing GeoGebraCAS-demo" << endl;
        return 0;
}
