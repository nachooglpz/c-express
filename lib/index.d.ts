// Type definitions for c-express
// Project: https://github.com/nachooglpz/c-express
// Definitions by: Nacho González López

export = express;

declare function express(): express.Application;

declare namespace express {
    interface Request {
        method: string;
        path: string;
        query: { [key: string]: string };
        params: { [key: string]: string };
        headers: { [key: string]: string };
        body?: any;
        
        header(name: string): string | undefined;
        param(name: string): string | undefined;
        query(name: string): string | undefined;
    }
    
    interface Response {
        statusCode: number;
        
        send(data: string | Buffer): Response;
        json(data: any): Response;
        status(code: number): Response;
        set(name: string, value: string): Response;
        header(name: string, value: string): Response;
        get(name: string): string | undefined;
        end(data?: string): Response;
        type(type: string): Response;
        cookie(name: string, value: string, options?: CookieOptions): Response;
        clearCookie(name: string, options?: CookieOptions): Response;
        redirect(url: string): Response;
        redirect(status: number, url: string): Response;
    }
    
    interface NextFunction {
        (err?: Error): void;
    }
    
    interface RequestHandler {
        (req: Request, res: Response, next: NextFunction): void;
    }
    
    interface ErrorRequestHandler {
        (err: Error, req: Request, res: Response, next: NextFunction): void;
    }
    
    interface Application {
        get(path: string, handler: RequestHandler): Application;
        post(path: string, handler: RequestHandler): Application;
        put(path: string, handler: RequestHandler): Application;
        delete(path: string, handler: RequestHandler): Application;
        patch(path: string, handler: RequestHandler): Application;
        options(path: string, handler: RequestHandler): Application;
        head(path: string, handler: RequestHandler): Application;
        
        // Multiple handlers support
        all(path: string, ...handlers: RequestHandler[]): Application;
        
        use(handler: RequestHandler): Application;
        use(path: string, handler: RequestHandler): Application;
        use(router: Router): Application;
        use(path: string, router: Router): Application;
        
        mount(prefix: string, router: Router): Application;
        
        listen(port: number, callback?: () => void): Application;
        
        error(handler: ErrorRequestHandler): Application;
        
        // Utility methods
        printRoutes(): void;
        getRoutes(): RouteInfo[];
        toString(): string;
        version: string;
        
        // Parameter middleware
        param(name: string, handler: (req: Request, res: Response, next: NextFunction, value: string, name: string) => void): Application;
        
        // Route creation
        route(path: string): RouteBuilder;
    }
    
    interface RouteBuilder {
        get(handler: RequestHandler): RouteBuilder;
        post(handler: RequestHandler): RouteBuilder;
        put(handler: RequestHandler): RouteBuilder;
        delete(handler: RequestHandler): RouteBuilder;
        patch(handler: RequestHandler): RouteBuilder;
        options(handler: RequestHandler): RouteBuilder;
    }
    
    interface Router {
        get(path: string, handler: RequestHandler): Router;
        post(path: string, handler: RequestHandler): Router;
        put(path: string, handler: RequestHandler): Router;
        delete(path: string, handler: RequestHandler): Router;
        patch(path: string, handler: RequestHandler): Router;
        options(path: string, handler: RequestHandler): Router;
        
        use(handler: RequestHandler): Router;
        use(path: string, handler: RequestHandler): Router;
    }
    
    interface RouteInfo {
        method: string;
        path: string;
        handler: string;
    }
    
    interface CookieOptions {
        maxAge?: number;
        expires?: Date;
        httpOnly?: boolean;
        secure?: boolean;
        sameSite?: boolean | 'lax' | 'strict' | 'none';
        domain?: string;
        path?: string;
    }
    
    interface StaticOptions {
        dotfiles?: 'allow' | 'deny' | 'ignore';
        etag?: boolean;
        extensions?: string[];
        fallthrough?: boolean;
        immutable?: boolean;
        index?: boolean | string | string[];
        lastModified?: boolean;
        maxAge?: number;
        redirect?: boolean;
        setHeaders?: (res: Response, path: string, stat: any) => void;
    }
    
    interface JsonOptions {
        inflate?: boolean;
        limit?: number | string;
        reviver?: (key: string, value: any) => any;
        strict?: boolean;
        type?: string | string[] | ((req: Request) => boolean);
        verify?: (req: Request, res: Response, buf: Buffer, encoding: string) => void;
    }
    
    interface UrlencodedOptions {
        extended?: boolean;
        inflate?: boolean;
        limit?: number | string;
        parameterLimit?: number;
        type?: string | string[] | ((req: Request) => boolean);
        verify?: (req: Request, res: Response, buf: Buffer, encoding: string) => void;
    }
    
    // Static constructors
    const App: {
        new(): Application;
    };
    
    // Debug and introspection utilities
    interface DebugInfo {
        isNativeAddon: boolean;
        buildInfo: {
            compiler: string;
            nodeVersion: string;
            platform: string;
            arch: string;
        };
    }
    
    interface InfoObject {
        name: string;
        version: string;
        description: string;
        features: string[];
    }
    
    // Factory functions
    function Router(): Router;
    
    // Middleware functions
    function static(root: string, options?: StaticOptions): RequestHandler;
    function json(options?: JsonOptions): RequestHandler;
    function urlencoded(options?: UrlencodedOptions): RequestHandler;
    
    // Metadata
    const version: string;
    const name: string;
    
    // Utility functions
    function info(): InfoObject;
    const debug: DebugInfo;
}
