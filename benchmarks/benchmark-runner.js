const autocannon = require('autocannon');
const { spawn } = require('child_process');
const { table } = require('table');
const chalk = require('chalk');

class BenchmarkRunner {
    constructor(options = {}) {
        this.duration = options.duration || 30; // seconds
        this.connections = options.connections || 100;
        this.quick = options.quick || false;
        
        if (this.quick) {
            this.duration = 10;
            this.connections = 50;
        }
    }

    async runBenchmark(name, serverFile, port, path = '/') {
        console.log(chalk.blue(`\nStarting ${name} server...`));
        
        // Start server
        const server = spawn('node', [serverFile], {
            stdio: ['pipe', 'pipe', 'pipe'],
            cwd: __dirname
        });

        // Wait for server to start
        await new Promise(resolve => setTimeout(resolve, 2000));

        try {
            console.log(chalk.yellow(`Running benchmark for ${name}...`));
            
            const result = await autocannon({
                url: `http://localhost:${port}${path}`,
                connections: this.connections,
                duration: this.duration,
                headers: {
                    'Content-Type': 'application/json'
                }
            });

            server.kill();
            return {
                name,
                requestsPerSecond: result.requests.average,
                latencyAvg: result.latency.average,
                latencyP99: result.latency.p99,
                throughput: result.throughput.average,
                errors: result.errors
            };

        } catch (error) {
            server.kill();
            throw error;
        }
    }

    async runJSONBenchmark() {
        console.log(chalk.green('\n=== JSON Response Benchmark ==='));
        
        const cExpressResult = await this.runBenchmark(
            'C-Express',
            'servers/c-express-json.js',
            3001,
            '/json'
        );

        const expressResult = await this.runBenchmark(
            'Express.js',
            'servers/express-json.js',
            3002,
            '/json'
        );

        return { cExpressResult, expressResult };
    }

    async runRoutingBenchmark() {
        console.log(chalk.green('\n=== Route Matching Benchmark ==='));
        
        const cExpressResult = await this.runBenchmark(
            'C-Express',
            'servers/c-express-routing.js',
            3003,
            '/users/123/posts/456'
        );

        const expressResult = await this.runBenchmark(
            'Express.js',
            'servers/express-routing.js',
            3004,
            '/users/123/posts/456'
        );

        return { cExpressResult, expressResult };
    }

    async runMiddlewareBenchmark() {
        console.log(chalk.green('\n=== Middleware Chain Benchmark ==='));
        
        const cExpressResult = await this.runBenchmark(
            'C-Express',
            'servers/c-express-middleware.js',
            3005,
            '/middleware'
        );

        const expressResult = await this.runBenchmark(
            'Express.js',
            'servers/express-middleware.js',
            3006,
            '/middleware'
        );

        return { cExpressResult, expressResult };
    }

    displayResults(results) {
        const data = [
            ['Metric', 'C-Express', 'Express.js', 'Improvement'],
            [
                'Requests/sec',
                this.formatNumber(results.cExpressResult.requestsPerSecond),
                this.formatNumber(results.expressResult.requestsPerSecond),
                this.formatImprovement(results.cExpressResult.requestsPerSecond, results.expressResult.requestsPerSecond)
            ],
            [
                'Avg Latency (ms)',
                this.formatNumber(results.cExpressResult.latencyAvg),
                this.formatNumber(results.expressResult.latencyAvg),
                this.formatImprovement(results.cExpressResult.latencyAvg, results.expressResult.latencyAvg, true)
            ],
            [
                'P99 Latency (ms)',
                this.formatNumber(results.cExpressResult.latencyP99),
                this.formatNumber(results.expressResult.latencyP99),
                this.formatImprovement(results.cExpressResult.latencyP99, results.expressResult.latencyP99, true)
            ],
            [
                'Throughput (MB/s)',
                this.formatNumber(results.cExpressResult.throughput / 1024 / 1024),
                this.formatNumber(results.expressResult.throughput / 1024 / 1024),
                this.formatImprovement(results.cExpressResult.throughput, results.expressResult.throughput)
            ]
        ];

        console.log('\n' + table(data, {
            border: {
                topBody: '─',
                topJoin: '┬',
                topLeft: '┌',
                topRight: '┐',
                bottomBody: '─',
                bottomJoin: '┴',
                bottomLeft: '└',
                bottomRight: '┘',
                bodyLeft: '│',
                bodyRight: '│',
                bodyJoin: '│',
                joinBody: '─',
                joinLeft: '├',
                joinRight: '┤',
                joinJoin: '┼'
            }
        }));
    }

    formatNumber(num) {
        return num.toFixed(2);
    }

    formatImprovement(cExpress, express, lowerIsBetter = false) {
        let improvement, description;
        
        if (lowerIsBetter) {
            // For latency: lower is better
            // Calculate how much lower C-Express latency is
            improvement = ((express - cExpress) / express) * 100;
            if (improvement > 0) {
                description = 'lower';
            } else {
                description = 'higher';
                improvement = Math.abs(improvement);
            }
        } else {
            // For requests/sec, throughput: higher is better
            improvement = ((cExpress - express) / express) * 100;
            if (improvement > 0) {
                description = 'faster';
            } else {
                description = 'slower';
                improvement = Math.abs(improvement);
            }
        }
        
        const color = (lowerIsBetter && description === 'lower') || (!lowerIsBetter && description === 'faster') 
            ? chalk.green : chalk.red;
        const sign = '+';
        return color(`${sign}${improvement.toFixed(1)}%`);
    }

    async run() {
        console.log(chalk.bold('\n🏁 C-Express vs Express.js Benchmark Suite\n'));
        console.log(`Duration: ${this.duration}s, Connections: ${this.connections}\n`);

        try {
            // JSON Benchmark
            const jsonResults = await this.runJSONBenchmark();
            this.displayResults(jsonResults);

            // Routing Benchmark
            const routingResults = await this.runRoutingBenchmark();
            this.displayResults(routingResults);

            // Middleware Benchmark
            const middlewareResults = await this.runMiddlewareBenchmark();
            this.displayResults(middlewareResults);

            console.log(chalk.bold.green('\nBenchmark suite completed!\n'));

        } catch (error) {
            console.error(chalk.red('Benchmark failed:'), error.message);
            process.exit(1);
        }
    }
}

// CLI usage
if (require.main === module) {
    const quick = process.argv.includes('--quick');
    const runner = new BenchmarkRunner({ quick });
    runner.run();
}

module.exports = BenchmarkRunner;