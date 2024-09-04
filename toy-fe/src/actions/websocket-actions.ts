import { ThunkAction } from 'utils/redux';
import Socket from 'utils/socket';
import { getTOYStore } from 'toy-store';
import {
    updateRobotState,
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
    getTOYStore().dispatch(updateRobotState(+msg.id, msg.hostname, msg.temperature));
};

export const webSocketActionsAsync = (): ThunkAction => async () => {
    const url = new URL(window.location.href);
    console.log('hostname:', url.hostname);
    const ws = new WebSocket(`ws://${url.hostname}:5050`);

    const socket = new Socket(ws);

    // // 핸들러 등록
    socket.on(SocketEvents.CONNECT, handleConnect);
    socket.on(SocketEvents.DISCONNECT, handleDisconnect);
    socket.on(SocketEvents.STATUS_CHANGED, handleStatusChanged);
};
