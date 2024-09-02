package auth

import (
	"net/http"
	"os"

	"toy/errors"
	"toy/pkg/settings"
	"toy/pkg/users"
)

const MethodProxyAuth settings.AuthMethod = "proxy"

type ProxyAuth struct {
	Header string `json:"header"`
}

func (a ProxyAuth) Auth(r *http.Request, sto users.Store, root string) (*users.User, error) {
	username := r.Header.Get(a.Header)
	user, err := sto.Get(root, username)
	if err == errors.ErrNotExist {
		return nil, os.ErrPermission
	}

	return user, err
}

func (a ProxyAuth) LoginPage() bool {
	return false
}
