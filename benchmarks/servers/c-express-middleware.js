// C-Express Middleware Benchmark Server
const express = require('../../lib/index.js');
const app = express();

// Middleware chain
app.use((req, res, next) => {
    req.timestamp = Date.now();
    next();
});

app.use((req, res, next) => {
    req.requestId = Math.random().toString(36).substr(2, 9);
    next();
});

app.use((req, res, next) => {
    req.userAgent = req.headers['user-agent'] || 'unknown';
    next();
});

app.use((req, res, next) => {
    req.method = req.method.toUpperCase();
    next();
});

app.use((req, res, next) => {
    req.processed = true;
    next();
});

app.get('/middleware', (req, res) => {
    res.json({
        framework: 'c-express',
        timestamp: req.timestamp,
        requestId: req.requestId,
        userAgent: req.userAgent,
        method: req.method,
        processed: req.processed,
        middlewareCount: 5
    });
});

const port = 3005;
app.listen(port, () => {
    console.log(`C-Express middleware benchmark server running on port ${port}`);
});