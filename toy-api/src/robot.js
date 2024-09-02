(() => {
    const { ArgumentError } = require('./exceptions');
    const serverProxy = require('./server-proxy');

    class Robot {
        constructor(initialData) {
            const data = {
                id: undefined,
                hostname: undefined,
                temperature: undefined,
            };

            let updatedFields = {
                name: false,
            };

            for (const property in data) {
                if (Object.prototype.hasOwnProperty.call(data, property) && property in initialData) {
                    data[property] = initialData[property];
                }
            }

            Object.defineProperties(
                this,
                Object.freeze({
                    id: {
                        get: () => data.id,
                    },
                    hostname: {
                        get: () => data.hostname,
                        set: (value) => {
                            if (!value.trim().length) {
                                throw new ArgumentError('Value must not be empty');
                            }
                            updatedFields.hostname = true;
                            data.hostname = value;
                        },
                    },
                    temperature: {
                        get: () => data.temperature,
                    },
                    __updatedFields: {
                        get: () => updatedFields,
                        set: (fields) => {
                            updatedFields = fields;
                        },
                    },
                }),
            );
        }

        serialize() {
            return {
                id: this.id,
                hostname: this.hostname,
                temperature: this.temperature,
            };
        }

        toJSON() {
            return this.serialize();
        }

        async save(which) {
            const result = await serverProxy.Robots.saveRobot(this.id, this.toJSON(), which);
            return result;
        }
    }

    module.exports = {
        Robot,
    };

})();
