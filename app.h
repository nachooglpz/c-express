#ifndef APP_H
#define APP_H

#include "router.h"
#include "request.h"
#include "response.h"
#include "error.h"

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

#endif