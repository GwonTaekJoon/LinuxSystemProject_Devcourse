function build() {
    const { Robot } = require('./robot');
    const serverProxy = require('./server-proxy');
    const enums = require('./enums');

    const {
        Exception, ArgumentError, DataError, ScriptingError, ServerError,
    } = require('./exceptions');
    const pjson = require('../package.json');
    const config = require('./config');

    const toy = {
        server: {
            async request(url, data) {
                const result = await serverProxy.server.request(url, data);
                return result;
            },
        },
        robots: {
            async get(filter = {}) {
                const searchParams = new URLSearchParams();
                for (const field of ['name', 'owner', 'search', 'status', 'type', 'id', 'page']) {
                    if (Object.prototype.hasOwnProperty.call(filter, field)) {
                        searchParams.set(field, filter[field]);
                    }
                }
                const robotsData = await serverProxy.robots.getRobots(searchParams.toString());
                const robots = robotsData.map((robot) => new Robot(robot));


                robots.count = robotsData.count;

                return robots;
            },
        },
        config: {
            get backendAPI() {
                return config.backendAPI;
            },
            set backendAPI(value) {
                config.backendAPI = value;
            },
        },
        client: {
            version: `${pjson.version}`,
        },
        enums,
        exceptions: {
            Exception,
            ArgumentError,
            DataError,
            ScriptingError,
            ServerError,
        },
        classes: {
            Robot,
        },
    };

    toy.server = Object.freeze(toy.server);
    toy.robots = Object.freeze(toy.robots);
    toy.client = Object.freeze(toy.client);
    toy.enums = Object.freeze(toy.enums);

    return toy;
}

module.exports = build();
