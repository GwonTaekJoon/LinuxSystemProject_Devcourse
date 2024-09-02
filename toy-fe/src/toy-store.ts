import thunk from 'redux-thunk';
import {
    createStore, applyMiddleware, Store, Reducer,
} from 'redux';
import { isDev } from 'utils/enviroment';

const middlewares = [thunk];

let store: Store | null = null;

export default function createTOYStore(createRootReducer: () => Reducer): void {
    let appliedMiddlewares = applyMiddleware(...middlewares);

    if (isDev()) {
        // eslint-disable-next-line @typescript-eslint/no-var-requires, global-require
        const { composeWithDevTools } = require('redux-devtools-extension');

        appliedMiddlewares = composeWithDevTools(appliedMiddlewares);
    }

    store = createStore(createRootReducer(), appliedMiddlewares);
}

export function getTOYStore(): Store {
    if (store) {
        return store;
    }

    throw new Error('First create a store');
}
