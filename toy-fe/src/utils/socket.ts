import { EventEmitter } from 'events';

export default class Socket {
    ws: any;
    ee: any;
    constructor(ws:any, ee = new EventEmitter()) {
        this.ws = ws;
        this.ee = ee;
        this.ws.onmessage = this.message.bind(this);
        this.ws.onopen = this.open.bind(this);
        this.ws.onclose = this.close.bind(this);
        this.ws.onerror = this.error.bind(this);
    }

    // eslint-disable-next-line @typescript-eslint/explicit-function-return-type
    on(name:any, fn:any) {
        this.ee.on(name, fn);
    }

    // eslint-disable-next-line @typescript-eslint/explicit-function-return-type
    off(name:any, fn:any) {
        this.ee.removeListener(name, fn);
    }

    // eslint-disable-next-line @typescript-eslint/explicit-function-return-type
    open() {
        this.ee.emit('connect');
    }

    close() {
        this.ee.emit('disconnect');
    }

    // eslint-disable-next-line class-methods-use-this
    error(e:any) {
        console.log('websocket error: ', e);
    }

    emit(name:any, data:any) {
        const message = JSON.stringify({ name, data });
        this.ws.send(message);
    }

    message(e:any) {
        try {
            const message = JSON.parse(e.data);
            this.ee.emit(message.name, message.data);
        } catch (err) {
            this.ee.emit('error', err);
            // console.log(`${Date().toString()}: `, err);
        }
    }
}
