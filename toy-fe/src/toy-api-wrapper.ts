import _toy from 'toy-api/src/api';

const toy: any = _toy;

toy.config.backendAPI =
    typeof process.env.REACT_APP_API_URL === 'undefined' ? '/api/v1' : `${process.env.REACT_APP_API_URL}/api/v1`;

toy.config.backendSocket =
    typeof process.env.REACT_APP_SOCKET_URL === 'undefined' ? '' : `${process.env.REACT_APP_SOCKET_URL}`;

export default function getAPI(): any {
    return toy;
}
