#ifndef APP_H
#define APP_H

#include "router.h"

typedef struct {
    Router router;
    void (*get)(struct App *, const char *path, Handler handler);
    void (*post)(struct App *, const char *path, Handler handler);
    void (*listen)(struct App *, int port);
    void (*use)(struct App*, Handler handler);
} App;

void express_init(int client_fd, void (*next)(void *), void *context);
void app_get(App *app, const char *path, Handler handler);
void app_post(App *app, const char *path, Handler handler);
void app_use(App *app, Handler handler);
void app_listen(App *app, int port);

App create_app();

#endif