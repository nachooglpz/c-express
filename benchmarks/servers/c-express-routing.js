// C-Express Routing Benchmark Server
const express = require('../../lib/index.js');
const app = express();

// Complex routing patterns
app.get('/users/:userId/posts/:postId', (req, res) => {
    res.json({
        framework: 'c-express',
        userId: req.params.userId,
        postId: req.params.postId,
        timestamp: Date.now()
    });
});

app.get('/api/v1/users/:id', (req, res) => {
    res.json({ id: req.params.id, framework: 'c-express' });
});

app.get('/api/v2/posts/:slug/comments/:commentId', (req, res) => {
    res.json({
        slug: req.params.slug,
        commentId: req.params.commentId,
        framework: 'c-express'
    });
});

app.get('/static/*', (req, res) => {
    res.json({ path: req.params[0], framework: 'c-express' });
});

const port = 3003;
app.listen(port, () => {
    console.log(`C-Express routing benchmark server running on port ${port}`);
});