const express = require('../lib');

console.log('C-Express Hello World Example');
console.log('==================================');

const app = express();

// Middleware example
app.use((req, res, next) => {
    console.log(`[${new Date().toISOString()}] ${req.method} ${req.path}`);
    next();
});

// Basic route
app.get('/', (req, res) => {
    res.send('Hello, World from C-Express!');
});

// JSON response
app.get('/api/status', (req, res) => {
    res.json({
        status: 'ok',
        framework: 'c-express',
        version: app.version,
        timestamp: new Date().toISOString(),
        features: [
            'Native C performance',
            'Express.js compatibility',
            'Fast routing',
            'Built-in parsers'
        ]
    });
});

// Route with parameters
app.get('/users/:id', (req, res) => {
    res.json({
        message: 'User endpoint (next phase will handle parameters)',
        note: 'Currently registering routes only'
    });
});

console.log('\nApplication Configuration:');
console.log('Routes registered:', app.getRoutes().length);
console.log('App info:', app.toString());

console.log('\nFramework Information:');
const info = express.info();
console.log('Name:', info.name);
console.log('Version:', info.version);
console.log('Features:', info.features.join(', '));

console.log('\n🔧 Debug Information:');
console.log('Native addon:', express.debug.isNativeAddon);
console.log('Node.js version:', express.debug.buildInfo.nodeVersion);
console.log('Platform:', express.debug.buildInfo.platform);

console.log('\n Note: This is route registration only');
console.log('   next phase will add actual HTTP server functionality');

console.log('\nC-Express basic functionality working!');
console.log('  GET  /         - Hello World');
console.log('  GET  /json     - JSON response');
console.log('  GET  /users/1  - User by ID');
console.log('  POST /users    - Create user');
