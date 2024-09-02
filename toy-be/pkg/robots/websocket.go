package robots

import (
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

type Handler func(*Client, interface{})

type Event string

type Router struct {
	hub   *Hub
	rules map[Event]Handler
}

func InitWebSocketService(hub *Hub) {
	go hub.HubStart()
	router := NewWebSocketRouter(hub)
	http.Handle("/", router)
	http.ListenAndServe(":5050", nil)
}

func NewWebSocketRouter(hub *Hub) *Router {
	return &Router{
		rules: make(map[Event]Handler),
		hub:   hub,
	}
}

func (rt *Router) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	upgrader := websocket.Upgrader{
		ReadBufferSize:  1024,
		WriteBufferSize: 1024,
		CheckOrigin:     func(r *http.Request) bool { return true },
	}

	socket, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("socket server configuration error: %v\n", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	client := NewClient(rt.hub, socket, rt.FindHandler)
	client.hub.register <- client

	go client.Read()
}

func (rt *Router) FindHandler(event Event) (Handler, bool) {
	handler, found := rt.rules["helloFromClient"]
	return handler, found
}

func (rt *Router) Handle(event Event, handler Handler) {
	rt.rules[event] = handler
}
