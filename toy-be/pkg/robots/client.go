package robots

import (
	"log"
	"sync/atomic"
	"time"

	"github.com/gorilla/websocket"
)

type Message struct {
	Name string      `json:"name"`
	Data interface{} `json:"data"`
}

type FindHandler func(Event) (Handler, bool)

type Client struct {
	sessionExpiresAt int64
	send             Message
	socket           *websocket.Conn
	findHandler      FindHandler
	Locale           string
	Sequence         int64
	UserId           string
	sessionToken     atomic.Value
	session          atomic.Value
	hub              *Hub
}

func NewClient(hub *Hub, socket *websocket.Conn, findHandler FindHandler) *Client {
	return &Client{
		hub:         hub,
		socket:      socket,
		findHandler: findHandler,
	}
}

const (
	// Time allowed to write a message to the peer.
	writeWait = 10 * time.Second
	// Time allowed to read the next pong message from the peer.
	pongWait = 60 * time.Second
	// Send pings to peer with this period. Must be less than pongWait.
	pingPeriod = (pongWait * 9) / 10
	// Maximum message size allowed from peer.
	maxMessageSize = 512
)

var (
	newline = []byte{'\n'}
	space   = []byte{' '}
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

const (
	broadcastQueueSize = 4096
)

func (c *Client) Write() {
	msg := c.send
	err := c.socket.WriteJSON(msg)
	if err != nil {
		log.Printf("socket write error: %v\n", err)
	}
}

func (c *Client) HelloFromClient() {
	log.Printf("hello from client! message: \n")

	c.send = Message{Name: "helloFromServer", Data: "hello client!"}
	c.Write()
}

func (c *Client) Read() {
	var msg Message
	for {
		if err := c.socket.ReadJSON(&msg); err != nil {
			log.Printf("socket read error: %v\n", err)
			break
		}
		if handler, found := c.findHandler(Event(msg.Name)); found {
			handler(c, msg.Data)
		}
	}
	log.Println("exiting read loop")

	// unregister web socket hub
	c.hub.unregister <- c
	c.socket.Close()
}
