package bolt

import (
	"reflect"

	"github.com/asdine/storm"

	"toy/errors"
	"toy/pkg/robots"
)

type robotsBackend struct {
	db *storm.DB
}

func (st robotsBackend) GetBy(i interface{}) (robot *robots.RobotState, err error) {
	robot = &robots.RobotState{}

	var arg string
	switch i.(type) {
	case uint64:
		arg = "ID"
	case string:
		arg = "Robotname"
	default:
		return nil, errors.ErrInvalidDataType
	}

	err = st.db.One(arg, i, robot)

	if err != nil {
		if err == storm.ErrNotFound {
			return nil, errors.ErrNotExist
		}
		return nil, err
	}

	return
}

func (st robotsBackend) Gets() ([]*robots.RobotState, error) {
	var allRobots []*robots.RobotState
	err := st.db.All(&allRobots)
	if err == storm.ErrNotFound {
		return nil, errors.ErrNotExist
	}

	if err != nil {
		return allRobots, err
	}

	return allRobots, err
}

func (st robotsBackend) Update(robot *robots.RobotState, fields ...string) error {
	if len(fields) == 0 {
		return st.Save(robot)
	}

	for _, field := range fields {
		val := reflect.ValueOf(robot).Elem().FieldByName(field).Interface()
		if err := st.db.UpdateField(robot, field, val); err != nil {
			return err
		}
	}

	return nil
}

func (st robotsBackend) Save(robot *robots.RobotState) error {
	err := st.db.Save(robot)
	if err == storm.ErrAlreadyExists {
		return errors.ErrExist
	}
	return err
}

func (st robotsBackend) DeleteByID(id uint64) error {
	return st.db.DeleteStruct(&robots.RobotState{ID: id})
}

func (st robotsBackend) DeleteByRobotname(robotname string) error {
	robot, err := st.GetBy(robotname)
	if err != nil {
		return err
	}

	return st.db.DeleteStruct(robot)
}
