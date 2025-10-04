const { describe, it } = require('mocha');
const { expect } = require('chai');
const http = require('http');
const express = require('../../lib');

describe('HTTP Server Integration', function() {
    this.timeout(10000); // Longer timeout for server tests
    
    let app;
    let server;
    
    afterEach(function(done) {
        if (server) {
            server.close(done);
        } else {
            done();
        }
    });
    
    describe('HTTP Server Bridge', function() {
        it('should create HTTP server bridge', function() {
            app = express();
            expect(app._httpBridge).to.exist;
            expect(typeof app._httpBridge.listen).to.equal('function');
        });
        
        it('should start HTTP server and handle basic requests', function(done) {
            app = express();
            
            // Register a simple route
            app.get('/', (req, res) => {
                res.json({ message: 'Hello from HTTP server integration!', framework: 'c-express' });
            });
            
            // Start server
            server = app.listen(3001, () => {
                // Make request to test the route
                http.get('http://localhost:3001/', (res) => {
                    let data = '';
                    res.on('data', chunk => data += chunk);
                    res.on('end', () => {
                        try {
                            const response = JSON.parse(data);
                            expect(response.message).to.equal('Hello from HTTP server integration!');
                            expect(response.framework).to.equal('c-express');
                            done();
                        } catch (error) {
                            done(error);
                        }
                    });
                }).on('error', done);
            });
        });
        
        it('should handle POST requests with JSON body', function(done) {
            app = express();
            
            app.post('/users', (req, res) => {
                res.json({
                    message: 'User created',
                    received: req.body,
                    method: req.method
                });
            });
            
            server = app.listen(3002, () => {
                const postData = JSON.stringify({ name: 'John', age: 30 });
                
                const options = {
                    hostname: 'localhost',
                    port: 3002,
                    path: '/users',
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'Content-Length': Buffer.byteLength(postData)
                    }
                };
                
                const req = http.request(options, (res) => {
                    let data = '';
                    res.on('data', chunk => data += chunk);
                    res.on('end', () => {
                        try {
                            const response = JSON.parse(data);
                            expect(response.message).to.equal('User created');
                            expect(response.received.name).to.equal('John');
                            expect(response.method).to.equal('POST');
                            done();
                        } catch (error) {
                            done(error);
                        }
                    });
                });
                
                req.on('error', done);
                req.write(postData);
                req.end();
            });
        });
        
        it('should handle route parameters', function(done) {
            app = express();
            
            app.get('/users/:id', (req, res) => {
                res.json({
                    userId: req.params.id,
                    path: req.path,
                    url: req.url
                });
            });
            
            server = app.listen(3003, () => {
                http.get('http://localhost:3003/users/123', (res) => {
                    let data = '';
                    res.on('data', chunk => data += chunk);
                    res.on('end', () => {
                        try {
                            const response = JSON.parse(data);
                            expect(response.userId).to.equal('123');
                            expect(response.path).to.equal('/users/123');
                            done();
                        } catch (error) {
                            done(error);
                        }
                    });
                }).on('error', done);
            });
        });
        
        it('should handle 404 for unmatched routes', function(done) {
            app = express();
            
            app.get('/existing', (req, res) => {
                res.json({ message: 'Found' });
            });
            
            server = app.listen(3004, () => {
                http.get('http://localhost:3004/nonexistent', (res) => {
                    expect(res.statusCode).to.equal(404);
                    
                    let data = '';
                    res.on('data', chunk => data += chunk);
                    res.on('end', () => {
                        try {
                            const response = JSON.parse(data);
                            expect(response.error).to.equal('Not Found');
                            done();
                        } catch (error) {
                            done(error);
                        }
                    });
                }).on('error', done);
            });
        });
        
        it('should handle middleware chain', function(done) {
            app = express();
            
            // Global middleware
            app.use((req, res, next) => {
                req.customProperty = 'middleware-value';
                next();
            });
            
            app.get('/middleware-test', (req, res) => {
                res.json({
                    customProperty: req.customProperty,
                    message: 'Middleware working'
                });
            });
            
            server = app.listen(3005, () => {
                http.get('http://localhost:3005/middleware-test', (res) => {
                    let data = '';
                    res.on('data', chunk => data += chunk);
                    res.on('end', () => {
                        try {
                            const response = JSON.parse(data);
                            expect(response.customProperty).to.equal('middleware-value');
                            expect(response.message).to.equal('Middleware working');
                            done();
                        } catch (error) {
                            done(error);
                        }
                    });
                }).on('error', done);
            });
        });
    });
    
    describe('Express.js Compatibility', function() {
        it('should support Express.js response methods', function(done) {
            app = express();
            
            app.get('/status', (req, res) => {
                res.status(201).json({ created: true });
            });
            
            app.get('/redirect', (req, res) => {
                res.redirect('/status');
            });
            
            app.get('/cookie', (req, res) => {
                res.cookie('test', 'value').send('Cookie set');
            });
            
            server = app.listen(3006, () => {
                // Test status method
                http.get('http://localhost:3006/status', (res) => {
                    expect(res.statusCode).to.equal(201);
                    done();
                }).on('error', done);
            });
        });
    });
});