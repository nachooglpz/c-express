#ifndef ADDON_COMMON_H
#define ADDON_COMMON_H

#include <napi.h>

// Temporarily rename delete to avoid C++ keyword conflict
#define delete delete_method

extern "C" {
    #include "core/app.h"
    #include "core/router.h"
    #include "core/route.h"
    #include "http/request.h"
    #include "http/response.h"
    #include "http/error.h"
    #include "parsers/json.h"
}

// Restore delete after including C headers
#undef delete

namespace CExpress {

// Forward declarations
class AppBinding;
class RequestBinding;
class ResponseBinding;
class RouterBinding;

// Utility functions for type conversion
Napi::Value ConvertCStringToJS(Napi::Env env, const char* str);
std::string ConvertJSStringToC(const Napi::Value& val);
bool IsValidJSFunction(const Napi::Value& val);

// Error handling utilities
void ThrowCExpressError(Napi::Env env, const char* message);
void ThrowCExpressError(Napi::Env env, const std::string& message);

// Memory management utilities
void RegisterCleanup(Napi::Env env, void* ptr, void (*cleanup_fn)(void*));

// JavaScript callback wrapper structure
struct JSCallbackContext {
    Napi::ThreadSafeFunction tsfn;
    Napi::FunctionReference callback_ref;
    void* user_data;
    bool is_middleware;
};

// Middleware/Handler bridge function
void BridgeHandler(int client_fd, void (*next)(void*), void* context);

// Utility macros
#define CEXPRESS_METHOD(name) \
    Napi::Value name(const Napi::CallbackInfo& info)

#define CEXPRESS_GETTER(name) \
    Napi::Value name(const Napi::CallbackInfo& info)

#define CEXPRESS_SETTER(name) \
    void name(const Napi::CallbackInfo& info, const Napi::Value& value)

// Validation macros for functions returning Napi::Value
#define CEXPRESS_VALIDATE_ARGS(info, expected_count, error_msg) \
    do { \
        if (info.Length() < expected_count) { \
            Napi::TypeError::New(info.Env(), error_msg).ThrowAsJavaScriptException(); \
            return info.Env().Undefined(); \
        } \
    } while(0)

#define CEXPRESS_VALIDATE_STRING(val, error_msg) \
    do { \
        if (!val.IsString()) { \
            Napi::TypeError::New(val.Env(), error_msg).ThrowAsJavaScriptException(); \
            return val.Env().Undefined(); \
        } \
    } while(0)

#define CEXPRESS_VALIDATE_FUNCTION(val, error_msg) \
    do { \
        if (!val.IsFunction()) { \
            Napi::TypeError::New(val.Env(), error_msg).ThrowAsJavaScriptException(); \
            return val.Env().Undefined(); \
        } \
    } while(0)

// Validation macros for void functions
#define CEXPRESS_VALIDATE_ARGS_VOID(info, expected_count, error_msg) \
    do { \
        if (info.Length() < expected_count) { \
            Napi::TypeError::New(info.Env(), error_msg).ThrowAsJavaScriptException(); \
            return; \
        } \
    } while(0)

#define CEXPRESS_VALIDATE_STRING_VOID(val, error_msg) \
    do { \
        if (!val.IsString()) { \
            Napi::TypeError::New(val.Env(), error_msg).ThrowAsJavaScriptException(); \
            return; \
        } \
    } while(0)

#define CEXPRESS_VALIDATE_FUNCTION_VOID(val, error_msg) \
    do { \
        if (!val.IsFunction()) { \
            Napi::TypeError::New(val.Env(), error_msg).ThrowAsJavaScriptException(); \
            return; \
        } \
    } while(0)

} // namespace CExpress

#endif // ADDON_COMMON_H
