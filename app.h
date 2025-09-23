#ifndef APP_H
#define APP_H

#include "router.h"
#include "request.h"

// Forward declaration
typedef struct App App;

typedef struct App {
    Router router;
    void (*get)(App *, const char *path, Handler handler);
    void (*post)(App *, const char *path, Handler handler);
    void (*put)(App *, const char *path, Handler handler);
    void (*delete)(App *, const char *path, Handler handler);
    void (*patch)(App *, const char *path, Handler handler);
    void (*options)(App *, const char *path, Handler handler);
    void (*listen)(App *, int port);
    void (*use)(App*, Handler handler);
} App;

void express_init(int client_fd, void (*next)(void *), void *context);
void app_get(App *app, const char *path, Handler handler);
void app_post(App *app, const char *path, Handler handler);
void app_put(App *app, const char *path, Handler handler);
void app_delete(App *app, const char *path, Handler handler);
void app_patch(App *app, const char *path, Handler handler);
void app_options(App *app, const char *path, Handler handler);
void app_use(App *app, Handler handler);
void app_listen(App *app, int port);

App create_app();

#endif