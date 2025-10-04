const express = require('../lib');

console.log('C-Express Middleware Chain Example');
console.log('====================================');

const app = express();

// Global error handler (for demo)
process.on('unhandledRejection', (err) => {
    console.error('Unhandled rejection:', err);
});

// Logger middleware
const logger = (req, res, next) => {
    const start = Date.now();
    console.log(`→ ${req.method} ${req.path} [${new Date().toISOString()}]`);
    
    // Mock response end to log completion time
    const originalEnd = res.end;
    res.end = function(...args) {
        const duration = Date.now() - start;
        console.log(`← ${res.statusCode} ${req.method} ${req.path} (${duration}ms)`);
        originalEnd.apply(this, args);
    };
    
    next();
};

// CORS middleware
const cors = (req, res, next) => {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization');
    next();
};

// Request ID middleware
const requestId = (req, res, next) => {
    req.id = Math.random().toString(36).substr(2, 9);
    res.setHeader('X-Request-ID', req.id);
    next();
};

// JSON body parser (mock for now)
const jsonParser = (req, res, next) => {
    console.log('JSON parser middleware (next phase will add actual parsing)');
    req.body = {};
    next();
};

// Apply middleware
app.use(logger);
app.use(cors);
app.use(requestId);
app.use(jsonParser); // Apply globally for this phase

// Route-specific middleware
const authenticate = (req, res, next) => {
    const auth = req.headers.authorization || 'Bearer demo-token';
    req.user = { id: 'demo-user', token: auth };
    console.log('Authentication middleware applied');
    next();
};

// Routes with middleware
app.get('/', (req, res) => {
    res.json({
        message: 'Middleware chain example',
        requestId: req.id,
        timestamp: new Date().toISOString()
    });
});

// Apply authentication middleware globally for this phase
app.use(authenticate);

app.get('/protected/profile', (req, res) => {
    res.json({
        message: 'Protected route accessed',
        user: req.user,
        requestId: req.id
    });
});

// API routes with JSON parser
app.get('/api/users', (req, res) => {
    res.json({
        users: [
            { id: 1, name: 'Alice' },
            { id: 2, name: 'Bob' }
        ],
        requestId: req.id,
        parsedBody: req.body
    });
});

app.post('/api/users', (req, res) => {
    res.status(201).json({
        message: 'User would be created',
        requestId: req.id,
        receivedData: req.body,
        note: 'Next phase will handle actual request parsing'
    });
});

// Error handling middleware (last)
app.use((err, req, res, next) => {
    console.error('Error:', err.message);
    res.status(500).json({
        error: 'Internal Server Error',
        requestId: req.id,
        message: err.message
    });
});

console.log('\nMiddleware Chain Analysis:');
console.log('Total routes registered:', app.getRoutes().length);
console.log('App configuration:', app.toString());

console.log('\nMiddleware Stack (registration only):');
console.log('1. Logger - Request/response timing');
console.log('2. CORS - Cross-origin headers');
console.log('3. Request ID - Unique request tracking');
console.log('4. JSON Parser - Body parsing (global)');
console.log('5. Authentication - Protected routes');
console.log('6. Error Handler - Global error handling');

console.log('\n Note: this phase shows middleware registration');
console.log('   Next phase will implement actual middleware execution');

module.exports = app;
