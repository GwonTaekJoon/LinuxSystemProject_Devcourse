(() => {
    const { ArgumentError } = require('./exceptions');
    const serverProxy = require('./server-proxy');

    class Robot {
        constructor(initialData) {
            const data = {
                id: undefined,
                hostname: undefined,
                temperature: undefined,
                lmotor_speed: undefined,
                rmotor_speed: undefined,
                lmotor_state: undefined,
                rmotor_state: undefined,
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
                    lmotorSpeed: {
                        get: () => data.lmotor_speed,
                        set: (value) => {
                            updatedFields.lmotor_speed = true;
                            data.lmotor_speed = value;
                        },
                    },
                    rmotorSpeed: {
                        get: () => data.rmotor_speed,
                        set: (value) => {
                            updatedFields.rmotor_speed = true;
                            data.rmotor_speed = value;
                        },
                    },
                    lmotorState: {
                        get: () => data.lmotor_state,
                    },
                    lmotorState: {
                        get: () => data.lmotor_state,
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
            const result = await serverProxy.robots.saveRobot(this.id, this.toJSON(), which);
            return result;
        }

        async setLeftMotorSpeed() {
            const result = await serverProxy.robots.setLeftMotorSpeed(this.id, this.lmotorSpeed);
            return result;
        }

        async setRightMotorSpeed() {
            const result = await serverProxy.robots.setLeftMotorSpeed(this.id, this.rmotorSpeed);
            return result;
        }

        async haltLeftMotor() {
            const result = await serverProxy.robots.haltLeftMotor(this.id);
            return result;
        }

        async haltRightMotor() {
            const result = await serverProxy.robots.haltRightMotor(this.id);
            return result;
        }
    }

    module.exports = {
        Robot,
    };

})();
