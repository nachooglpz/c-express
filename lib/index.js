const addon = require('../build/Release/c_express_addon');
const HTTPServerBridge = require('./http-bridge');

// Track callback registry
const callbackRegistry = new Map();
let nextCallbackId = 1;

/**
 * Creates a new C-Express application
 * @returns {App} A new application instance with Express.js-compatible API
 */
function express() {
    const app = addon.createApp();
    
    // Create HTTP server bridge
    const httpBridge = new HTTPServerBridge(app, addon);
    
    // Store reference to bridge and callback registry
    app._httpBridge = httpBridge;
    app._callbackRegistry = callbackRegistry;
    
    // Intercept route registration to store JavaScript callbacks
    const originalMethods = ['get', 'post', 'put', 'delete', 'patch', 'options', 'head', 'use'];
    
    originalMethods.forEach(method => {
        const originalMethod = app[method];
        if (originalMethod) {
            Object.defineProperty(app, method, {
                value: function(path, handler) {
                    // For 'use' method, path is optional
                    if (method === 'use' && typeof path === 'function') {
                        handler = path;
                        path = '*'; // Match all paths for middleware
                    }
                    
                    // Generate route ID for callback storage
                    const routeId = `${method.toUpperCase()}::${path}::${Date.now()}::${Math.random()}`;
                    
                    console.log(`Registering route: ${method.toUpperCase()} ${path} (ID: ${routeId})`);
                    
                    // Store the JavaScript callback
                    httpBridge.registerCallback(routeId, handler);
                    
                    // For this phase, skip the native method call and just return this
                    // The HTTP bridge handles all routing
                    return this;
                },
                writable: true,
                enumerable: false,
                configurable: true
            });
        }
    });
    
    // Add Express.js compatibility methods
    app.all = function(path, ...handlers) {
        const methods = ['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS', 'HEAD'];
        handlers.forEach(handler => {
            methods.forEach(method => {
                this[method.toLowerCase()](path, handler);
            });
        });
        return this;
    };
    
    // Add route parameter method for better compatibility
    app.param = function(name, handler) {
        // TODO: Implement parameter middleware
        console.warn('app.param() not yet implemented in C-Express');
        return this;
    };
    
    // Add route method for dynamic HTTP method routing
    app.route = function(path) {
        const route = {
            get: (handler) => { this.get(path, handler); return route; },
            post: (handler) => { this.post(path, handler); return route; },
            put: (handler) => { this.put(path, handler); return route; },
            delete: (handler) => { this.delete(path, handler); return route; },
            patch: (handler) => { this.patch(path, handler); return route; },
            options: (handler) => { this.options(path, handler); return route; },
        };
        return route;
    };
    
    // Override listen method to use HTTP bridge
    // Note: listen is on the prototype, so we need to override it as a property
    const originalListen = app.listen;
    Object.defineProperty(app, 'listen', {
        value: function(port, callback) {
            console.log(`listen() called with port ${port}`);
            
            // The JavaScript callbacks are already stored in the HTTP bridge
            // No need to extract them from the native addon
            
            console.log(`Starting C-Express server with HTTP bridge`);
            console.log(`Registered ${httpBridge.callbacks.size} route callbacks`);
            
            // Start HTTP server using Node.js HTTP server with C-Express routing
            return httpBridge.listen(port, callback);
        },
        writable: true,
        enumerable: false,
        configurable: true
    });
    
    return app;
}

// Export the App and Router classes
express.App = addon.App;
express.Router = () => new addon.Router();

// Export utility functions
express.static = (root, options = {}) => {
    // TODO: Implement static file serving middleware
    return (req, res, next) => {
        next(new Error('Static file serving not yet implemented'));
    };
};

express.json = (options = {}) => {
    // TODO: Implement JSON body parser middleware
    return (req, res, next) => {
        next();
    };
};

express.urlencoded = (options = {}) => {
    // TODO: Implement URL-encoded body parser middleware
    return (req, res, next) => {
        next();
    };
};

// Export version and metadata
express.version = addon.version;

// Override the function name property
Object.defineProperty(express, 'name', {
    value: 'c-express',
    writable: false,
    enumerable: false,
    configurable: true
});

// Add utility functions for introspection
express.info = () => ({
    name: 'c-express',
    version: addon.version,
    description: 'Fast C-powered Express.js alternative',
    features: [
        'Express.js compatible API',
        'Native C performance',
        'Route pattern matching',
        'Middleware support',
        'JSON and form parsing'
    ]
});

// Development helpers
express.debug = {
    isNativeAddon: true,
    buildInfo: {
        compiler: 'gcc/g++',
        nodeVersion: process.version,
        platform: process.platform,
        arch: process.arch
    }
};

module.exports = express;
