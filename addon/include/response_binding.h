#ifndef RESPONSE_BINDING_H
#define RESPONSE_BINDING_H

#include "common.h"

namespace CExpress {

class ResponseBinding : public Napi::ObjectWrap<ResponseBinding> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;
    static Napi::Value NewInstance(Napi::Env env, Response* res);
    
    ResponseBinding(const Napi::CallbackInfo& info);
    ~ResponseBinding();
    
private:
    Response* res_;
    bool owns_response_;
    
    // Core response methods
    CEXPRESS_METHOD(Send);
    CEXPRESS_METHOD(Json);
    CEXPRESS_METHOD(Status);
    CEXPRESS_METHOD(SetHeader);
    CEXPRESS_METHOD(GetHeader);
    CEXPRESS_METHOD(End);
    
    // Convenience methods
    CEXPRESS_METHOD(Type);
    CEXPRESS_METHOD(Cookie);
    CEXPRESS_METHOD(ClearCookie);
    CEXPRESS_METHOD(Redirect);
    
    // Property getters/setters
    CEXPRESS_GETTER(GetStatusCode);
    CEXPRESS_SETTER(SetStatusCode);
    
    // Internal helpers
    void SetResponse(Response* res, bool owns = false);
};

} // namespace CExpress

#endif // RESPONSE_BINDING_H
