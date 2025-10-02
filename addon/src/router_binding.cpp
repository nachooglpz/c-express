#include "router_binding.h"

namespace CExpress {

Napi::FunctionReference RouterBinding::constructor;

Napi::Object RouterBinding::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    
    Napi::Function func = DefineClass(env, "Router", {
        InstanceMethod("get", &RouterBinding::Get),
        InstanceMethod("post", &RouterBinding::Post),
        InstanceMethod("put", &RouterBinding::Put),
        InstanceMethod("delete", &RouterBinding::Delete),
        InstanceMethod("patch", &RouterBinding::Patch),
        InstanceMethod("options", &RouterBinding::Options),
        InstanceMethod("use", &RouterBinding::Use)
    });
    
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    
    exports.Set("Router", func);
    return exports;
}

Napi::Function RouterBinding::GetConstructor(Napi::Env env) {
    return constructor.Value();
}

Napi::Value RouterBinding::NewInstance(Napi::Env env) {
    Napi::EscapableHandleScope scope(env);
    Napi::Object obj = constructor.New({});
    return scope.Escape(napi_value(obj)).ToObject();
}

RouterBinding::RouterBinding(const Napi::CallbackInfo& info) : Napi::ObjectWrap<RouterBinding>(info) {
    // TODO: Create C Router instance
    router_ = nullptr;
}

RouterBinding::~RouterBinding() {
    // TODO: Clean up router
    if (router_) {
        router_ = nullptr;
    }
}

Napi::Value RouterBinding::Get(const Napi::CallbackInfo& info) {
    // TODO: Implement router methods
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value RouterBinding::Post(const Napi::CallbackInfo& info) {
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value RouterBinding::Put(const Napi::CallbackInfo& info) {
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value RouterBinding::Delete(const Napi::CallbackInfo& info) {
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value RouterBinding::Patch(const Napi::CallbackInfo& info) {
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value RouterBinding::Options(const Napi::CallbackInfo& info) {
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value RouterBinding::Use(const Napi::CallbackInfo& info) {
    Napi::Error::New(info.Env(), "Router methods not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

} // namespace CExpress
