package auth

import (
	"net/http"
	"toy/pkg/users"
)

type Auther interface {
	Auth(r *http.Request, s users.Store, root string) (*users.User, error)
	LoginPage() bool
}
