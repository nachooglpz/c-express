#include "common.h"
#include <string>

namespace CExpress {

Napi::Value ConvertCStringToJS(Napi::Env env, const char* str) {
    if (!str) {
        return env.Null();
    }
    return Napi::String::New(env, str);
}

std::string ConvertJSStringToC(const Napi::Value& val) {
    if (!val.IsString()) {
        return "";
    }
    return val.As<Napi::String>().Utf8Value();
}

bool IsValidJSFunction(const Napi::Value& val) {
    return val.IsFunction();
}

void ThrowCExpressError(Napi::Env env, const char* message) {
    Napi::Error::New(env, message).ThrowAsJavaScriptException();
}

void ThrowCExpressError(Napi::Env env, const std::string& message) {
    Napi::Error::New(env, message).ThrowAsJavaScriptException();
}

void RegisterCleanup(Napi::Env env, void* ptr, void (*cleanup_fn)(void*)) {
    // Register a cleanup function to be called when the environment is destroyed
    // This is useful for cleaning up C resources
    env.AddCleanupHook([ptr, cleanup_fn]() {
        if (ptr && cleanup_fn) {
            cleanup_fn(ptr);
        }
    });
}

void BridgeHandler(int client_fd, void (*next)(void*), void* context) {
    // This is a generic bridge function that can be used by different binding classes
    // The actual implementation will depend on the specific context
    
    // For now, just call next to continue the middleware chain
    if (next && context) {
        next(context);
    }
}

} // namespace CExpress
