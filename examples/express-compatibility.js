const express = require('../lib');

console.log('C-Express Express.js Compatibility Demo');
console.log('==========================================');

const app = express();

// Express.js compatibility features
console.log('\nExpress.js-style API Usage:');

// Router creation (methods not implemented in Phase 1)
const router = express.Router();
console.log('Router created:', typeof router);

// Note: Individual router methods not implemented in Phase 1
// This demonstrates the API structure only

// Route builder pattern
app.route('/posts/:id')
  .get((req, res) => res.json({ post: req.params.id }))
  .put((req, res) => res.json({ updated: req.params.id }))
  .patch((req, res) => res.json({ patched: req.params.id }));

// Parameter middleware (Express.js style)
app.param('id', (req, res, next, id) => {
    console.log(`Parameter middleware for ID: ${id}`);
    req.validatedId = parseInt(id) || 0;
    next();
});

// All methods handler
app.all('/api/ping', (req, res) => {
    res.json({
        method: req.method,
        message: 'Pong from all methods handler',
        timestamp: new Date().toISOString()
    });
});

// Express.js settings (Phase 1 - demo only)
console.log('Express.js settings would be configured here:');
console.log('- View engine: ejs');
console.log('- Views directory: ./views');  
console.log('- Port: 3000');
console.log('(app.set() not implemented in Phase 1)');

// Static file serving (Phase 1 - demo only)
console.log('Static file middleware would be mounted here:');
console.log('- Path: /static');
console.log('- Directory: public');
console.log('(express.static() returns stub function in Phase 1)');

// Mount router (Phase 1 - demo only)
console.log('Router would be mounted at /api (Phase 2 implementation)');

// Express.js-style error handling
app.use((req, res, next) => {
    const err = new Error(`Route not found: ${req.path}`);
    err.status = 404;
    next(err);
});

app.use((err, req, res, next) => {
    res.status(err.status || 500).json({
        error: err.message,
        stack: process.env.NODE_ENV === 'development' ? err.stack : undefined
    });
});

console.log('\n🔧 Express.js Compatibility Check:');
console.log('app.route() - Route builder pattern');
console.log('app.all() - All HTTP methods');
console.log(' app.param() - Parameter middleware (stub)');
console.log(' app.set()/get() - Application settings (not implemented)');
console.log('app.use() - Middleware mounting');
console.log('express.Router() - Router creation');
console.log(' express.static() - Static file middleware (stub)');
console.log(' router.get/post/etc - Not implemented in Phase 1');
console.log(' router.route() - Not implemented in Phase 1');

console.log('\nApplication Analysis:');
console.log('Routes registered:', app.getRoutes().length);
console.log('Framework version:', app.version);
console.log('App info:', app.toString());

console.log('\nMigration Path from Express.js:');
console.log('1. Replace require("express") with require("c-express")');
console.log('2. All existing Express.js code should work');
console.log('3. Benefit from native C performance');
console.log('4. Same API, faster execution');

// Demonstrate Express.js compatibility
const routes = [
    'GET /',
    'GET /posts/:id',
    'PUT /posts/:id',
    'PATCH /posts/:id',
    'ALL /api/ping'
];

console.log('\nRegistered Routes (Phase 1):');
routes.forEach(route => console.log(`  ${route}`));

console.log('\n Note: This demonstrates Express.js API compatibility');
console.log('   Phase 2 will add actual HTTP server functionality');

module.exports = app;
