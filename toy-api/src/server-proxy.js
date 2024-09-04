(() => {
    const { ServerError } = require('./exceptions');
    const store = require('store');
    const config = require('./config');

    function generateError(errorData) {
        if (errorData.response) {
            const message = `${errorData.message}. ${JSON.stringify(errorData.response.data) || ''}.`;
            return new ServerError(message, errorData.response.status);
        }

        const message = `${errorData.message}.`;
        return new ServerError(message, 0);
    }

    class ServerProxy {
        constructor() {
            const Axios = require('axios');
            Axios.defaults.withCredentials = true;
            Axios.defaults.xsrfHeaderName = 'X-CSRFTOKEN';
            Axios.defaults.xsrfCookieName = 'csrftoken';

            let token = store.get('token');
            if (token) {
                Axios.defaults.headers.common.Authorization = `Token ${token}`;
            }

            async function exception(exceptionObject) {
                const { backendAPI } = config;

                try {
                    await Axios.post(`${backendAPI}/server/exception`, JSON.stringify(exceptionObject), {
                        proxy: config.proxy,
                        headers: {
                            'Content-Type': 'application/json',
                        },
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            async function serverRequest(url, data) {
                try {
                    return (
                        await Axios({
                            url,
                            ...data,
                        })
                    ).data;
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            async function getRobots(filter = '') {
                const { backendAPI } = config;

                let response = null;
                try {
                    response = await Axios.get(`${backendAPI}/robots`, {
                        proxy: config.proxy,
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }

                response.data.results.count = response.data.count;
                return response.data.results;
            }

            async function saveRobot(id, RobotData, which = ['all']) {
                const { backendAPI } = config;
                const body = JSON.stringify({
                    what: 'robot',
                    which,
                    data: RobotData,
                });

                try {
                    await Axios.put(`${backendAPI}/robots/${id}`, body, {
                        proxy: config.proxy,
                        headers: {
                            'Content-Type': 'application/json',
                        },
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            async function setLeftMotorSpeed(id, speed) {
                const { backendAPI } = config;
                const body = JSON.stringify({
                    speed,
                });

                try {
                    await Axios.put(`${backendAPI}/robots/${id}/lspeed`, body, {
                        proxy: config.proxy,
                        headers: {
                            'Content-Type': 'application/json',
                        },
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            async function setRightMotorSpeed(id, speed) {
                const { backendAPI } = config;
                const body = JSON.stringify({
                    speed,
                });

                try {
                    await Axios.put(`${backendAPI}/robots/${id}/rspeed`, body, {
                        proxy: config.proxy,
                        headers: {
                            'Content-Type': 'application/json',
                        },
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            async function haltLeftMotor(id) {
                const { backendAPI } = config;
                const body = JSON.stringify({
                    op: 'Halt',
                    motor: 'Left',
                });

                try {
                    await Axios.put(`${backendAPI}/robots/${id}/halt`, body, {
                        proxy: config.proxy,
                        headers: {
                            'Content-Type': 'application/json',
                        },
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            async function haltRightMotor(id) {
                const { backendAPI } = config;
                const body = JSON.stringify({
                    op: 'Halt',
                    motor: 'Right',
                });

                try {
                    await Axios.put(`${backendAPI}/robots/${id}/halt`, body, {
                        proxy: config.proxy,
                        headers: {
                            'Content-Type': 'application/json',
                        },
                    });
                } catch (errorData) {
                    throw generateError(errorData);
                }
            }

            Object.defineProperties(
                this,
                Object.freeze({
                    server: {
                        value: Object.freeze({
                            exception,
                            request: serverRequest,
                        }),
                        writable: false,
                    },
                    robots: {
                        value: Object.freeze({
                            getRobots,
                            saveRobot,
                            setLeftMotorSpeed,
                            setRightMotorSpeed,
                            haltLeftMotor,
                            haltRightMotor,
                        }),
                        writable: false,
                    },
                }),
            );
        }
    }

    const serverProxy = new ServerProxy();
    module.exports = serverProxy;
})();
