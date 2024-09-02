package settings

import (
	"toy/pkg/users"
)

// UserDefaults is a type that holds the default values
// for some fields on User.
type UserDefaults struct {
	Scope       string            `json:"scope"`
	Locale      string            `json:"locale"`
	ViewMode    users.ViewMode    `json:"viewMode"`
	SingleClick bool              `json:"singleClick"`
	Perm        users.Permissions `json:"perm"`
	Commands    []string          `json:"commands"`
}

// Apply applies the default options to a user.
func (d *UserDefaults) Apply(u *users.User) {
	u.Scope = d.Scope
	u.Locale = d.Locale
	u.ViewMode = d.ViewMode
	u.SingleClick = d.SingleClick
	u.Perm = d.Perm
	u.Commands = d.Commands
}
