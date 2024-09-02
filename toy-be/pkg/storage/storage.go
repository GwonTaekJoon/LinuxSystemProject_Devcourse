package storage

import (
	"toy/pkg/auth"
	"toy/pkg/robots"

	"toy/pkg/settings"
	"toy/pkg/users"
)

// Storage is a storage powered by a Backend which makes the necessary
// verifications when fetching and saving data to ensure consistency.
type Storage struct {
	Robots   robots.Store
	Users    users.Store
	Auth     *auth.Storage
	Settings *settings.Storage
}
