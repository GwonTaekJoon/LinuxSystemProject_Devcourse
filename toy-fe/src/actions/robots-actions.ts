import { AnyAction, Dispatch, ActionCreator } from 'redux';
import { ThunkAction } from 'redux-thunk';
import { RobotsQuery, CombinedState } from 'reducers/interfaces';
import getAPI from 'toy-api-wrapper';

const api = getAPI();

export enum RobotsActionTypes {
    GET_ROBOTS = 'GET_ROBOTS',
    GET_ROBOTS_SUCCESS = 'GET_ROBOTS_SUCCESS',
    GET_ROBOTS_FAILED = 'GET_ROBOTS_FAILED',
    UPDATE_ROBOT = 'UPDATE_ROBOT',
    UPDATE_ROBOT_SUCCESS = 'UPDATE_ROBOT_SUCCESS',
    UPDATE_ROBOT_FAILED = 'UPDATE_ROBOT_FAILED',
    UPDATE_ROBOT_STATE = 'UPDATE_ROBOT_STATE',
    SET_LEFT_MOTOR_SPEED = 'SET_LEFT_MOTOR_SPEED',
    SET_LEFT_MOTOR_SPEED_SUCCESS = 'SET_LEFT_MOTOR_SPEED_SUCCESS',
    SET_LEFT_MOTOR_SPEED_FAILED = 'SET_LEFT_MOTOR_SPEED_FAILED',
}

function getRobots(): AnyAction {
    const action = {
        type: RobotsActionTypes.GET_ROBOTS,
        payload: {},
    };

    return action;
}

export function getRobotsSuccess(array: any[], count: number,
    query: RobotsQuery): AnyAction {
    const action = {
        type: RobotsActionTypes.GET_ROBOTS_SUCCESS,
        payload: {
            array,
            count,
            query,
        },
    };

    return action;
}

function getRobotsFailed(error: any, query: RobotsQuery): AnyAction {
    const action = {
        type: RobotsActionTypes.GET_ROBOTS_FAILED,
        payload: {
            error,
            query,
        },
    };

    return action;
}

export function getRobotsAsync(query: RobotsQuery): ThunkAction<Promise<void>, {}, {}, AnyAction> {
    return async (dispatch: ActionCreator<Dispatch>): Promise<void> => {
        dispatch(getRobots());

        const filteredQuery = { ...query };
        for (const key in filteredQuery) {
            if (filteredQuery[key] === null) {
                delete filteredQuery[key];
            }
        }

        let result = null;
        try {
            result = await api.robots.get(filteredQuery);
        } catch (error) {
            dispatch(getRobotsFailed(error, query));
            return;
        }

        const array = Array.from(result);
        // console.log(array);

        dispatch(getRobotsSuccess(array, result.count, query));
    };
}

function updateRobot(): AnyAction {
    const action = {
        type: RobotsActionTypes.UPDATE_ROBOT,
        payload: {},
    };

    return action;
}

export function updateRobotSuccess(robot: any, robotID: number): AnyAction {
    const action = {
        type: RobotsActionTypes.UPDATE_ROBOT_SUCCESS,
        payload: { robot, robotID },
    };

    return action;
}

function updateRobotFailed(error: any, robot: any): AnyAction {
    const action = {
        type: RobotsActionTypes.UPDATE_ROBOT_FAILED,
        payload: { error, robot },
    };

    return action;
}

export function updateRobotAsync(robotInstance: any,
    which: any[]): ThunkAction<Promise<void>, CombinedState, {}, AnyAction> {
    return async (dispatch: ActionCreator<Dispatch>): Promise<void> => {
        try {
            dispatch(updateRobot());
            await robotInstance.save(which);
            const [robot] = await api.robots.get({ id: robotInstance.id });
            dispatch(updateRobotSuccess(robot, robotInstance.id));
        } catch (error) {
            let robot = null;
            try {
                [robot] = await api.robots.get({ id: robotInstance.id });
            } catch (fetchError) {
                dispatch(updateRobotFailed(error, robotInstance));
                return;
            }

            dispatch(updateRobotFailed(error, robot));
        }
    };
}

function setLeftMotorSpeed(): AnyAction {
    const action = {
        type: RobotsActionTypes.SET_LEFT_MOTOR_SPEED,
    };

    return action;
}

export function setLeftMotorSpeedSuccess(robot: any): AnyAction {
    const action = {
        type: RobotsActionTypes.SET_LEFT_MOTOR_SPEED_SUCCESS,
        payload: { robot },
    };

    return action;
}

function setLeftMotorSpeedFailed(error: any): AnyAction {
    const action = {
        type: RobotsActionTypes.SET_LEFT_MOTOR_SPEED_FAILED,
        payload: { error },
    };

    return action;
}

export function setLeftMotorSpeedAsync(robotInstance: any,
    speed: number): ThunkAction<Promise<void>, CombinedState, {}, AnyAction> {
    return async (dispatch: ActionCreator<Dispatch>): Promise<void> => {
        try {
            dispatch(setLeftMotorSpeed());
            robotInstance.lmotorSpeed = speed;
            await robotInstance.setLeftMotorSpeed();
            dispatch(setLeftMotorSpeedSuccess(robotInstance));
        } catch (error) {
            dispatch(setLeftMotorSpeedFailed(error));
        }
    };
}

export function updateRobotState(id: number,
    hostname: string,
    temperature: string): AnyAction {
    const action = {
        type: RobotsActionTypes.UPDATE_ROBOT_STATE,
        payload: {
            id,
            hostname,
            temperature,
        },
    };

    return action;
}
