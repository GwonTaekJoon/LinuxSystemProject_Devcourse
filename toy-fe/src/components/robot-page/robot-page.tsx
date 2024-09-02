import './styles.scss';
import React from 'react';
import { RouteComponentProps } from 'react-router';
import { withRouter } from 'react-router-dom';
import { Row, Col } from 'antd/lib/grid';
import Spin from 'antd/lib/spin';
import Result from 'antd/lib/result';

import RobotInfoComponent from 'components/robot-page/robot-info';
import { Robot } from 'reducers/interfaces';
import TopBarComponent from './top-bar';

interface RobotPageComponentProps {
    robot: Robot | null | undefined;
    fetching: boolean;
    updating: boolean;
    getRobot: () => void;
    webSocketInitialized: () => void;
}

type Props = RobotPageComponentProps & RouteComponentProps<{ id: string }>;

class RobotPageComponent extends React.PureComponent<Props> {
    public componentDidMount(): void {
        const {
            robot,
            fetching,
            getRobot,
            webSocketInitialized,
        } = this.props;

        if (robot === undefined && !fetching) {
            getRobot();
            webSocketInitialized();
        }
    }

    public componentDidUpdate(): void {
        const {
            robot, fetching, getRobot,
        } = this.props;

        if (robot === null && !fetching) {
            getRobot();
        }
    }

    public render(): JSX.Element {
        const { robot, updating } = this.props;

        if (robot === null) {
            return <Spin size='large' className='toy-spinner' />;
        }

        if (typeof robot === 'undefined') {
            return (
                <Result
                    className='toy-not-found'
                    status='404'
                    title='Sorry, but this robot was not found'
                    subTitle='Please, be sure information you tried to get exist and you have access'
                />
            );
        }

        return (
            <>
                <Row
                    style={{ display: updating ? 'none' : undefined }}
                    justify='center'
                    align='top'
                    className='toy-robot-details-wrapper'
                >
                    <Col md={22} lg={18} xl={16} xxl={14}>
                        <TopBarComponent />
                        <RobotInfoComponent robot={robot as Robot} />
                    </Col>
                </Row>
                {updating && <Spin size='large' className='toy-spinner' />}
            </>
        );
    }
}

export default withRouter(RobotPageComponent);
