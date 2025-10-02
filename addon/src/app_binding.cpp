#include "app_binding.h"
#include "request_binding.h"
#include "response_binding.h"
#include <thread>
#include <memory>

namespace CExpress {

Napi::FunctionReference AppBinding::constructor;

Napi::Object AppBinding::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    
    Napi::Function func = DefineClass(env, "App", {
        InstanceMethod("get", &AppBinding::Get),
        InstanceMethod("post", &AppBinding::Post),
        InstanceMethod("put", &AppBinding::Put),
        InstanceMethod("delete", &AppBinding::Delete),
        InstanceMethod("patch", &AppBinding::Patch),
        InstanceMethod("options", &AppBinding::Options),
        InstanceMethod("head", &AppBinding::Head),
        InstanceMethod("use", &AppBinding::Use),
        InstanceMethod("mount", &AppBinding::Mount),
        InstanceMethod("listen", &AppBinding::Listen),
        InstanceMethod("error", &AppBinding::SetErrorHandler),
        InstanceMethod("printRoutes", &AppBinding::PrintRoutes),
        InstanceMethod("getRoutes", &AppBinding::GetRoutes),
        // Add utility methods
        InstanceMethod("toString", &AppBinding::ToString),
        InstanceAccessor("version", &AppBinding::GetVersion, nullptr)
    });
    
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    
    exports.Set("App", func);
    return exports;
}

Napi::Function AppBinding::GetConstructor(Napi::Env env) {
    return constructor.Value();
}

Napi::Value AppBinding::NewInstance(Napi::Env env) {
    Napi::EscapableHandleScope scope(env);
    Napi::Object obj = constructor.New({});
    return scope.Escape(napi_value(obj)).ToObject();
}

AppBinding::AppBinding(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AppBinding>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    
    // Create the underlying C App structure
    App temp_app = create_app();
    app_ = new App(temp_app);
    
    if (!app_) {
        Napi::Error::New(env, "Failed to create C-Express app").ThrowAsJavaScriptException();
        return;
    }
    
    // Initialize callback storage
    callbacks_.reserve(32); // Pre-allocate space for better performance
}

AppBinding::~AppBinding() {
    // Clean up the app and all callbacks
    if (app_) {
        // TODO: Add proper app cleanup function in C
        delete app_;
        app_ = nullptr;
    }
    
    // Clear all callback contexts
    callbacks_.clear();
}

Napi::Value AppBinding::Get(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "GET");
    return info.This();
}

Napi::Value AppBinding::Post(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "POST");
    return info.This();
}

Napi::Value AppBinding::Put(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "PUT");
    return info.This();
}

Napi::Value AppBinding::Delete(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "DELETE");
    return info.This();
}

Napi::Value AppBinding::Patch(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "PATCH");
    return info.This();
}

Napi::Value AppBinding::Options(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "OPTIONS");
    return info.This();
}

Napi::Value AppBinding::Head(const Napi::CallbackInfo& info) {
    RegisterRoute(info, "HEAD");
    return info.This();
}

void AppBinding::RegisterRoute(const Napi::CallbackInfo& info, const char* method) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS_VOID(info, 2, "Expected path and handler function");
    CEXPRESS_VALIDATE_STRING_VOID(info[0], "Path must be a string");
    CEXPRESS_VALIDATE_FUNCTION_VOID(info[1], "Handler must be a function");
    
    std::string path = ConvertJSStringToC(info[0]);
    
    // Create callback context for the handler
    auto* callback_ctx = CreateCallbackContext(env, info[1], false);
    if (!callback_ctx) {
        Napi::Error::New(env, "Failed to create callback context").ThrowAsJavaScriptException();
        return;
    }
    
    // Register the route with the C app
    if (strcmp(method, "GET") == 0) {
        app_->get(app_, path.c_str(), HandleRequest);
    } else if (strcmp(method, "POST") == 0) {
        app_->post(app_, path.c_str(), HandleRequest);
    } else if (strcmp(method, "PUT") == 0) {
        app_->put(app_, path.c_str(), HandleRequest);
    } else if (strcmp(method, "DELETE") == 0) {
        app_->delete_method(app_, path.c_str(), HandleRequest);
    } else if (strcmp(method, "PATCH") == 0) {
        app_->patch(app_, path.c_str(), HandleRequest);
    } else if (strcmp(method, "OPTIONS") == 0) {
        app_->options(app_, path.c_str(), HandleRequest);
    }
    
    // Store the callback context
    callbacks_.push_back(std::unique_ptr<JSCallbackContext>(callback_ctx));
}

