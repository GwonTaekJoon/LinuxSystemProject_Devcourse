package robots

import (
	"encoding/json"
	"log"
)

type BoradCastRobotState struct {
	RobotId uint64 `json:"robot_id"`
	Robot   string `json:"robot"`
}

var errLog *log.Logger

func checkErr(err error, errorHappenPlace string) {
	if err != nil {
		errLog.Println(errorHappenPlace)
		// panic(err)
		errLog.Println(err)
	}
}

func BroadcastRobotInfo(id uint64, data RobotState, hub *Hub, status string) {

	state := &BoradCastRobotState{
		RobotId: id,
		Robot:   status,
	}
	log.Println(status)
	marsh, err := json.Marshal(state)
	checkErr(err, "BoradCastRobotState: json marshal")
	// send status message to web clients
	hub.BroadcastRobotStateMessage(marsh)
}
