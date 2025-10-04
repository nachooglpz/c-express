const express = require('../lib');

console.log('HTTP Server Bridge Test');
console.log('==================================');

const app = express();

// Basic middleware
app.use((req, res, next) => {
    console.log(`${req.method} ${req.path}`);
    req.timestamp = new Date().toISOString();
    next();
});

// Basic route
app.get('/', (req, res) => {
    res.json({
        message: 'Hello from HTTP Server Brigde!',
        framework: 'c-express',
        timestamp: req.timestamp,
        phase: 2
    });
});

// Route with parameters
app.get('/users/:id', (req, res) => {
    res.json({
        message: `User ${req.params.id}`,
        userId: req.params.id,
        phase: 2,
        timestamp: req.timestamp
    });
});

// POST route
app.post('/data', (req, res) => {
    res.json({
        message: 'Data received',
        received: req.body,
        method: req.method,
        phase: 2
    });
});

const port = 3000;
console.log(`Starting server on port ${port}...`);

app.listen(port, () => {
    console.log(`Server listening on port ${port}`);
    console.log(`Test URLs:`);
    console.log(`   GET  http://localhost:${port}/`);
    console.log(`   GET  http://localhost:${port}/users/123`);
    console.log(`   POST http://localhost:${port}/data`);
    console.log('');
    console.log('Press Ctrl+C to stop the server');
});