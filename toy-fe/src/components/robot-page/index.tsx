import { connect } from 'react-redux';
import { withRouter } from 'react-router-dom';
import { RouteComponentProps } from 'react-router';

import { getRobotsAsync } from 'actions/robots-actions';

import RobotPageComponent from 'components/robot-page/robot-page';
import { Robot, CombinedState } from 'reducers/interfaces';
import { webSocketActionsAsync } from 'actions/websocket-actions';

type Props = RouteComponentProps<{ id: string }>;

interface StateToProps {
    robot: Robot | null | undefined;
    fetching: boolean;
    updating: boolean;
}

interface DispatchToProps {
    getRobot: () => void;
    webSocketInitialized: () => void;
}

function mapStateToProps(state: CombinedState, own: Props): StateToProps {
    const { robots } = state;
    const { gettingQuery, fetching, updating } = robots;

    const filteredRobots = state.robots.current;

    const robot = filteredRobots[0];

    return {
        robot,
        fetching,
        updating,
    };
}

function mapDispatchToProps(dispatch: any, own: Props): DispatchToProps {
    const id = +own.match.params.id;

    return {
        getRobot: (): void => {
            dispatch(
                getRobotsAsync({
                    id,
                    page: 1,
                    search: null,
                    owner: null,
                    assignee: null,
                    name: null,
                    status: null,
                    mode: null,
                }),
            );
        },
        webSocketInitialized: (): void => dispatch(webSocketActionsAsync()),
    };
}

export default withRouter(connect(mapStateToProps, mapDispatchToProps)(RobotPageComponent));
