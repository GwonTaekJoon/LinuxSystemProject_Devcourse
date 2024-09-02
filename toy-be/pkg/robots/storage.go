package robots

import (
	"sync"
	"time"
	"toy/errors"
)

type StorageBackend interface {
	GetBy(interface{}) (*RobotState, error)
	Gets() ([]*RobotState, error)
	Save(p *RobotState) error
	Update(p *RobotState, fields ...string) error
	DeleteByID(uint64) error
	DeleteByRobotname(string) error
}

type Store interface {
	Get(baseScope string, id interface{}) (robot *RobotState, err error)
	Gets(baseScope string) ([]*RobotState, error)
	Update(robot *RobotState, fields ...string) error
	Save(robot *RobotState) error
	Delete(id interface{}) error
	LastUpdate(id uint64) int64
}

type Storage struct {
	back    StorageBackend
	updated map[uint64]int64
	mux     sync.RWMutex
}

func NewStorage(back StorageBackend) *Storage {
	return &Storage{
		back:    back,
		updated: map[uint64]int64{},
	}
}

func (s *Storage) Get(baseScope string, id interface{}) (robot *RobotState, err error) {
	robot, err = s.back.GetBy(id)
	if err != nil {
		return
	}
	if err := robot.Clean(baseScope); err != nil {
		return nil, err
	}
	return
}

func (s *Storage) Gets(baseScope string) ([]*RobotState, error) {
	robots, err := s.back.Gets()
	if err != nil {
		return nil, err
	}

	for _, robot := range robots {
		if err := robot.Clean(baseScope); err != nil { //nolint:govet
			return nil, err
		}
	}

	return robots, err
}

func (s *Storage) Update(robot *RobotState, fields ...string) error {
	err := robot.Clean("", fields...)
	if err != nil {
		return err
	}

	err = s.back.Update(robot, fields...)
	if err != nil {
		return err
	}

	s.mux.Lock()
	s.updated[robot.ID] = time.Now().Unix()
	s.mux.Unlock()
	return nil
}

func (s *Storage) Save(robot *RobotState) error {
	if err := robot.Clean(""); err != nil {
		return err
	}

	return s.back.Save(robot)
}

func (s *Storage) Delete(id interface{}) error {
	switch id := id.(type) {
	case string:
		robot, err := s.back.GetBy(id)
		if err != nil {
			return err
		}
		if robot.ID == 1 {
			return errors.ErrRootUserDeletion
		}
		return s.back.DeleteByRobotname(id)
	case uint64:
		if id == 1 {
			return errors.ErrRootUserDeletion
		}
		return s.back.DeleteByID(id)
	default:
		return errors.ErrInvalidDataType
	}
}

func (s *Storage) LastUpdate(id uint64) int64 {
	s.mux.RLock()
	defer s.mux.RUnlock()
	if val, ok := s.updated[id]; ok {
		return val
	}
	return 0
}
