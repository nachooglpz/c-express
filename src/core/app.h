#ifndef APP_H
#define APP_H

#include "router.h"
#include "../http/request.h"
#include "../http/response.h"
#include "../http/error.h"

// Forward declaration
typedef struct App App;

// Error handler function type
typedef void (*ErrorHandler)(Error *error, int client_fd, void *context);

typedef struct App {
    Router router;
    ErrorHandler error_handler;  // Global error handler
    void (*get)(App *, const char *path, Handler handler);
    void (*post)(App *, const char *path, Handler handler);
    void (*put)(App *, const char *path, Handler handler);
    void (*delete)(App *, const char *path, Handler handler);
    void (*patch)(App *, const char *path, Handler handler);
    void (*options)(App *, const char *path, Handler handler);
    void (*listen)(App *, int port);
    void (*use)(App*, Handler handler);
    void (*mount)(App*, const char *prefix, Router *router);  // Mount sub-router
    void (*error)(App*, ErrorHandler handler);  // Set error handler
} App;

void express_init(int client_fd, void (*next)(void *), void *context);
void app_get(App *app, const char *path, Handler handler);
void app_post(App *app, const char *path, Handler handler);
void app_put(App *app, const char *path, Handler handler);
void app_delete(App *app, const char *path, Handler handler);
void app_patch(App *app, const char *path, Handler handler);
void app_options(App *app, const char *path, Handler handler);
void app_use(App *app, Handler handler);
void app_mount(App *app, const char *prefix, Router *router);
void app_error(App *app, ErrorHandler handler);
void app_listen(App *app, int port);
void app_handle_request(App *app, const char *method, const char *path, int client_fd, Request *req);

App create_app();

// Enhanced route registration with metadata support
typedef struct {
    const char *route_id;
    const char *summary;
    const char *description;
    const char **tags;
    int deprecated;
    const char *deprecated_reason;
} RouteConfig;

// Enhanced route registration functions
void app_get_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config);
void app_post_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config);
void app_put_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config);
void app_delete_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config);
void app_patch_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config);
void app_options_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config);

// Documentation and introspection functions
void app_print_routes(App *app);
char* app_generate_openapi_doc(App *app);
Route** app_get_routes_by_tag(App *app, const char *tag, int *count);
Route* app_get_route_by_id(App *app, const char *route_id);

#endif