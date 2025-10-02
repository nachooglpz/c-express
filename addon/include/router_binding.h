#ifndef ROUTER_BINDING_H
#define ROUTER_BINDING_H

#include "common.h"

namespace CExpress {

class RouterBinding : public Napi::ObjectWrap<RouterBinding> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;
    static Napi::Value NewInstance(Napi::Env env);
    static Napi::Function GetConstructor(Napi::Env env);
    
    RouterBinding(const Napi::CallbackInfo& info);
    ~RouterBinding();
    
private:
    Router* router_;
    
    // HTTP method handlers
    CEXPRESS_METHOD(Get);
    CEXPRESS_METHOD(Post);
    CEXPRESS_METHOD(Put);
    CEXPRESS_METHOD(Delete);
    CEXPRESS_METHOD(Patch);
    CEXPRESS_METHOD(Options);
    
    // Middleware
    CEXPRESS_METHOD(Use);
};

} // namespace CExpress

#endif // ROUTER_BINDING_H
