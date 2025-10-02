#ifndef REQUEST_BINDING_H
#define REQUEST_BINDING_H

#include "common.h"

namespace CExpress {

class RequestBinding : public Napi::ObjectWrap<RequestBinding> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;
    static Napi::Value NewInstance(Napi::Env env, Request* req);
    
    RequestBinding(const Napi::CallbackInfo& info);
    ~RequestBinding();
    
private:
    Request* req_;
    bool owns_request_;
    
    // Property getters
    CEXPRESS_GETTER(GetMethod);
    CEXPRESS_GETTER(GetPath);
    CEXPRESS_GETTER(GetQuery);
    CEXPRESS_GETTER(GetParams);
    CEXPRESS_GETTER(GetHeaders);
    CEXPRESS_GETTER(GetBody);
    
    // Utility methods
    CEXPRESS_METHOD(GetHeader);
    CEXPRESS_METHOD(GetParam);
    CEXPRESS_METHOD(GetQueryParam);
    
    // Internal helpers
    void SetRequest(Request* req, bool owns = false);
};

} // namespace CExpress

#endif // REQUEST_BINDING_H
