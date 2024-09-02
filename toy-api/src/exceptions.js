(() => {
    const Platform = require('platform');
    const ErrorStackParser = require('error-stack-parser');
    const config = require('./config');

    class Exception extends Error {
        constructor(message) {
            super(message);

            const time = new Date().toISOString();
            const system = Platform.os.toString();
            const client = `${Platform.name} ${Platform.version}`;
            const info = ErrorStackParser.parse(this)[0];
            const filename = `${info.fileName}`;
            const line = info.lineNumber;
            const column = info.columnNumber;
            const { clientID } = config;

            Object.defineProperties(
                this,
                Object.freeze({
                    system: {
                        get: () => system,
                    },
                    client: {
                        get: () => client,
                    },
                    time: {
                        get: () => time,
                    },
                    clientID: {
                        get: () => clientID,
                    },
                    filename: {
                        get: () => filename,
                    },
                    line: {
                        get: () => line,
                    },
                    column: {
                        get: () => column,
                    },
                }),
            );
        }

        async save() {
            const exceptionObject = {
                system: this.system,
                client: this.client,
                time: this.time,
                client_id: this.clientID,
                message: this.message,
                filename: this.filename,
                line: this.line,
                column: this.column,
                stack: this.stack,
            };

            try {
                const serverProxy = require('./server-proxy');
                await serverProxy.server.exception(exceptionObject);
            } catch (exception) {
                console.log('add exception');
            }
        }
    }

    class ArgumentError extends Exception {
        constructor(message) {
            super(message);
        }
    }

    class DataError extends Exception {
        constructor(message) {
            super(message);
        }
    }

    class ScriptingError extends Exception {
        constructor(message) {
            super(message);
        }
    }

    class ServerError extends Exception {
        constructor(message, code) {
            super(message);

            Object.defineProperties(
                this,
                Object.freeze({
                    code: {
                        get: () => code,
                    },
                }),
            );
        }
    }

    module.exports = {
        Exception,
        ArgumentError,
        DataError,
        ScriptingError,
        ServerError,
    };
})();
