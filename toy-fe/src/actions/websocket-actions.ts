import { ThunkAction } from 'utils/redux';
import Socket from 'utils/socket';
import { getTOYStore } from 'toy-store';
import {
    updateRobotStateAsync,
} from 'actions/robots-actions';

export const SocketEvents = {
    CONNECT: 'conntect',
    DISCONNECT: 'disconnect',
    DIRECT_ADDED: 'direct_added',
    STATUS_CHANGED: 'status_change',
};

// eslint-disable-next-line @typescript-eslint/explicit-function-return-type
const handleConnect = () => {
    console.log('onConnect...');
};

// eslint-disable-next-line @typescript-eslint/explicit-function-return-type
const handleDisconnect = () => {
    console.log('onDisconnect...');
    setTimeout(window.location.reload.bind(window.location), 5000);
};

// eslint-disable-next-line @typescript-eslint/explicit-function-return-type
const handleStatusChanged = (msg: any) => {
    console.log('Handle status info:', msg);
    // console.log('Handle status robot id:', msg.robot_id);
    // console.log('Handle status robot_state:', msg.robot_state);
    // getTOYStore().dispatch(updateRobotStateAsync(msg.robot_id, msg.robot_state));
};

export const webSocketActionsAsync = (): ThunkAction => async () => {
    const url = new URL(window.location.href);
    const ws = new WebSocket(`ws://${url.hostname}:5050`);

    const socket = new Socket(ws);

    // // 핸들러 등록
    socket.on(SocketEvents.CONNECT, handleConnect);
    socket.on(SocketEvents.DISCONNECT, handleDisconnect);
    socket.on(SocketEvents.STATUS_CHANGED, handleStatusChanged);
};
