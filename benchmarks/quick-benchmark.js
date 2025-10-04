// Quick benchmark results script
const BenchmarkRunner = require('./benchmark-runner');

async function runQuickBenchmarks() {
    console.log('C-Express Performance Benchmark\n');
    
    const runner = new BenchmarkRunner({ quick: true });
    
    try {
        // JSON Response Test
        console.log('Testing JSON Response Performance...');
        const jsonResults = await runner.runJSONBenchmark();
        
        console.log('\nJSON RESPONSE RESULTS:');
        console.log(`C-Express:  ${jsonResults.cExpressResult.requestsPerSecond.toFixed(0)} req/s`);
        console.log(`Express.js: ${jsonResults.expressResult.requestsPerSecond.toFixed(0)} req/s`);
        console.log(`Improvement: ${((jsonResults.cExpressResult.requestsPerSecond / jsonResults.expressResult.requestsPerSecond - 1) * 100).toFixed(1)}%\n`);
        
        // Routing Test
        console.log('Testing Route Matching Performance...');
        const routingResults = await runner.runRoutingBenchmark();
        
        console.log('\nROUTING RESULTS:');
        console.log(`C-Express:  ${routingResults.cExpressResult.requestsPerSecond.toFixed(0)} req/s`);
        console.log(`Express.js: ${routingResults.expressResult.requestsPerSecond.toFixed(0)} req/s`);
        console.log(`Improvement: ${((routingResults.cExpressResult.requestsPerSecond / routingResults.expressResult.requestsPerSecond - 1) * 100).toFixed(1)}%\n`);
        
        console.log('Benchmark Complete!');
        process.exit(0);
        
    } catch (error) {
        console.error('Benchmark failed:', error.message);
        process.exit(1);
    }
}

runQuickBenchmarks();