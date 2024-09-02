package auth

import (
	"toy/pkg/settings"
	"toy/pkg/users"
)

type StorageBackend interface {
	Get(settings.AuthMethod) (Auther, error)
	Save(Auther) error
}

type Storage struct {
	back  StorageBackend
	users *users.Storage
}

func NewStorage(back StorageBackend, userStore *users.Storage) *Storage {
	return &Storage{back: back, users: userStore}
}

func (s *Storage) Get(t settings.AuthMethod) (Auther, error) {
	return s.back.Get(t)
}

func (s *Storage) Save(a Auther) error {
	return s.back.Save(a)
}
