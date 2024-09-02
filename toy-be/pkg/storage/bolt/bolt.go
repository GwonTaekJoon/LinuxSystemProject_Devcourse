package bolt

import (
	"github.com/asdine/storm"

	"toy/pkg/auth"
	"toy/pkg/robots"
	"toy/pkg/settings"
	"toy/pkg/storage"
	"toy/pkg/users"
)

// NewStorage creates a storage.Storage based on Bolt DB.
func NewStorage(db *storm.DB) (*storage.Storage, error) {
	robotStore := robots.NewStorage(robotsBackend{db: db})
	userStore := users.NewStorage(usersBackend{db: db})
	settingsStore := settings.NewStorage(settingsBackend{db: db})
	authStore := auth.NewStorage(authBackend{db: db}, userStore)

	err := save(db, "version", 3)
	if err != nil {
		return nil, err
	}

	return &storage.Storage{
		Auth:     authStore,
		Users:    userStore,
		Robots:   robotStore,
		Settings: settingsStore,
	}, nil
}
