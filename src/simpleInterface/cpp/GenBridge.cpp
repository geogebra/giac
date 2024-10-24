#include "GenBridge.hpp"
#include "giac.h"
#include "vecteur.h"

GenBridge::GenBridge(string expression, ContextBridge& context) {
	g = new giac::gen(expression, context.c);
}

GenBridge::GenBridge(giac::gen* g) {
	this->g = g;
}

GenBridge::~GenBridge() {
	delete g;
}

void GenBridge::resetTimeout() {
	g->resetTimeout();
}

void GenBridge::selfEval(int level, ContextBridge& context) {
	try {
		giac::gen* newg = new giac::gen(g->eval(level, context.c));
		delete g;
		g = newg;
	} catch (...) {
		throw "Error in selfEval: giac::gen::eval() failed";
	}
}

GenBridge* GenBridge::eval(int level, ContextBridge& context) {
	GenBridge* pointer = new GenBridge(new giac::gen(g->eval(level, context.c)));
	return pointer;
}

string GenBridge::print(ContextBridge& context) {
	return g->print(context.c);
}

GenBridge* GenBridge::lname(ContextBridge& context) {
	GenBridge* pointer = new GenBridge(new giac::gen("lname(" + g->print(context.c) + ")", context.c));
	pointer->selfEval(1, context);
	return pointer;
}

int GenBridge::type() {
	return g->type;
}

int GenBridge::lenght() {
	if (g->type == 7) { // type is vector
		giac::vecteur* v = g->_VECTptr;
		return v->size();
	} else {
		return 1;
	}
}

int GenBridge::isNull() {
	return (g) ? 0 : 1;
}

void GenBridge::getListVector(vector<GenBridge*>& list) {
	if (g->type == 7) { // type is vector
		giac::vecteur* v = g->_VECTptr;
		for (int i = 0; i < v->size(); i++) {
			list.push_back(new GenBridge(new giac::gen(v->at(i))));
		}
	} else {
		throw "Error in getListVector: type is not vector";
	}
}

int GenBridge::equalSign(GenBridge& other) {
	return *g == *other.g;
}

GenBridge* GenBridge::left(ContextBridge& context) {
	GenBridge* pointer =  new GenBridge(new giac::gen("left(" + g->print(context.c) + ")", context.c));
	pointer->selfEval(1, context);
	return pointer;
}

GenBridge* GenBridge::right(ContextBridge& context) {
	GenBridge* pointer =  new GenBridge(new giac::gen("right(" + g->print(context.c) + ")", context.c));
	pointer->selfEval(1, context);
	return pointer;
}

GenBridge* GenBridge::canonicalForm(ContextBridge& context) {
	GenBridge* pointer =  new GenBridge(new giac::gen("canonical_form(" + g->print(context.c) + ")", context.c));
	pointer->selfEval(1, context);
	return pointer;
}

GenBridge* GenBridge::regroup(ContextBridge& context) {
	GenBridge* pointer =  new GenBridge(new giac::gen("regroup(" + g->print(context.c) + ")", context.c));
	pointer->selfEval(1, context);
	return pointer;
}

GenBridge* GenBridge::simplify(ContextBridge& context) {
	GenBridge* pointer =  new GenBridge(new giac::gen("simplify(" + g->print(context.c) + ")", context.c));
	pointer->selfEval(1, context);
	return pointer;
}

GenBridge* GenBridge::operator+(GenBridge& other) {
	return new GenBridge(new giac::gen(*g + *other.g));
}

GenBridge* GenBridge::operator-(GenBridge& other) {
	return new GenBridge(new giac::gen(*g - *other.g));
}

GenBridge* GenBridge::operator*(GenBridge& other) {
	return new GenBridge(new giac::gen(*g * *other.g));
}

GenBridge* GenBridge::operator/(GenBridge& other) {
	return new GenBridge(new giac::gen(*g / *other.g));
}

GenBridge* GenBridge::operator-() {
	return new GenBridge(new giac::gen(-*g));
}