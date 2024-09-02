import React from 'react';
import ReactDOM from 'react-dom';
import { connect, Provider } from 'react-redux';
import { BrowserRouter } from 'react-router-dom';
import createtoyStore, { getTOYStore } from 'toy-store';
import toyApplication from 'components/toy-app';
import createRootReducer from 'reducers/root-reducer';
import { resetErrors, resetMessages } from './actions/notification-actions';
import { CombinedState, NotificationsState } from './reducers/interfaces';

createtoyStore(createRootReducer);
const toyStore = getTOYStore();

interface StateToProps {
    notifications: NotificationsState
}

interface DispatchToProps {
    resetErrors: () => void;
    resetMessages: () => void
}

function mapStateToProps(state: CombinedState): StateToProps {
    return {
        notifications: state.notifications,
    };
}

function mapDispatchToProps(dispatch: any): DispatchToProps {
    return {
        resetErrors: (): void => dispatch(resetErrors()),
        resetMessages: (): void => dispatch(resetMessages()),
    };
}

const ReduxAppWrapper = connect(mapStateToProps, mapDispatchToProps)(toyApplication);

ReactDOM.render(
    <Provider store={toyStore}>
        <BrowserRouter>
            <ReduxAppWrapper />
        </BrowserRouter>
    </Provider>,
    document.getElementById('root'),
);
