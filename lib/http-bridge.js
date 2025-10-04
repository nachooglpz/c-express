// HTTP Server Integration
// This module provides the bridge between Node.js HTTP server and C-Express framework

const http = require('http');
const url = require('url');
const querystring = require('querystring');

class HTTPServerBridge {
    constructor(cExpressApp, nativeBinding) {
        this.app = cExpressApp;
        this.nativeBinding = nativeBinding;
        this.server = null;
        this.callbacks = new Map(); // Store JavaScript callbacks by route ID
    }

    /**
     * Create and start HTTP server that integrates with C-Express
     */
    listen(port, callback) {
        this.server = http.createServer((req, res) => {
            this.handleRequest(req, res);
        });

        this.server.listen(port, () => {
            if (callback) {
                callback();
            }
        });

        return this.server;
    }

    /**
     * Handle incoming HTTP request by bridging to C framework
     */
    async handleRequest(nodeReq, nodeRes) {
        try {
            // Parse URL and extract components
            const parsedUrl = url.parse(nodeReq.url, true);
            const pathname = parsedUrl.pathname;
            const query = parsedUrl.query;
            const method = nodeReq.method.toUpperCase();

            // Create C-Express compatible request object
            const cExpressReq = this.createCExpressRequest(nodeReq, pathname, query, method);
            
            // Create C-Express compatible response object  
            const cExpressRes = this.createCExpressResponse(nodeRes);

            // Parse request body if present
            await this.parseRequestBody(nodeReq, cExpressReq);

            // Route the request through C framework and execute JavaScript callbacks
            await this.routeRequest(method, pathname, cExpressReq, cExpressRes);

        } catch (error) {
            console.error('HTTP request handling error:', error);
            if (!nodeRes.headersSent) {
                nodeRes.statusCode = 500;
                nodeRes.setHeader('Content-Type', 'application/json');
                nodeRes.end(JSON.stringify({
                    error: 'Internal Server Error',
                    message: error.message,
                    framework: 'c-express'
                }));
            }
        }
    }

    /**
     * Create C-Express compatible request object from Node.js request
     */
    createCExpressRequest(nodeReq, pathname, query, method) {
        const cExpressReq = {
            method: method,
            url: nodeReq.url,
            path: pathname,
            query: query,
            headers: nodeReq.headers,
            params: {}, // Will be populated by route matching
            body: null, // Will be populated by body parsing
            ip: nodeReq.socket.remoteAddress,
            userAgent: nodeReq.headers['user-agent'],
            
            // Express.js compatibility methods
            get: (headerName) => nodeReq.headers[headerName.toLowerCase()],
            header: (headerName) => nodeReq.headers[headerName.toLowerCase()],
            param: (name) => cExpressReq.params[name] || cExpressReq.query[name],
            
            // Internal properties
            _nodeReq: nodeReq,
            _cExpressApp: this.app
        };

        return cExpressReq;
    }

    /**
     * Create C-Express compatible response object from Node.js response
     */
    createCExpressResponse(nodeRes) {
        const cExpressRes = {
            statusCode: 200,
            
            // Express.js compatible methods
            status: (code) => {
                cExpressRes.statusCode = code;
                nodeRes.statusCode = code;
                return cExpressRes;
            },
            
            setHeader: (name, value) => {
                nodeRes.setHeader(name, value);
                return cExpressRes;
            },
            
            getHeader: (name) => {
                return nodeRes.getHeader(name);
            },
            
            send: (data) => {
                if (!nodeRes.headersSent) {
                    if (typeof data === 'string') {
                        nodeRes.setHeader('Content-Type', 'text/html');
                    } else if (typeof data === 'object') {
                        nodeRes.setHeader('Content-Type', 'application/json');
                        data = JSON.stringify(data);
                    }
                    nodeRes.end(data);
                }
            },
            
            json: (data) => {
                if (!nodeRes.headersSent) {
                    nodeRes.setHeader('Content-Type', 'application/json');
                    nodeRes.end(JSON.stringify(data));
                }
            },
            
            redirect: (statusOrUrl, url) => {
                let redirectUrl, statusCode;
                if (typeof statusOrUrl === 'string') {
                    redirectUrl = statusOrUrl;
                    statusCode = 302;
                } else {
                    statusCode = statusOrUrl;
                    redirectUrl = url;
                }
                
                nodeRes.statusCode = statusCode;
                nodeRes.setHeader('Location', redirectUrl);
                nodeRes.end();
            },
            
            cookie: (name, value, options = {}) => {
                let cookieStr = `${name}=${value}`;
                if (options.maxAge) cookieStr += `; Max-Age=${options.maxAge}`;
                if (options.httpOnly) cookieStr += '; HttpOnly';
                if (options.secure) cookieStr += '; Secure';
                if (options.path) cookieStr += `; Path=${options.path}`;
                
                const existingCookies = nodeRes.getHeader('Set-Cookie') || [];
                const cookies = Array.isArray(existingCookies) ? existingCookies : [existingCookies];
                cookies.push(cookieStr);
                nodeRes.setHeader('Set-Cookie', cookies);
                return cExpressRes;
            },
            
            end: (data) => {
                if (!nodeRes.headersSent) {
                    nodeRes.end(data);
                }
            },
            
            // Internal properties
            _nodeRes: nodeRes,
            _cExpressApp: this.app
        };

        return cExpressRes;
    }

