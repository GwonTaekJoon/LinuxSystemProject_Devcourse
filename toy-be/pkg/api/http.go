package http

import (
	"embed"
	"io/fs"
	"net/http"

	"github.com/gorilla/mux"

	"toy/pkg/settings"
	"toy/pkg/storage"
)

type modifyRequest struct {
	What  string   `json:"what"`  // Answer to: what data type?
	Which []string `json:"which"` // Answer to: which fields?
}

func EmbedFolder(fsEmbed embed.FS, targetPath string) http.FileSystem {
	fsys, err := fs.Sub(fsEmbed, targetPath)
	if err != nil {
		panic(err)
	}
	return http.FS(fsys)
}

func CORS(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {

		// Set headers
		w.Header().Set("Access-Control-Allow-Headers:", "*")
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "*")

		if r.Method == "OPTIONS" {
			w.WriteHeader(http.StatusOK)
			return
		}

		// Next
		next.ServeHTTP(w, r)
	})
}

func NewHandler(
	store *storage.Storage,
	server *settings.Server,
	assetsFs fs.FS,
) (http.Handler, error) {
	server.Clean()

	r := mux.NewRouter()
	r.Use(CORS)
	index, static := getStaticHandlers(store, server, assetsFs)

	r = r.SkipClean(true)

	monkey := func(fn handleFunc, prefix string) http.Handler {
		return handle(fn, prefix, store, server)
	}

	r.PathPrefix("/static").Handler(static)
	r.NotFoundHandler = index

	api := r.PathPrefix("/api/v1").Subrouter()

	// REST APIs
	robots := api.PathPrefix("/robots").Subrouter()
	robots.Handle("", monkey(robotsGetHandler, "")).Methods("GET")
	robots.Handle("", monkey(robotsPostHandler, "")).Methods("POST")
	robots.Handle("/{id:[0-9]+}", monkey(robotPutByIDHandler, "")).Methods("PUT")
	robots.Handle("/{id:[0-9]+}", monkey(robotGetByIDHandler, "")).Methods("GET")
	robots.Handle("/{id:[0-9]+}", monkey(robotDeleteHandler, "")).Methods("DELETE")
	robots.Handle("/{id:[0-9]+}/lspeed", monkey(robotSetLeftSpeedHandler, "")).Methods("PUT")
	robots.Handle("/{id:[0-9]+}/rspeed", monkey(robotSetRightSpeedHandler, "")).Methods("PUT")
	robots.Handle("/{id:[0-9]+}/halt", monkey(robotHaltMotorHandler, "")).Methods("PUT")

	return stripPrefix(server.BaseURL, r), nil
}
