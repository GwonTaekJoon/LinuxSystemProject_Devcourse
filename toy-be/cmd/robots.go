package cmd

import (
	"log"
	"os"
	"strconv"
	"toy/pkg/robots"

	"github.com/spf13/pflag"
)

func InitRobots(hub *robots.Hub, flags *pflag.FlagSet, d pythonData, id string) {
	var err error

	robotList, _ := d.store.Robots.Gets("")
	for _, p := range robotList {
		err = d.store.Robots.Delete(p.ID)
		if err != nil {
			log.Print(err)
		}
	}

	host, _ := os.Hostname()
	name := string(host)

	device_id, _ := strconv.ParseUint(id, 10, 64)

	robot := robots.RobotState{
		ID:          device_id,
		HostName:    name,
		Temperature: "0.0",
		LMotorSpeed: 50,
		RMotorSpeed: 30,
		LMotorState: 1,
		RMotorState: 1,
	}
	err = d.store.Robots.Save(&robot)
	if err != nil {
		log.Print(err)
	}
}
