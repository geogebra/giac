#include <node.h>
#include <v8.h>

#include "giac.h"

using namespace std;
using namespace giac;

using namespace v8;

context ct;

void Evaluate(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();
  HandleScope scope(isolate);
  std::string line_out;

  v8::String::Utf8Value param1(isolate, args[0]->ToString(context).ToLocalChecked());
  std::string line_in = std::string(*param1);

  gen g;
  g=gen(line_in,&ct);
  try {
    line_out = giac::print(giac::eval(g,&ct),&ct);
  } catch (runtime_error & err) {
    line_out = std::string("ERROR: ") + err.what();
  }

  Local<Value> line_out_v8 = String::NewFromUtf8(isolate, line_out.c_str(), NewStringType::kNormal).ToLocalChecked();
  args.GetReturnValue().Set(line_out_v8);
}

void Initialize(Local<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();
  exports->Set(context, String::NewFromUtf8(isolate, "evaluate", NewStringType::kNormal).ToLocalChecked(),
      FunctionTemplate::New(isolate, Evaluate)->GetFunction(context).ToLocalChecked());
}

NODE_MODULE(giac, Initialize)