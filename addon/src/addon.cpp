#include "common.h"
#include "../include/app_binding.h"
#include "../include/request_binding.h"
#include "../include/response_binding.h"
#include "../include/router_binding.h"

namespace CExpress {

// Module initialization function
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Initialize the C-Express core (if needed)
    // This would be where we set up any global C state
    
    // Export the main express function that creates an App
    exports.Set("createApp", Napi::Function::New(env, [](const Napi::CallbackInfo& info) -> Napi::Value {
        return AppBinding::NewInstance(info.Env());
    }));
    
    // Export the App constructor directly
    AppBinding::Init(env, exports);
    
    // Export Router constructor
    RouterBinding::Init(env, exports);
    
    // Export utility functions
    exports.Set("version", Napi::String::New(env, "1.0.0"));
    
    return exports;
}

} // namespace CExpress

// Node.js module registration
extern "C" {
    static napi_value init(napi_env env, napi_value exports) {
        Napi::Env napi_env(env);
        Napi::Object napi_exports = Napi::Object(napi_env, exports);
        return CExpress::Init(napi_env, napi_exports);
    }
    NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
}
