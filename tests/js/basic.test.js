const { expect } = require('chai');
const express = require('../../lib');

describe('C-Express Basic Functionality', () => {
    describe('App Creation', () => {
        it('should create an app instance', () => {
            const app = express();
            expect(app).to.be.an('object');
            expect(app.get).to.be.a('function');
            expect(app.post).to.be.a('function');
            expect(app.listen).to.be.a('function');
        });
        
        it('should have HTTP method functions', () => {
            const app = express();
            expect(app.get).to.be.a('function');
            expect(app.post).to.be.a('function');
            expect(app.put).to.be.a('function');
            expect(app.delete).to.be.a('function');
            expect(app.patch).to.be.a('function');
            expect(app.options).to.be.a('function');
        });
        
        it('should have middleware and utility functions', () => {
            const app = express();
            expect(app.use).to.be.a('function');
            expect(app.mount).to.be.a('function');
            expect(app.error).to.be.a('function');
        });
        
        it('should have enhanced Express.js compatibility methods', () => {
            const app = express();
            expect(app.all).to.be.a('function');
            expect(app.param).to.be.a('function');
            expect(app.route).to.be.a('function');
            expect(app.toString).to.be.a('function');
            expect(app.version).to.be.a('string');
        });
        
        it('should have introspection capabilities', () => {
            const app = express();
            expect(app.getRoutes).to.be.a('function');
            expect(app.printRoutes).to.be.a('function');
            
            const routes = app.getRoutes();
            expect(routes).to.be.an('array');
        });
    });
    
    describe('Route Registration', () => {
        it('should register GET routes without error', () => {
            const app = express();
            expect(() => {
                app.get('/', (req, res) => {
                    res.send('Hello World');
                });
            }).to.not.throw();
        });
        
        it('should register POST routes without error', () => {
            const app = express();
            expect(() => {
                app.post('/users', (req, res) => {
                    res.json({ message: 'User created' });
                });
            }).to.not.throw();
        });
        
        it('should register middleware without error', () => {
            const app = express();
            expect(() => {
                app.use((req, res, next) => {
                    console.log('Middleware executed');
                    next();
                });
            }).to.not.throw();
        });
    });
    
    describe('Express Factory Function', () => {
        it('should export a function', () => {
            expect(express).to.be.a('function');
        });
        
        it('should have static properties', () => {
            expect(express.App).to.be.a('function');
            expect(express.Router).to.be.a('function');
            expect(express.version).to.be.a('string');
            expect(express.name).to.equal('c-express');
        });
        
        it('should provide metadata and debug information', () => {
            expect(express.info).to.be.a('function');
            expect(express.debug).to.be.an('object');
            
            const info = express.info();
            expect(info).to.have.property('name', 'c-express');
            expect(info).to.have.property('version');
            expect(info).to.have.property('features');
            expect(info.features).to.be.an('array');
            
            expect(express.debug.isNativeAddon).to.be.true;
            expect(express.debug.buildInfo).to.be.an('object');
        });
    });
    
    describe('Router Creation', () => {
        it('should create router instances', () => {
            const router = express.Router();
            expect(router).to.be.an('object');
            expect(router.get).to.be.a('function');
            expect(router.post).to.be.a('function');
            expect(router.use).to.be.a('function');
        });
    });
    
    describe('Advanced Functionality', () => {
        it('should support method chaining', () => {
            const app = express();
            
            const result = app
                .get('/', (req, res) => res.send('Hello'))
                .post('/users', (req, res) => res.json({}))
                .use((req, res, next) => next());
                
            expect(result).to.equal(app);
        });
        
        it('should support route building pattern', () => {
            const app = express();
            const route = app.route('/users');
            
            expect(route).to.be.an('object');
            expect(route.get).to.be.a('function');
            expect(route.post).to.be.a('function');
            expect(route.put).to.be.a('function');
            expect(route.delete).to.be.a('function');
        });
        
        it('should track registered handlers', () => {
            const app = express();
            
            // Initial state
            let routes = app.getRoutes();
            const initialCount = routes.length;
            
            // Register some routes
            app.get('/', (req, res) => res.send('Hello'));
            app.post('/users', (req, res) => res.json({}));
            app.use((req, res, next) => next());
            
            // Check updated state
            routes = app.getRoutes();
            expect(routes.length).to.be.greaterThan(initialCount);
        });
        
        it('should provide useful toString representation', () => {
            const app = express();
            app.get('/', (req, res) => res.send('Hello'));
            
            const str = app.toString();
            expect(str).to.be.a('string');
            expect(str).to.include('C-Express App');
            expect(str).to.include('active');
        });
    });
});
