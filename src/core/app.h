#ifndef APP_H
#define APP_H

#include "router.h"
#include "../http/request.h"
#include "../http/response.h"
#include "../http/error.h"
#include "../http/negotiation.h"

// Forward declaration for self-referencing pointers
struct App;

// Error handler function type
typedef void (*ErrorHandler)(Error *error, int client_fd, void *context);

struct App {
    struct Router router;
    ErrorHandler error_handler;  // Global error handler
    void (*get)(struct App *, const char *path, Handler handler);
    void (*post)(struct App *, const char *path, Handler handler);
    void (*put)(struct App *, const char *path, Handler handler);
    void (*delete)(struct App *, const char *path, Handler handler);
    void (*patch)(struct App *, const char *path, Handler handler);
    void (*options)(struct App *, const char *path, Handler handler);
    void (*listen)(struct App *, int port);
    void (*use)(struct App*, Handler handler);
    void (*mount)(struct App*, const char *prefix, struct Router *router);  // Mount sub-router
    void (*error)(struct App*, ErrorHandler handler);  // Set error handler
};

typedef struct App App;

void express_init(int client_fd, void (*next)(void *), void *context);
void app_get(struct App *app, const char *path, Handler handler);
void app_post(struct App *app, const char *path, Handler handler);
void app_put(struct App *app, const char *path, Handler handler);
void app_delete(struct App *app, const char *path, Handler handler);
void app_patch(struct App *app, const char *path, Handler handler);
void app_options(struct App *app, const char *path, Handler handler);
void app_use(struct App *app, Handler handler);
void app_mount(struct App *app, const char *prefix, Router *router);
void app_error(struct App *app, ErrorHandler handler);
void app_listen(struct App *app, int port);
void app_handle_request(struct App *app, const char *method, const char *path, int client_fd, Request *req);

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
void app_get_with_metadata(struct App *app, const char *path, Handler handler, const RouteConfig *config);
void app_post_with_metadata(struct App *app, const char *path, Handler handler, const RouteConfig *config);
void app_put_with_metadata(struct App *app, const char *path, Handler handler, const RouteConfig *config);
void app_delete_with_metadata(struct App *app, const char *path, Handler handler, const RouteConfig *config);
void app_patch_with_metadata(struct App *app, const char *path, Handler handler, const RouteConfig *config);
void app_options_with_metadata(struct App *app, const char *path, Handler handler, const RouteConfig *config);

// Documentation and introspection functions
void app_print_routes(struct App *app);
char* app_generate_openapi_doc(struct App *app);
Route** app_get_routes_by_tag(struct App *app, const char *tag, int *count);
Route* app_get_route_by_id(struct App *app, const char *route_id);

#endif