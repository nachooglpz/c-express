# C-Express Web Framework

A fast, lightweight HTTP web framework for Node.js with Express.js-inspired API, powered by native C implementation.

[![npm version](https://badge.fury.io/js/@nachooglpz%2Fc-express.svg)](https://badge.fury.io/js/@nachooglpz%2Fc-express)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Node.js Version](https://img.shields.io/node/v/@nachooglpz/c-express.svg)](https://nodejs.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#)
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](#)

## Features

✓ **Express.js Compatible** - Drop-in replacement for many Express.js applications  
✓ **High Performance** - Native C implementation for speed  
✓ **Request Streaming** - Handle large uploads efficiently with automatic detection  
✓ **Route Patterns** - Advanced pattern matching with parameters and constraints  
✓ **JSON/Form Parsing** - Built-in parsing with schema validation  
✓ **Content Negotiation** - HTTP content negotiation support  
✓ **Middleware System** - Express-style middleware with error handling  
✓ **Memory Safe** - AddressSanitizer and memory leak detection  
✓ **TypeScript Support** - Full TypeScript definitions included

## Installation

```bash
npm install @nachooglpz/c-express
```

## Quick Start

### Basic Usage

```javascript
const express = require('@nachooglpz/c-express');
const app = express();

// Middleware
app.use(express.json());

// Routes
app.get('/', (req, res) => {
  res.json({ message: 'Hello from C-Express!' });
});

app.get('/users/:id', (req, res) => {
  res.json({ user: req.params.id });
});

// Start server
app.listen(3000, () => {
  console.log('Server running on port 3000');
});
```

### Express.js Migration

C-Express is designed to be a drop-in replacement for Express.js:

```javascript
// Just change this line:
// const express = require('express');
const express = require('@nachooglpz/c-express');

// Rest of your Express.js code works the same!
```

## API Reference

### Application

```javascript
const app = express();

// HTTP methods
app.get(path, handler)
app.post(path, handler)
app.put(path, handler)
app.delete(path, handler)
app.use([path], middleware)

// Start server
app.listen(port, [callback])
```

### Request Object

```javascript
app.get('/users/:id', (req, res) => {
  console.log(req.params.id);    // Route parameters
  console.log(req.query);        // Query string
  console.log(req.body);         // Request body (with middleware)
  console.log(req.headers);      // HTTP headers
  console.log(req.method);       // HTTP method
  console.log(req.url);          // Request URL
});
```

### Response Object

```javascript
app.get('/', (req, res) => {
  res.status(200);                    // Set status code
  res.json({ key: 'value' });         // Send JSON
  res.send('Hello World');            // Send response
  res.setHeader('Content-Type', 'text/html');
});
```

### Middleware

```javascript
// Application-level middleware
app.use((req, res, next) => {
  console.log('Time:', Date.now());
  next(); // Call next middleware
});

// Route-specific middleware
app.get('/protected', authenticate, (req, res) => {
  res.json({ message: 'Protected route' });
});

function authenticate(req, res, next) {
  // Your authentication logic
  next();
}
```

## Examples

See the `examples/` directory for more comprehensive examples:

- **Hello World** - Basic server setup
- **Middleware Demo** - Middleware usage patterns  
- **Express Compatibility** - Express.js migration examples

## Performance

C-Express is built for performance:

- **Native C Implementation** - Core routing and parsing in C
- **Optimized Memory Usage** - Efficient memory management
- **Fast Route Matching** - Advanced pattern matching algorithms
- **Streaming Support** - Handle large requests efficiently

## Building from Source

If you need to build from source:

```bash
git clone https://github.com/nachooglpz/c-express.git
cd c-express
npm install
npm run build
npm test
```

## Requirements

- Node.js >= 16.0.0
- C compiler (gcc/clang)
- Python 3.x (for node-gyp)

## Available Tests

### Unit Tests
- `make test-minimal_pattern` - Route pattern matching
- `make test-response_api` - Response API functionality

### Server Examples  
- `make test-streaming` - Request streaming
- `make test-validation_simple` - Route validation
- `make test-json_parsing` - JSON parsing
- `make test-form_data` - Form handling

See [`docs/TESTS_SUITE.md`](docs/TEST_SUITE.md) for complete test documentation.

## Installation

### As a Node.js Package

```bash
npm install @nachooglpz/c-express
```

## Known Issues & Limitations

### Current Limitations:
- **File uploads**: Multipart form data not yet implemented
- **Static file serving**: Basic implementation, no advanced caching
- **Session management**: Not included in v1.0.0
- **HTTPS**: Requires manual Node.js HTTPS server setup
- **Clustering**: Single-process only in current version

### Platform Support:
- **Linux** (x64) - Fully tested and supported
- **macOS** - Should work but not extensively tested
- **Windows** - Requires Visual Studio build tools

### Security Considerations:
- Input validation is your responsibility - sanitize user inputs
- Rate limiting should be implemented at application level
- Use HTTPS in production (via Node.js HTTPS server)

### Roadmap:
See [GitHub Issues](https://github.com/nachooglpz/c-express/issues) for planned features and improvements.

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Support

- [Documentation](https://github.com/nachooglpz/c-express#readme)
- [Issue Tracker](https://github.com/nachooglpz/c-express/issues)
- [Discussions](https://github.com/nachooglpz/c-express/discussions)

## Changelog

See [Releases](https://github.com/nachooglpz/c-express/releases) for detailed changelog.
