import '../style/toy.css';
import Layout from 'antd/lib/layout';
import React from 'react';
import { Redirect, Route, Switch } from 'react-router';
import { RouteComponentProps, withRouter } from 'react-router-dom';
import { Col, Modal, Row } from 'antd';
import notification from 'antd/lib/notification';
import Text from 'antd/lib/typography/Text';
import HeaderComponent from 'components/header/header';
import RobotPageContainer from 'components/robot-page/index';
import { NotificationsState } from 'reducers/interfaces';
import showPlatformNotification, {
    platformInfo,
    stopNotifications,
    showUnsupportedNotification,
} from 'utils/platform-checker';
import '../styles.scss';

interface TOYAppProps {
    resetErrors: () => void;
    resetMessages: () => void;
    notifications: NotificationsState;
}

class TOYApplication extends React.PureComponent<TOYAppProps & RouteComponentProps> {
    public componentDidMount(): void {
        const {
            name, version, engine, os,
        } = platformInfo();

        if (showPlatformNotification()) {
            stopNotifications(false);
            Modal.warning({
                title: 'unsupported platform detected',
                className: 'toy-modal-unsupported-platform-warning',
                content: (
                    <>
                        <Row>
                            <Col>
                                <Text>
                                    {`The browser you are using is ${name} ${version} based on ${engine}.` +
                                        'We recommend to use Chrome (or another Chromium based browser)'}
                                </Text>
                            </Col>
                        </Row>
                        <Row>
                            <Col>
                                <Text type='secondary'>{`The operating system is ${os}`}</Text>
                            </Col>
                        </Row>
                    </>
                ),
                onOk: () => stopNotifications(true),
            });
        } else if (showUnsupportedNotification()) {
            stopNotifications(false);
            Modal.warning({
                title: 'Unsupported features detected',
                className: 'toy-modal-unsupported-features-warning',
                content: (
                    <Text>
                        {`${name} v${version} does not support API, which is used by 3dp Monitor. `}
                        It is strongly recommended to update your browser.
                    </Text>
                ),
                onOk: () => stopNotifications(true),
            });
        }
    }

    public componentDidUpdate(): void {
        this.showErrors();
        this.showMessages();
    }

    private showMessages(): void {
        function showMessage(title: string): void {
            notification.info({
                message: (
                    <div
                        // eslint-disable-next-line
                        dangerouslySetInnerHTML={{
                            __html: title,
                        }}
                    />
                ),
                placement: 'topLeft',
                duration: 4.5,
            });
        }

        const { notifications, resetMessages } = this.props;

        let shown = false;
        for (const where of Object.keys(notifications.messages)) {
            for (const what of Object.keys((notifications as any).messages[where])) {
                const message = (notifications as any).messages[where][what];
                shown = shown || !!message;
                if (message) {
                    showMessage(message);
                }
            }
        }

        if (shown) {
            resetMessages();
        }
    }

    private showErrors(): void {
        function showError(title: string, _error: any, className?: string): void {
            const error = _error.toString();
            const dynamicProps = typeof className === 'undefined' ? {} : { className };
            notification.error({
                ...dynamicProps,
                message: (
                    <div
                        // eslint-disable-next-line
                        dangerouslySetInnerHTML={{
                            __html: title,
                        }}
                    />
                ),
                duration: null,
                placement: 'topLeft',
                description: error.length > 200 ? 'Open the Browser Console to get details' : error,
            });

            // eslint-disable-next-line no-console
            console.error(error);
        }

        const { notifications, resetErrors } = this.props;

        let shown = false;
        for (const where of Object.keys(notifications.errors)) {
            for (const what of Object.keys((notifications as any).errors[where])) {
                const error = (notifications as any).errors[where][what];
                shown = shown || !!error;
                if (error) {
                    showError(error.message, error.reason, error.className);
                }
            }
        }

        if (shown) {
            resetErrors();
        }
    }

    public render(): JSX.Element {
        const {
            location,
        } = this.props;

        return (
            <Layout>
                <HeaderComponent />
                <Layout.Content style={{ height: '100%' }}>
                    <Switch>
                        <Route exact path='/robot' component={RobotPageContainer} />
                        <Redirect
                            push
                            to={new URLSearchParams(location.search).get('next') || '/robot'}
                        />
                    </Switch>
                </Layout.Content>
            </Layout>
        );
    }
}

export default withRouter(TOYApplication);
