import React from 'react';
import { Row, Col } from 'antd/lib/grid';
import Text from 'antd/lib/typography/Text';
import Title from 'antd/lib/typography/Title';
import Icon from '@ant-design/icons';
import { Divider } from 'antd';
import { Robot } from 'reducers/interfaces';
import {
    RPIIcon,
} from 'icons';

interface Props {
    robot: Robot;
    lspeed: number;
    setLeftMotorSpeed: (robotInstance: any, speed: number) => void;
}

interface State {
    name: string;
}

class RobotInfoComponent extends React.PureComponent<Props, State> {
    constructor(props: Props) {
        super(props);

        const { robot } = props;

        this.state = {
            name: robot.state.hostname,
        };
    }

    private renderRobotName(): JSX.Element {
        const { name } = this.state;
        const { robot } = this.props;

        return (
            <Title
                level={4}
                editable={{
                    onChange: (value: string): void => {
                        this.setState({
                            name: value,
                        });

                        robot.instance.hostname = value;
                    },
                }}
                className='toy-text-color'
            >
                {name}
            </Title>
        );
    }

    // eslint-disable-next-line class-methods-use-this
    private renderPreview(): JSX.Element {
        return (
            <div className='toy-robot-item-preview-wrapper'>
                <Icon className='toy-logo-icon' component={RPIIcon} />
            </div>
        );
    }

    private renderRobotDescription(): JSX.Element {
        const { robot, lspeed, setLeftMotorSpeed } = this.props;
        const {
            temperature,
        } = robot.state;

        return (
            <Col>
                <Text strong style={{ marginRight: 5 }}>
                    {` * Temperature: ${temperature} ˚C `}
                </Text>
                <Divider />
                <Text strong style={{ marginRight: 5 }}>
                    {' * Left Motor Speed '}
                </Text>
                <Title
                    level={5}
                    editable={{
                        onChange: (value: string): void => {
                            setLeftMotorSpeed(robot.instance, Number(value));
                        },
                    }}
                    className='toy-text-color'
                >
                    {robot.instance.lmotorSpeed}
                </Title>
            </Col>
        );
    }

    public render(): JSX.Element {
        return (
            <div className='toy-robot-details'>
                <Row justify='space-between' align='top'>
                    <Col className='toy-robot-details-robot-name'>
                        {this.renderRobotName()}
                    </Col>
                </Row>
                <Row justify='space-between' align='top'>
                    <Col md={8} lg={7} xl={7} xxl={6}>
                        <Row justify='start' align='middle'>
                            <Col span={24}>{this.renderPreview()}</Col>
                        </Row>
                    </Col>
                    <Col md={16} lg={17} xl={17} xxl={18}>
                        <Row justify='space-between' align='top'>
                            {this.renderRobotDescription()}
                        </Row>
                    </Col>
                </Row>
            </div>
        );
    }
}

export default RobotInfoComponent;
