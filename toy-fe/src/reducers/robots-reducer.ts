import { AnyAction } from 'redux';
import { RobotsActionTypes } from 'actions/robots-actions';

import {
    RobotsState,
    Robot,
} from './interfaces';

const defaultState: RobotsState = {
    initialized: false,
    fetching: false,
    updating: false,
    hideEmpty: false,
    count: 0,
    current: [],
    gettingQuery: {
        page: 1,
        id: null,
        search: null,
        owner: null,
        assignee: null,
        name: null,
        status: null,
        mode: null,
    },
    activities: {
        dumps: {},
    },
};

export default (state: RobotsState = defaultState, action: AnyAction): RobotsState => {
    switch (action.type) {
        case RobotsActionTypes.GET_ROBOTS:
            return {
                ...state,
                activities: {
                    ...state.activities,
                },
                initialized: false,
                fetching: true,
                hideEmpty: true,
                count: 0,
                current: [],
                gettingQuery: { ...action.payload.query },
            };
        case RobotsActionTypes.GET_ROBOTS_SUCCESS: {
            const combined = action.payload.array.map(
                (robot: any): Robot => ({
                    instance: robot,
                    state: {
                        id: robot.id,
                        hostname: robot.hostname,
                        temperature: robot.temperature,
                    },
                }),
            );

            return {
                ...state,
                initialized: true,
                fetching: false,
                count: action.payload.count,
                current: combined,
                gettingQuery: { ...action.payload.query },
            };
        }
        case RobotsActionTypes.GET_ROBOTS_FAILED:
            return {
                ...state,
                initialized: true,
                fetching: false,
            };
        case RobotsActionTypes.UPDATE_ROBOT: {
            return {
                ...state,
                updating: true,
            };
        }
        case RobotsActionTypes.UPDATE_ROBOT_SUCCESS: {
            const { robot, robotID } = action.payload;

            if (typeof robot === 'undefined') {
                return {
                    ...state,
                    updating: false,
                    current: state.current.filter((_robot: Robot): boolean => _robot.instance.id !== robotID),
                };
            }

            return {
                ...state,
                updating: false,
                current: state.current.map(
                    (_robot): Robot => {
                        if (_robot.instance.id === robot.id) {
                            return {
                                ..._robot,
                                instance: robot,
                                state: {
                                    id: robot.id,
                                    hostname: robot.hostname,
                                    temperature: robot.temperature,
                                },
                            };
                        }

                        return _robot;
                    },
                ),
            };
        }
        case RobotsActionTypes.UPDATE_ROBOT_FAILED: {
            return {
                ...state,
                updating: false,
                current: state.current.map(
                    (robot): Robot => {
                        if (robot.instance.id === action.payload.robot.id) {
                            return {
                                ...robot,
                                instance: action.payload.robot,
                            };
                        }

                        return robot;
                    },
                ),
            };
        }
        case RobotsActionTypes.UPDATE_ROBOT_STATE: {
            const {
                id,
                hostname,
                temperature,
            } = action.payload;

            return {
                ...state,
                current: state.current.map(
                    (robot): Robot => {
                        if (robot.instance.id === id) {
                            return {
                                ...robot,
                                state: {
                                    ...state,
                                    id,
                                    hostname,
                                    temperature,
                                },
                            };
                        }
                        return robot;
                    },
                ),
            };
        }
        case RobotsActionTypes.SET_LEFT_MOTOR_SPEED: {
            return {
                ...state,
                updating: true,
            };
        }
        case RobotsActionTypes.SET_LEFT_MOTOR_SPEED_SUCCESS: {
            return {
                ...state,
                updating: false,
                current: state.current.map(
                    (robot): Robot => {
                        if (robot.instance.id === action.payload.robot.id) {
                            return {
                                ...robot,
                                instance: action.payload.robot,
                            };
                        }

                        return robot;
                    },
                ),
            };
        }
        case RobotsActionTypes.SET_LEFT_MOTOR_SPEED_FAILED: {
            return {
                ...state,
                updating: true,
            };
        }
        default:
            return state;
    }
};