    /**
     * Parse request body for POST/PUT/PATCH requests
     */
    async parseRequestBody(nodeReq, cExpressReq) {
        return new Promise((resolve) => {
            if (['POST', 'PUT', 'PATCH'].includes(cExpressReq.method)) {
                let body = '';
                
                nodeReq.on('data', (chunk) => {
                    body += chunk.toString();
                });
                
                nodeReq.on('end', () => {
                    try {
                        const contentType = nodeReq.headers['content-type'] || '';
                        
                        if (contentType.includes('application/json')) {
                            cExpressReq.body = JSON.parse(body);
                        } else if (contentType.includes('application/x-www-form-urlencoded')) {
                            cExpressReq.body = querystring.parse(body);
                        } else {
                            cExpressReq.body = body;
                        }
                    } catch (error) {
                        console.error('Body parsing error:', error);
                        cExpressReq.body = body; // Fallback to raw body
                    }
                    resolve();
                });
                
                // Timeout for body parsing
                setTimeout(() => {
                    resolve();
                }, 30000); // 30 second timeout
                
            } else {
                resolve();
            }
        });
    }

    /**
     * Route request through C framework and execute JavaScript callbacks
     */
    async routeRequest(method, pathname, req, res) {
        // Get matched routes from C framework (this will be implemented)
        const matchedRoutes = this.getMatchedRoutes(method, pathname);
        
        if (matchedRoutes.length === 0) {
            // No routes matched - send 404
            res.status(404).json({
                error: 'Not Found',
                message: `Cannot ${method} ${pathname}`,
                framework: 'c-express'
            });
            return;
        }

        // Execute middleware chain
        await this.executeMiddlewareChain(matchedRoutes, req, res);
    }

    /**
     * Get matched routes from C framework 
     */
    getMatchedRoutes(method, pathname) {
        // For now, we'll use a simple JavaScript-based routing
        // that works with the stored callbacks
        const matchedRoutes = [];
        
        this.callbacks.forEach((callback, routeId) => {
            // Parse route ID to get method and pattern
            const parts = routeId.split('::');
            if (parts.length >= 2) {
                const routeMethod = parts[0];
                const routePattern = parts[1];
                
                if (this.routeMatches(routeMethod, routePattern, method, pathname)) {
                    matchedRoutes.push({
                        id: routeId,
                        method: routeMethod,
                        pattern: routePattern,
                        callback: callback
                    });
                }
            }
        });
        
        return matchedRoutes;
    }
    
    /**
     * Check if a route matches the current request
     */
    routeMatches(routeMethod, routePattern, requestMethod, requestPath) {
        // Check method match (or if it's middleware with USE)
        if (routeMethod !== 'USE' && routeMethod !== requestMethod) {
            return false;
        }
        
        // For middleware (USE), always match
        if (routeMethod === 'USE') {
            return true;
        }
        
        // Simple pattern matching for now
        if (routePattern === requestPath) {
            return true;
        }
        
        // Check for parameter routes (/users/:id)
        const paramRegex = new RegExp(
            '^' + routePattern.replace(/:([^/]+)/g, '([^/]+)') + '$'
        );
        
        return paramRegex.test(requestPath);
    }

    /**
     * Execute middleware and route handlers in order
     */
    async executeMiddlewareChain(routes, req, res) {
        let currentIndex = 0;
        
        const next = async (error) => {
            if (error) {
                // Handle error - call error middleware
                this.handleError(error, req, res);
                return;
            }
            
            if (currentIndex >= routes.length) {
                // All middleware executed, check if response was sent
                if (!res._nodeRes.headersSent) {
                    // No route handled the request - this should not happen as we check for matches
                    res.status(404).json({
                        error: 'Not Found',
                        message: `Cannot ${req.method} ${req.path}`,
                        framework: 'c-express'
                    });
                }
                return;
            }
            
            const route = routes[currentIndex++];
            const callback = route.callback;
            
            if (callback && typeof callback === 'function') {
                try {
                    // Extract route parameters
                    req.params = this.extractParams(route.pattern, req.path);
                    
                    // Call the JavaScript callback
                    if (callback.constructor.name === 'AsyncFunction') {
                        await callback(req, res, next);
                    } else {
                        // For non-async functions, we need to handle the next() call properly
                        const result = callback(req, res, next);
                        if (result && typeof result.then === 'function') {
                            await result;
                        }
                    }
                } catch (error) {
                    next(error);
                }
            } else {
                // Skip this route if no callback
                next();
            }
        };
        
        // Start middleware chain
        await next();
    }

    /**
     * Extract route parameters from path pattern
     */
    extractParams(pattern, path) {
        const params = {};
        
        // Simple parameter extraction (will be enhanced)
        // Convert /users/:id to regex and extract params
        const paramRegex = /:([^/]+)/g;
        const paramNames = [];
        let match;
        
        while ((match = paramRegex.exec(pattern)) !== null) {
            paramNames.push(match[1]);
        }
        
        if (paramNames.length > 0) {
            const patternRegex = new RegExp(
                '^' + pattern.replace(/:([^/]+)/g, '([^/]+)') + '$'
            );
            const pathMatch = path.match(patternRegex);
            
            if (pathMatch) {
                paramNames.forEach((name, index) => {
                    params[name] = pathMatch[index + 1];
                });
            }
        }
        
        return params;
    }

    /**
     * Handle errors in middleware chain
     */
    handleError(error, req, res) {
        console.error('Middleware error:', error);
        
        if (!res._nodeRes.headersSent) {
            res.status(500).json({
                error: 'Internal Server Error',
                message: error.message,
                stack: process.env.NODE_ENV === 'development' ? error.stack : undefined,
                framework: 'c-express'
            });
        }
    }

    /**
     * Register JavaScript callback for route
     */
    registerCallback(routeId, callback) {
        this.callbacks.set(routeId, callback);
    }

    /**
     * Get server instance
     */
    getServer() {
        return this.server;
    }

    /**
     * Close the server
     */
    close(callback) {
        if (this.server) {
            this.server.close(callback);
        }
    }
}

module.exports = HTTPServerBridge;