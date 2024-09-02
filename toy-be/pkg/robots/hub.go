package robots

import (
	"encoding/json"
	"log"
)

type Hub struct {
	clients             map[*Client]bool
	boradcastRobotState chan []byte
	register            chan *Client
	unregister          chan *Client
}

func NewWebSocketHub() *Hub {
	return &Hub{
		clients:             make(map[*Client]bool),
		boradcastRobotState: make(chan []byte),
		register:            make(chan *Client),
		unregister:          make(chan *Client),
	}
}

func (h *Hub) BroadcastRobotStateMessage(message []byte) {
	h.boradcastRobotState <- message
}

func (h *Hub) HubStart() {
	for {
		select {
		case client := <-h.register:
			h.clients[client] = true
		case client := <-h.unregister:
			if _, ok := h.clients[client]; ok {
				delete(h.clients, client)
			}
		case message := <-h.boradcastRobotState:
			var t BoradCastRobotState
			for client := range h.clients {
				err := json.Unmarshal(message, &t)
				checkErr(err, "HubStart: broadcast json unmarshal")
				// log.Print("status change:")
				log.Println(t)
				client.send = Message{Name: "status_change", Data: t}
				client.Write()
				// log.Println(message)
			}
		}
	}
}
