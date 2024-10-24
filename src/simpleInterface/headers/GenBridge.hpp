#include "ContextBridge.hpp"
#include <string>
#include <vector>

using namespace std;
namespace giac {
	class gen;	
}

class GenBridge {
public:
	giac::gen* g;
	GenBridge(string expression, ContextBridge& context);
	GenBridge(giac::gen* g);
	~GenBridge();
	void selfEval(int level, ContextBridge& context);
	void resetTimeout();
	GenBridge* eval(int level, ContextBridge& context);
	string print(ContextBridge& context);
	int type();
	GenBridge* lname(ContextBridge& context);
	void getListVector(vector<GenBridge*>& list);
	int lenght();
	int isNull();
	int equalSign(GenBridge& other);
	GenBridge* left(ContextBridge& context);
	GenBridge* right(ContextBridge& context);
	GenBridge* canonicalForm(ContextBridge& context);
	GenBridge* regroup(ContextBridge& context);
	GenBridge* simplify(ContextBridge& context);
	GenBridge* operator+(GenBridge& other);
	GenBridge* operator-(GenBridge& other);
	GenBridge* operator*(GenBridge& other);
	GenBridge* operator/(GenBridge& other);
	GenBridge* operator-();
};
