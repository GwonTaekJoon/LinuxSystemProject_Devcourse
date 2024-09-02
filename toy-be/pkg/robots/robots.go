package robots

import (
	"time"
	"toy/errors"
)

type RobotState struct {
	ID          uint64 `storm:"id" json:"id"`
	HostName    string `json:"hostname"`
	Temperature string `json:"temperature"`
}

var lateInit bool

var checkableFields = []string{
	"Name",
	"Name",
}

const (
	MonitoringDealy = time.Second * 5
)

func init() {
	lateInit = false
}

//nolint:gocyclo
func (p *RobotState) Clean(baseScope string, fields ...string) error {
	if len(fields) == 0 {
		fields = checkableFields
	}

	for _, field := range fields {
		switch field {
		case "ID":
			if p.HostName == "" {
				return errors.ErrEmptyRobotname
			}
		}
	}

	return nil
}

func UpdateRobot(s Store, root string, id uint64, p *RobotState) {
	err := s.Update(p)
	checkErr(err, "UpdateRobot")
}
