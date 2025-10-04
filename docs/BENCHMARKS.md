# C-Express vs Express.js Benchmarks

This benchmark suite compares the performance of C-Express against the standard Express.js framework across different scenarios.

## Quick Start

```bash
# Install dependencies
cd benchmarks
npm install

# Run all benchmarks
npm run bench

# Run quick benchmarks (10s duration, 50 connections)
npm run bench:quick

# Run specific benchmarks
npm run bench:json      # JSON response performance
npm run bench:routing   # Route matching performance
npm run bench:middleware # Middleware chain performance
```

## Benchmark Categories

### 1. JSON Response Benchmark
Tests the performance of returning JSON responses with a moderately complex payload (~100 user objects).

**Endpoints tested:**
- `GET /json` - Returns JSON with user data array

### 2. Route Matching Benchmark
Tests the performance of complex route pattern matching with parameters.

**Endpoints tested:**
- `GET /users/:userId/posts/:postId` - Nested parameter routes

### 3. Middleware Chain Benchmark
Tests the performance of middleware execution chains.

**Middleware stack:**
- Timestamp assignment
- Request ID generation
- User agent extraction
- Method normalization
- Processing flag setting

## Configuration

### Benchmark Parameters

- **Duration**: 30 seconds (10s for quick mode)
- **Connections**: 100 concurrent (50 for quick mode)
- **HTTP Tool**: Autocannon
- **Metrics Collected**: Requests/sec, Latency (avg/p99), Throughput

### Environment Requirements

- Node.js >= 16.0.0
- Linux/macOS (recommended for accurate timing)
- Available ports: 3001-3006

## Expected Results

C-Express should demonstrate significant performance improvements due to:

1. **Native C Implementation**: Core routing and parsing in C
2. **Optimized Memory Usage**: Efficient memory management
3. **Fast Route Matching**: Advanced pattern matching algorithms
4. **Reduced JavaScript Overhead**: Less V8 execution for core operations

## Interpreting Results

The benchmark runner displays results in a formatted table showing:

- **Requests/sec**: Higher is better
- **Avg Latency (ms)**: Lower is better
- **P99 Latency (ms)**: Lower is better (99th percentile)
- **Throughput (MB/s)**: Higher is better
- **Improvement**: Percentage improvement of C-Express vs Express.js

### Sample Output

```
┌───────────────────┬───────────┬────────────┬─────────────┐
│ Metric            │ C-Express │ Express.js │ Improvement │
├───────────────────┼───────────┼────────────┼─────────────┤
│ Requests/sec      │ 37463.31  │ 14798.87   │ +153.1%     │
│ Avg Latency (ms)  │ 2.17      │ 6.11       │ +64.5%      │
│ P99 Latency (ms)  │ 5.00      │ 13.00      │ +61.5%      │
│ Throughput (MB/s) │ 262.21    │ 104.72     │ +150.4%     │
└───────────────────┴───────────┴────────────┴─────────────┘
```

## Advanced Profiling

For detailed performance analysis, you can use clinic.js:

```bash
# Install clinic globally
npm install -g clinic

# Profile C-Express
clinic doctor -- node servers/c-express-json.js

# Profile Express.js
clinic doctor -- node servers/express-json.js
```

## Troubleshooting

### Port Already in Use
If benchmark servers fail to start, ensure ports 3001-3006 are available:

```bash
# Check for processes using these ports
lsof -i :3001-3006

# Kill processes if needed
kill -9 <PID>
```

### Memory Issues
For high-load benchmarks, you may need to increase Node.js memory:

```bash
node --max-old-space-size=4096 benchmark-runner.js
```

### Inconsistent Results
- Ensure no other CPU-intensive processes are running
- Run benchmarks multiple times and average results
- Use a dedicated machine for accurate measurements

## Adding Custom Benchmarks

To add a new benchmark:

1. Create server files in `servers/` directory
2. Add benchmark method to `BenchmarkRunner` class
3. Update the `run()` method to include your benchmark

Example:

```javascript
async runCustomBenchmark() {
    const cExpressResult = await this.runBenchmark(
        'C-Express Custom',
        'servers/c-express-custom.js',
        3007,
        '/custom'
    );

    const expressResult = await this.runBenchmark(
        'Express.js Custom',
        'servers/express-custom.js',
        3008,
        '/custom'
    );

    return { cExpressResult, expressResult };
}
```

## Contributing

To improve benchmarks:

1. Add more realistic test scenarios
2. Include different payload sizes
3. Test edge cases (error handling, invalid routes)
4. Add memory usage profiling
5. Include long-running stability tests

## CI/CD Integration

For continuous performance monitoring:

```bash
# Run benchmarks in CI
npm run bench:quick > benchmark-results.txt

# Parse results for regression detection
node parse-benchmark-results.js benchmark-results.txt
```