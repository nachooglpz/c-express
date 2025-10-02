console.log('Testing C-Express addon loading...');

try {
    console.log('Step 1: Loading addon...');
    const addon = require('./build/Release/c_express_addon');
    console.log('✓ Addon loaded successfully');
    console.log('Addon exports:', Object.keys(addon));
    
    console.log('Step 2: Testing createApp...');
    const app = addon.createApp();
    console.log('✓ App created successfully');
    console.log('App methods:', Object.getOwnPropertyNames(app).filter(name => typeof app[name] === 'function'));
    
    console.log('Step 3: Testing route registration...');
    app.get('/', function(req, res) {
        console.log('Route handler called');
    });
    console.log('✓ Route registered successfully');
    
    console.log('\n🎉 Basic addon functionality is working!');
    
} catch (error) {
    console.error('❌ Error:', error.message);
    console.error('Stack:', error.stack);
}
