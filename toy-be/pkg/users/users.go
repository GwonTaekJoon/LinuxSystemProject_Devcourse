package users

import (
	"path/filepath"

	"github.com/spf13/afero"

	"toy/errors"
)

// ViewMode describes a view mode.
type ViewMode string

const (
	ListViewMode   ViewMode = "list"
	MosaicViewMode ViewMode = "mosaic"
)

// User describes a user.
type User struct {
	ID           uint        `storm:"id,increment" json:"id"`
	Username     string      `storm:"unique" json:"username"`
	Password     string      `json:"password"`
	Scope        string      `json:"scope"`
	Locale       string      `json:"locale"`
	LockPassword bool        `json:"lockPassword"`
	ViewMode     ViewMode    `json:"viewMode"`
	SingleClick  bool        `json:"singleClick"`
	Perm         Permissions `json:"perm"`
	Commands     []string    `json:"commands"`
	Fs           afero.Fs    `json:"-" yaml:"-"`
}

var checkableFields = []string{
	"Username",
	"Password",
	"Scope",
	"ViewMode",
	"Commands",
	"Sorting",
}

// Clean cleans up a user and verifies if all its fields
// are alright to be saved.
//
//nolint:gocyclo
func (u *User) Clean(baseScope string, fields ...string) error {
	if len(fields) == 0 {
		fields = checkableFields
	}

	for _, field := range fields {
		switch field {
		case "Username":
			if u.Username == "" {
				return errors.ErrEmptyUsername
			}
		case "Password":
			if u.Password == "" {
				return errors.ErrEmptyPassword
			}
		case "ViewMode":
			if u.ViewMode == "" {
				u.ViewMode = ListViewMode
			}
		case "Commands":
			if u.Commands == nil {
				u.Commands = []string{}
			}
		}
	}

	if u.Fs == nil {
		scope := u.Scope

		if !filepath.IsAbs(scope) {
			scope = filepath.Join(baseScope, scope)
		}

		u.Fs = afero.NewBasePathFs(afero.NewOsFs(), scope)
	}

	return nil
}

// FullPath gets the full path for a user's relative path.
func (u *User) FullPath(path string) string {
	return afero.FullBaseFsPath(u.Fs.(*afero.BasePathFs), path)
}
