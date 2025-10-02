#ifndef APP_BINDING_H
#define APP_BINDING_H

#include "common.h"

namespace CExpress {

class AppBinding : public Napi::ObjectWrap<AppBinding> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;
    static Napi::Value NewInstance(Napi::Env env);
    static Napi::Function GetConstructor(Napi::Env env);
    
    AppBinding(const Napi::CallbackInfo& info);
    ~AppBinding();
    
private:
    App* app_;
    std::vector<std::unique_ptr<JSCallbackContext>> callbacks_;
    
    // HTTP method handlers
    CEXPRESS_METHOD(Get);
    CEXPRESS_METHOD(Post);
    CEXPRESS_METHOD(Put);
    CEXPRESS_METHOD(Delete);
    CEXPRESS_METHOD(Patch);
    CEXPRESS_METHOD(Options);
    CEXPRESS_METHOD(Head);
    
    // Middleware
    CEXPRESS_METHOD(Use);
    
    // Router mounting
    CEXPRESS_METHOD(Mount);
    
    // Server control
    CEXPRESS_METHOD(Listen);
    
    // Error handling
    CEXPRESS_METHOD(SetErrorHandler);
    
    // Utility methods
    CEXPRESS_METHOD(PrintRoutes);
    CEXPRESS_METHOD(GetRoutes);
    CEXPRESS_METHOD(ToString);
    CEXPRESS_GETTER(GetVersion);
    
    // Internal helper methods
    void RegisterRoute(const Napi::CallbackInfo& info, const char* method);
    JSCallbackContext* CreateCallbackContext(Napi::Env env, const Napi::Value& callback, bool is_middleware = false);
    static void HandleRequest(int client_fd, void (*next)(void*), void* context);
};

} // namespace CExpress

#endif // APP_BINDING_H
