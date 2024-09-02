import './styles.scss';
import React from 'react';
import Layout from 'antd/lib/layout';
import Icon from '@ant-design/icons';
import { withRouter } from 'react-router';
import {
    TOYLogo,
} from 'icons';

// eslint-disable-next-line @typescript-eslint/no-unused-vars
function HeaderComponent(): JSX.Element {
    return (
        <Layout.Header className='toy-header'>
            <div className='toy-left-header'>
                <Icon className='toy-logo-icon' component={TOYLogo} />
            </div>
        </Layout.Header>
    );
}

export default withRouter(HeaderComponent);
