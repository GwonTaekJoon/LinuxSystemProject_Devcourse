import { AnyAction } from 'redux';

import { RobotsActionTypes } from 'actions/robots-actions';
import { NotificationsActionType } from 'actions/notification-actions';

import { NotificationsState } from './interfaces';

const defaultState: NotificationsState = {
    errors: {
        robots: {
            fetching: null,
            updating: null,
        },
    },
    messages: {
        robots: {
            cancelingDone: '',
            pausingDone: '',
            resumingDone: '',
        },
    },
};

export default function (state = defaultState, action: AnyAction): NotificationsState {
    switch (action.type) {
        case RobotsActionTypes.GET_ROBOTS_FAILED: {
            return {
                ...state,
                errors: {
                    ...state.errors,
                    robots: {
                        ...state.errors.robots,
                        fetching: {
                            message: 'Could not fetch robots',
                            reason: action.payload.error.toString(),
                        },
                    },
                },
            };
        }
        case RobotsActionTypes.UPDATE_ROBOT_FAILED: {
            const robotID = action.payload.robot.id;
            return {
                ...state,
                errors: {
                    ...state.errors,
                    robots: {
                        ...state.errors.robots,
                        updating: {
                            // eslint-disable-next-line max-len
                            message: `Could not update <a href="/robots/${robotID}" target="_blank">robot ${robotID}</a>`,
                            reason: action.payload.error.toString(),
                            className: 'ans-notification-notice-update-robot-failed',
                        },
                    },
                },
            };
        }
        case NotificationsActionType.RESET_ERRORS: {
            return {
                ...state,
                errors: {
                    ...defaultState.errors,
                },
            };
        }
        case NotificationsActionType.RESET_MESSAGES: {
            return {
                ...state,
                messages: {
                    ...defaultState.messages,
                },
            };
        }
        default: {
            return state;
        }
    }
}
