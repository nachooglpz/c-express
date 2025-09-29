# C Express

A high-performance Express.js-like web framework implemented in C with Node.js bindings.

## Features

- 🚀 **High Performance**: Written in C for maximum speed
- 🔧 **Middleware Support**: Composable request/response handling
- 🛣️ **Advanced Routing**: Pattern matching, parameters, subrouters
- 📝 **Body Parsing**: JSON and form data parsing built-in
- 🧪 **Well Tested**: Comprehensive unit and integration tests

## Installation

### As a Node.js Package (Coming Soon)

```bash
npm install c-express
```

### Building from Source

```bash
git clone https://github.com/nachooglpz/c-express.git
cd c-express
make
```

## Quick Start

### C Usage

```c
#include "src/core/app.h"

void hello_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = ctx->user_context;
    
    response_send(res, "Hello, World!");
}

int main() {
    App app = create_app();
    app.get(&app, "/", hello_handler);
    app.listen(&app, 3000);
    return 0;
}

### Node.js Usage (Future)

```javascript
const express = require('c-express');
const app = express();

app.get('/', (req, res) => {
  res.send('Hello, World!');
});

app.listen(3000);
```

## Project Structure

```
c-express/
├── src/                    # Core C library source
│   ├── core/               # Framework core (app, router, routes)
│   ├── http/               # HTTP handling (request, response, errors)
│   └── parsers/            # Data parsing (JSON, forms)
├── tests/                  # Test suite
│   ├── unit/               # Unit tests
│   └── integration/        # Integration tests
├── examples/               # Usage examples
├── docs/                   # Documentation
└── lib/                    # Node.js bindings (future)
```

## Development

### Building

```bash
make                   # Build library
make example           # Build example server
make test              # Run all tests
```

### Testing

```bash
make test_unit         # Run unit tests
make test_integration  # Run integration tests
```

### Code Quality

```bash
make format          # Format code
make lint            # Lint code
```

## API Reference

### Core Components

- **App**: Main application instance
- **Router**: Route management and middleware
- **Request**: HTTP request parsing and utilities
- **Response**: HTTP response building and sending
- **Middleware**: Composable request/response handlers

### Key Functions

- `create_app()`: Create new application
- `app.get/post/put/delete()`: Route registration
- `app.use()`: Middleware registration
- `app.listen()`: Start server

## Roadmap

- [✓] Core HTTP server functionality
- [✓] Routing and middleware system
- [✓] JSON and form parsing
- [ ] Node.js native addon bindings
- [ ] NPM package publication
- [ ] Performance benchmarks
- [ ] Documentation website

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Run `make test`
6. Submit a pull request

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Performance

C Express is designed for high-performance scenarios where Node.js might be a bottleneck. Benchmarks coming soon.

## Support

- GitHub Issues: [Report bugs and request features](https://github.com/nachooglpz/c-express/issues)
- Documentation: [Full API documentation](docs/API.md)