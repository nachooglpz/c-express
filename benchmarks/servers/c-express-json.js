// C-Express JSON Response Benchmark Server
const express = require('../../lib/index.js'); // Use local C-Express
const app = express();

const testData = {
    message: 'Hello from C-Express!',
    timestamp: new Date().toISOString(),
    data: {
        users: Array.from({ length: 100 }, (_, i) => ({
            id: i + 1,
            name: `User ${i + 1}`,
            email: `user${i + 1}@example.com`,
            active: Math.random() > 0.5
        }))
    },
    meta: {
        total: 100,
        framework: 'c-express',
        version: '1.0.0'
    }
};

app.get('/json', (req, res) => {
    res.json(testData);
});

app.get('/health', (req, res) => {
    res.json({ status: 'ok', framework: 'c-express' });
});

const port = 3001;
app.listen(port, () => {
    console.log(`C-Express JSON benchmark server running on port ${port}`);
});