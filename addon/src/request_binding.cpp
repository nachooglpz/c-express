#include "request_binding.h"
#include <cstring>

namespace CExpress {

Napi::FunctionReference RequestBinding::constructor;

Napi::Object RequestBinding::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    
    Napi::Function func = DefineClass(env, "Request", {
        InstanceAccessor("method", &RequestBinding::GetMethod, nullptr),
        InstanceAccessor("path", &RequestBinding::GetPath, nullptr),
        InstanceAccessor("query", &RequestBinding::GetQuery, nullptr),
        InstanceAccessor("params", &RequestBinding::GetParams, nullptr),
        InstanceAccessor("headers", &RequestBinding::GetHeaders, nullptr),
        InstanceAccessor("body", &RequestBinding::GetBody, nullptr),
        InstanceMethod("header", &RequestBinding::GetHeader),
        InstanceMethod("param", &RequestBinding::GetParam),
        InstanceMethod("query", &RequestBinding::GetQueryParam)
    });
    
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    
    exports.Set("Request", func);
    return exports;
}

Napi::Value RequestBinding::NewInstance(Napi::Env env, Request* req) {
    Napi::EscapableHandleScope scope(env);
    Napi::Object obj = constructor.New({});
    
    RequestBinding* binding = Napi::ObjectWrap<RequestBinding>::Unwrap(obj);
    binding->SetRequest(req, false);  // Don't take ownership
    
    return scope.Escape(napi_value(obj)).ToObject();
}

RequestBinding::RequestBinding(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<RequestBinding>(info), req_(nullptr), owns_request_(false) {
    // Constructor is mainly called internally
}

RequestBinding::~RequestBinding() {
    if (req_ && owns_request_) {
        // TODO: Add proper request cleanup when available in C API
        req_ = nullptr;
    }
}

void RequestBinding::SetRequest(Request* req, bool owns) {
    req_ = req;
    owns_request_ = owns;
}

Napi::Value RequestBinding::GetMethod(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!req_) {
        return env.Null();
    }
    
    return ConvertCStringToJS(env, req_->method);
}

Napi::Value RequestBinding::GetPath(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!req_) {
        return env.Null();
    }
    
    return ConvertCStringToJS(env, req_->path);
}

Napi::Value RequestBinding::GetQuery(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!req_) {
        return Napi::Object::New(env);
    }
    
    Napi::Object queryObj = Napi::Object::New(env);
    
    for (int i = 0; i < req_->query_count; i++) {
        const char* key = req_->query[i].key;
        const char* value = req_->query[i].value;
        
        if (key && value && strlen(key) > 0) {
            queryObj.Set(key, value);
        }
    }
    
    return queryObj;
}

Napi::Value RequestBinding::GetParams(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!req_) {
        return Napi::Object::New(env);
    }
    
    Napi::Object paramsObj = Napi::Object::New(env);
    
    for (int i = 0; i < req_->param_count; i++) {
        const char* key = req_->params[i].key;
        const char* value = req_->params[i].value;
        
        if (key && value && strlen(key) > 0) {
            paramsObj.Set(key, value);
        }
    }
    
    return paramsObj;
}

Napi::Value RequestBinding::GetHeaders(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!req_) {
        return Napi::Object::New(env);
    }
    
    Napi::Object headersObj = Napi::Object::New(env);
    
    // Safely iterate through headers with bounds checking
    int maxHeaders = req_->header_count < MAX_HEADERS ? req_->header_count : MAX_HEADERS;
    
    for (int i = 0; i < maxHeaders; i++) {
        const char* key = req_->headers[i].key;
        const char* value = req_->headers[i].value;
        
        if (key && value && strlen(key) > 0 && strlen(value) > 0) {
            try {
                headersObj.Set(key, value);
            } catch (...) {
                // Skip invalid headers rather than failing completely
                continue;
            }
        }
    }
    
    return headersObj;
}

Napi::Value RequestBinding::GetBody(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!req_ || strlen(req_->body) == 0) {
        return env.Null();
    }
    
    return ConvertCStringToJS(env, req_->body);
}

Napi::Value RequestBinding::GetHeader(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected header name");
    CEXPRESS_VALIDATE_STRING(info[0], "Header name must be a string");
    
    if (!req_) {
        return env.Null();
    }
    
    std::string headerName = ConvertJSStringToC(info[0]);
    const char* headerValue = req_->get_header(req_, headerName.c_str());
    
    return ConvertCStringToJS(env, headerValue);
}

Napi::Value RequestBinding::GetParam(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected parameter name");
    CEXPRESS_VALIDATE_STRING(info[0], "Parameter name must be a string");
    
    if (!req_) {
        return env.Null();
    }
    
    std::string paramName = ConvertJSStringToC(info[0]);
    const char* paramValue = req_->get_param(req_, paramName.c_str());
    
    return ConvertCStringToJS(env, paramValue);
}

Napi::Value RequestBinding::GetQueryParam(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    CEXPRESS_VALIDATE_ARGS(info, 1, "Expected query parameter name");
    CEXPRESS_VALIDATE_STRING(info[0], "Query parameter name must be a string");
    
    if (!req_) {
        return env.Null();
    }
    
    std::string queryName = ConvertJSStringToC(info[0]);
    const char* queryValue = req_->get_query(req_, queryName.c_str());
    
    return ConvertCStringToJS(env, queryValue);
}

} // namespace CExpress
