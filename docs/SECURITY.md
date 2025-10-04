# Security Policy

## Supported Versions

We actively maintain security updates for the following versions:

| Version | Supported          |
| ------- | -----------------  |
| 1.0.x   |          ✓         |

## Security Considerations

### Input Validation
- **Always validate user inputs** - C-Express does not automatically sanitize inputs
- **Use parameterized queries** for database operations
- **Validate file uploads** before processing
- **Sanitize HTML output** to prevent XSS attacks

### Memory Safety
- C-Express uses memory-safe practices but C code requires careful handling
- All user inputs are bounds-checked
- Memory leaks are actively monitored in our test suite

### Production Recommendations
- **Use HTTPS in production** - Set up Node.js HTTPS server
- **Implement rate limiting** at application or reverse proxy level
- **Keep dependencies updated** - Run `npm audit` regularly
- **Use environment variables** for sensitive configuration
- **Enable logging and monitoring** for security events

### Known Security Considerations
- **Session management**: Not included - use external session stores
- **CSRF protection**: Not built-in - implement middleware as needed
- **Content Security Policy**: Configure at application level

## Reporting a Vulnerability

If you discover a security vulnerability in C-Express, please report it privately by:

1. **Email**: gomezlopez.2004[at]gmail.com
2. **GitHub Security Advisories**: Use the private vulnerability reporting feature
3. **Response Time**: We aim to respond within 48 hours

### What to Include
- Description of the vulnerability
- Steps to reproduce
- Potential impact assessment
- Any proof-of-concept code (if applicable)

### Our Process
1. **Acknowledgment**: We'll confirm receipt within 48 hours
2. **Investigation**: Our team will investigate and assess the impact
3. **Fix Development**: We'll develop and test a fix
4. **Disclosure**: We'll coordinate responsible disclosure with you
5. **Credit**: We'll acknowledge your contribution (if desired)

## Security Best Practices for Users

### Environment Configuration
```javascript
// Use environment variables for sensitive data
const port = process.env.PORT || 3000;
const dbUrl = process.env.DATABASE_URL;

// Never hardcode secrets
// Bad
const apiKey = "sk-1234567890abcdef";

// Good  
const apiKey = process.env.API_KEY;
```

### Input Validation Example
```javascript
const app = require('@nachooglpz/c-express')();

// Validate and sanitize inputs
app.post('/users', (req, res) => {
  const { name, email } = req.body;
  
  // Validate required fields
  if (!name || !email) {
    return res.status(400).json({ error: 'Missing required fields' });
  }
  
  // Validate email format
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  if (!emailRegex.test(email)) {
    return res.status(400).json({ error: 'Invalid email format' });
  }
  
  // Sanitize name (remove potential HTML/script tags)
  const sanitizedName = name.replace(/<[^>]*>/g, '');
  
  // Process safely...
});
```

### HTTPS Setup
```javascript
const https = require('https');
const fs = require('fs');
const app = require('@nachooglpz/c-express')();

const options = {
  key: fs.readFileSync('path/to/private-key.pem'),
  cert: fs.readFileSync('path/to/certificate.pem')
};

https.createServer(options, app).listen(443);
```

## Updates and Patches

- Security patches will be released as soon as possible
- We'll notify users through GitHub releases and npm advisories
- Update to the latest version to receive security fixes

---

**Note**: This security policy will be updated as the project evolves. Please check back regularly for updates.