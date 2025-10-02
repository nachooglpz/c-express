#include "response_binding.h"
#include <cstring>

namespace CExpress {

Napi::FunctionReference ResponseBinding::constructor;

Napi::Object ResponseBinding::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    
    Napi::Function func = DefineClass(env, "Response", {
        InstanceMethod("send", &ResponseBinding::Send),
        InstanceMethod("json", &ResponseBinding::Json),
        InstanceMethod("status", &ResponseBinding::Status),
        InstanceMethod("set", &ResponseBinding::SetHeader),
        InstanceMethod("header", &ResponseBinding::SetHeader),
        InstanceMethod("get", &ResponseBinding::GetHeader),
        InstanceMethod("end", &ResponseBinding::End),
        InstanceMethod("type", &ResponseBinding::Type),
        InstanceMethod("cookie", &ResponseBinding::Cookie),
        InstanceMethod("clearCookie", &ResponseBinding::ClearCookie),
        InstanceMethod("redirect", &ResponseBinding::Redirect),
        InstanceAccessor("statusCode", &ResponseBinding::GetStatusCode, &ResponseBinding::SetStatusCode)
    });
    
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    
    exports.Set("Response", func);
    return exports;
}

Napi::Value ResponseBinding::NewInstance(Napi::Env env, Response* res) {
    Napi::EscapableHandleScope scope(env);
    Napi::Object obj = constructor.New({});
    
    ResponseBinding* binding = Napi::ObjectWrap<ResponseBinding>::Unwrap(obj);
    binding->SetResponse(res, false);  // Don't take ownership
    
    return scope.Escape(napi_value(obj)).ToObject();
}

ResponseBinding::ResponseBinding(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<ResponseBinding>(info), res_(nullptr), owns_response_(false) {
    // Constructor is mainly called internally
}

ResponseBinding::~ResponseBinding() {
    if (res_ && owns_response_) {
        // TODO: Add proper response cleanup when available in C API
        res_ = nullptr;
    }
}

void ResponseBinding::SetResponse(Response* res, bool owns) {
    res_ = res;
    owns_response_ = owns;
}

Napi::Value ResponseBinding::Send(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected data to send");
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    std::string data;
    if (info[0].IsString()) {
        data = ConvertJSStringToC(info[0]);
    } else if (info[0].IsBuffer()) {
        Napi::Buffer<char> buffer = info[0].As<Napi::Buffer<char>>();
        data = std::string(buffer.Data(), buffer.Length());
    } else {
        // Try to convert to string
        data = ConvertJSStringToC(info[0].ToString());
    }
    
    res_->send(res_, data.c_str());
    
    return info.This();
}

Napi::Value ResponseBinding::Json(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected object to send as JSON");
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    // Convert JavaScript object to JSON string
    Napi::Object global = env.Global();
    Napi::Object JSON = global.Get("JSON").As<Napi::Object>();
    Napi::Function stringify = JSON.Get("stringify").As<Napi::Function>();
    
    Napi::Value jsonString = stringify.Call(JSON, { info[0] });
    std::string jsonData = ConvertJSStringToC(jsonString);
    
    // Set Content-Type header
    res_->set_header(res_, "Content-Type", "application/json");
    res_->send(res_, jsonData.c_str());
    
    return info.This();
}

Napi::Value ResponseBinding::Status(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected status code");
    
    if (!info[0].IsNumber()) {
        ThrowCExpressError(env, "Status code must be a number");
        return env.Undefined();
    }
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    int statusCode = info[0].As<Napi::Number>().Int32Value();
    res_->status(res_, statusCode);
    
    return info.This();
}

Napi::Value ResponseBinding::SetHeader(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 2, "Expected header name and value");
    CEXPRESS_VALIDATE_STRING(info[0], "Header name must be a string");
    CEXPRESS_VALIDATE_STRING(info[1], "Header value must be a string");
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    std::string name = ConvertJSStringToC(info[0]);
    std::string value = ConvertJSStringToC(info[1]);
    
    res_->set_header(res_, name.c_str(), value.c_str());
    
    return info.This();
}

Napi::Value ResponseBinding::GetHeader(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected header name");
    CEXPRESS_VALIDATE_STRING(info[0], "Header name must be a string");
    
    if (!res_) {
        return env.Null();
    }
    
    std::string name = ConvertJSStringToC(info[0]);
    
    // Search through headers manually since there's no get_header method
    for (int i = 0; i < res_->header_count; i++) {
        if (strcmp(res_->headers[i].key, name.c_str()) == 0) {
            return ConvertCStringToJS(env, res_->headers[i].value);
        }
    }
    
    return env.Null();
}

Napi::Value ResponseBinding::End(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    if (info.Length() > 0) {
        // Send data and end
        std::string data = ConvertJSStringToC(info[0]);
        res_->send(res_, data.c_str());
    } else {
        // Just end the response - send empty string
        res_->send(res_, "");
    }
    
    return info.This();
}

Napi::Value ResponseBinding::Type(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected content type");
    CEXPRESS_VALIDATE_STRING(info[0], "Content type must be a string");
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    std::string contentType = ConvertJSStringToC(info[0]);
    
    // Add charset=utf-8 for text types if not present
    if (contentType.find("text/") == 0 && contentType.find("charset=") == std::string::npos) {
        contentType += "; charset=utf-8";
    }
    
    res_->set_header(res_, "Content-Type", contentType.c_str());
    
    return info.This();
}

Napi::Value ResponseBinding::Cookie(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // TODO: Implement cookie setting when available in C API
    ThrowCExpressError(env, "Cookie support not yet implemented");
    return info.This();
}

Napi::Value ResponseBinding::ClearCookie(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // TODO: Implement cookie clearing when available in C API
    ThrowCExpressError(env, "Cookie support not yet implemented");
    return info.This();
}

Napi::Value ResponseBinding::Redirect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected URL to redirect to");
    CEXPRESS_VALIDATE_STRING(info[0], "Redirect URL must be a string");
    
    if (!res_) {
        ThrowCExpressError(env, "Response object is not initialized");
        return env.Undefined();
    }
    
    std::string url = ConvertJSStringToC(info[0]);
    int statusCode = 302;  // Default redirect status
    
    if (info.Length() > 1 && info[1].IsNumber()) {
        statusCode = info[1].As<Napi::Number>().Int32Value();
    }
    
    res_->status(res_, statusCode);
    res_->set_header(res_, "Location", url.c_str());
    res_->send(res_, "");  // Send empty body for redirect
    
    return info.This();
}

Napi::Value ResponseBinding::GetStatusCode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!res_) {
        return Napi::Number::New(env, 200);  // Default status
    }
    
    // TODO: Add getter for status code in C Response structure
    return Napi::Number::New(env, 200);
}

void ResponseBinding::SetStatusCode(const Napi::CallbackInfo& info, const Napi::Value& value) {
    if (!value.IsNumber() || !res_) {
        return;
    }
    
    int statusCode = value.As<Napi::Number>().Int32Value();
    res_->status(res_, statusCode);
}

} // namespace CExpress
