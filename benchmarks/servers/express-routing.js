// Express.js Routing Benchmark Server
const express = require('express');
const app = express();

// Complex routing patterns
app.get('/users/:userId/posts/:postId', (req, res) => {
    res.json({
        framework: 'express',
        userId: req.params.userId,
        postId: req.params.postId,
        timestamp: Date.now()
    });
});

app.get('/api/v1/users/:id', (req, res) => {
    res.json({ id: req.params.id, framework: 'express' });
});

app.get('/api/v2/posts/:slug/comments/:commentId', (req, res) => {
    res.json({
        slug: req.params.slug,
        commentId: req.params.commentId,
        framework: 'express'
    });
});

app.get('/static/*', (req, res) => {
    res.json({ path: req.params[0], framework: 'express' });
});

const port = 3004;
app.listen(port, () => {
    console.log(`Express.js routing benchmark server running on port ${port}`);
});