Napi::Value AppBinding::Use(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected handler function");
    CEXPRESS_VALIDATE_FUNCTION(info[0], "Handler must be a function");
    
    // Create callback context for the middleware
    auto* callback_ctx = CreateCallbackContext(env, info[0], true);
    if (!callback_ctx) {
        Napi::Error::New(env, "Failed to create callback context").ThrowAsJavaScriptException();
        return info.This();
    }
    
    // Register the middleware with the C app
    app_->use(app_, HandleRequest);
    
    // Store the callback context
    callbacks_.push_back(std::unique_ptr<JSCallbackContext>(callback_ctx));
    
    return info.This();
}

Napi::Value AppBinding::Mount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 2, "Expected prefix and router");
    CEXPRESS_VALIDATE_STRING(info[0], "Prefix must be a string");
    
    // TODO: Implement router mounting when RouterBinding is complete
    Napi::Error::New(env, "Router mounting not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value AppBinding::Listen(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected port number");
    
    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Port must be a number").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    
    int port = info[0].As<Napi::Number>().Int32Value();
    
    // Optional callback for when server starts
    Napi::Function callback;
    if (info.Length() > 1 && info[1].IsFunction()) {
        callback = info[1].As<Napi::Function>();
    }
    
    // Start the server in a separate thread to avoid blocking Node.js event loop
    std::thread server_thread([this, port, callback, env]() {
        app_->listen(app_, port);
        
        // Call the callback if provided
        if (!callback.IsEmpty()) {
            callback.Call({});
        }
    });
    
    server_thread.detach();
    
    return info.This();
}

Napi::Value AppBinding::SetErrorHandler(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected error handler function");
    CEXPRESS_VALIDATE_FUNCTION(info[0], "Error handler must be a function");
    
    // TODO: Implement error handler binding
    Napi::Error::New(env, "Error handler not yet implemented").ThrowAsJavaScriptException();
    return info.This();
}

Napi::Value AppBinding::PrintRoutes(const Napi::CallbackInfo& info) {
    if (app_) {
        // TODO: Implement route printing when available in C API
        printf("C-Express Routes (placeholder implementation)\n");
    }
    return info.This();
}

Napi::Value AppBinding::GetRoutes(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // TODO: Implement route introspection
    Napi::Array routes = Napi::Array::New(env);
    
    // For now, return basic information about registered callbacks
    for (size_t i = 0; i < callbacks_.size(); i++) {
        Napi::Object route = Napi::Object::New(env);
        route.Set("index", static_cast<uint32_t>(i));
        route.Set("type", callbacks_[i]->is_middleware ? "middleware" : "route");
        routes.Set(static_cast<uint32_t>(i), route);
    }
    
    return routes;
}

Napi::Value AppBinding::ToString(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    std::string appInfo = "[C-Express App";
    if (app_) {
        appInfo += " (active)";
    } else {
        appInfo += " (inactive)";
    }
    appInfo += " - " + std::to_string(callbacks_.size()) + " handlers]";
    
    return Napi::String::New(env, appInfo);
}

Napi::Value AppBinding::GetVersion(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "1.0.0");
}

JSCallbackContext* AppBinding::CreateCallbackContext(Napi::Env env, const Napi::Value& callback, bool is_middleware) {
    if (!callback.IsFunction()) {
        return nullptr;
    }
    
    auto* ctx = new JSCallbackContext();
    ctx->callback_ref = Napi::Persistent(callback.As<Napi::Function>());
    ctx->user_data = this;  // Store reference to this AppBinding instance
    ctx->is_middleware = is_middleware;
    
    return ctx;
}

void AppBinding::HandleRequest(int client_fd, void (*next)(void*), void* context) {
    // This is the bridge function that gets called from C
    // We need to extract the JavaScript callback and call it
    
    // TODO: Implement the bridge between C handler and JavaScript callback
    // This is a complex part that requires:
    // 1. Extracting the NextContext from context
    // 2. Creating Request and Response JavaScript objects
    // 3. Calling the JavaScript callback with proper arguments
    // 4. Handling the response
    
    // For now, just call next to continue the middleware chain
    if (next && context) {
        next(context);
    }
}

} // namespace CExpress
