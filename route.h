#ifndef ROUTE_H
#define ROUTE_H

#include "layer.h"

typedef struct {
    const char *path;
    Layer layer;
} Route;

#endif