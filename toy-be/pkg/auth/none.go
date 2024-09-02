package auth

import (
	"net/http"

	"toy/pkg/settings"
	"toy/pkg/users"
)

const MethodNoAuth settings.AuthMethod = "noauth"

type NoAuth struct{}

func (a NoAuth) Auth(r *http.Request, sto users.Store, root string) (*users.User, error) {
	return sto.Get(root, uint(1))
}

func (a NoAuth) LoginPage() bool {
	return false
}
