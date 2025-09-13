#ifndef APP_H
#define APP_H

#include "router.h"

typedef struct {
    Router router;
    void (*get)(struct App *, const char *path, void (*handler)(int));
    void (*listen)(struct App *, int port);
} App;

void app_get(App *app, const char *path, void (*handler)(int));
void app_listen(App *app, int port);
App create_app();

#endif