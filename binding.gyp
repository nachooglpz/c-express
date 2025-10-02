{
  "targets": [
    {
      "target_name": "c_express_addon",
      "sources": [
        "addon/src/addon.cpp",
        "addon/src/app_binding.cpp",
        "addon/src/request_binding.cpp",
        "addon/src/response_binding.cpp",
        "addon/src/router_binding.cpp",
        "addon/src/utils.cpp",
        "src/core/app.c",
        "src/core/router.c",
        "src/core/route.c",
        "src/core/layer.c",
        "src/http/request.c",
        "src/http/response.c",
        "src/http/error.c",
        "src/http/negotiation.c",
        "src/http/streaming.c",
        "src/parsers/json.c",
        "src/parsers/form.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "src/",
        "addon/include/"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags": [
        "-std=c99",
        "-Wall",
        "-Wextra",
        "-O3",
        "-DNAPI_VERSION=6",
        "-fPIC"
      ],
      "cflags_cc": [
        "-std=c++17",
        "-Wall",
        "-Wextra", 
        "-O3",
        "-DNAPI_VERSION=6",
        "-fPIC"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "NODE_ADDON_API_DISABLE_DEPRECATED"
      ],
      "conditions": [
        ['OS=="win"', {
          "defines": [
            "_WIN32_WINNT=0x0600"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }],
        ['OS=="mac"', {
          "cflags": [
            "-stdlib=libc++"
          ],
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.15"
          }
        }],
        ['OS=="linux"', {
          "cflags": [
            "-fPIC"
          ],
          "cflags_cc!": [ "-std=c99" ]
        }]
      ]
    }
  ]
}
