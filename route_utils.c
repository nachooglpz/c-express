// Helper function to duplicate a RouteMatch structure
RouteMatch* duplicate_route_match(const RouteMatch* original) {
    if (!original || !original->matched) return NULL;
    
    RouteMatch* copy = malloc(sizeof(RouteMatch));
    if (!copy) return NULL;
    
    copy->matched = original->matched;
    copy->param_count = original->param_count;
    
    // Duplicate parameters
    if (original->param_count > 0 && original->params) {
        copy->params = malloc(original->param_count * sizeof(RouteParam));
        if (!copy->params) {
            free(copy);
            return NULL;
        }
        
        for (int i = 0; i < original->param_count; i++) {
            copy->params[i].name = original->params[i].name ? strdup(original->params[i].name) : NULL;
            copy->params[i].value = original->params[i].value ? strdup(original->params[i].value) : NULL;
            copy->params[i].type = original->params[i].type;
            copy->params[i].is_optional = original->params[i].is_optional;
        }
    } else {
        copy->params = NULL;
    }
    
    // Duplicate wildcard path
    copy->wildcard_path = original->wildcard_path ? strdup(original->wildcard_path) : NULL;
    
    return copy;
}
