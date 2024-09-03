package robots

import (
	"encoding/json"
	"log"
)

type BroadcastRobotState struct {
	RobotId     uint64 `json:"id"`
	Hostname    string `json:"hostname"`
	Temperature string `json:"temperature"`
}

var errLog *log.Logger

func checkErr(err error, errorHappenPlace string) {
	if err != nil {
		errLog.Println(errorHappenPlace)
		// panic(err)
		errLog.Println(err)
	}
}

func BroadcastRobot(hub *Hub, id uint64, hostname string, temperature string) {

	state := &BroadcastRobotState{
		RobotId:     id,
		Hostname:    hostname,
		Temperature: temperature,
	}
	log.Printf("id: %d, hostname: %s, temerature: %s", id, hostname, temperature)
	marsh, err := json.Marshal(state)
	checkErr(err, "BroadcastRobotState: json marshal")
	// send status message to web clients
	hub.BroadcastRobotStateMessage(marsh)
}
