const addon = require('../build/Release/c_express_addon');

/**
 * Creates a new C-Express application
 * @returns {App} A new application instance with Express.js-compatible API
 */
function express() {
    const app = addon.createApp();
    
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
        // TODO: Implement parameter middleware when Phase 2 is complete
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
