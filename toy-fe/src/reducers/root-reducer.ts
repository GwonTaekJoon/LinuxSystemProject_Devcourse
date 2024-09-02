import { combineReducers, Reducer } from 'redux';
import robotsReducer from './robots-reducer';
import notificationsReducer from './notifications-reducer';

export default function createRootReducer(): Reducer {
    return combineReducers({
        robots: robotsReducer,
        notifications: notificationsReducer,
    });
}
