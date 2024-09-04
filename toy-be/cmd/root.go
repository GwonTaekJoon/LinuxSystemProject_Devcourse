package cmd

import (
	"crypto/tls"
	"errors"
	"io/fs"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"
	"time"

	homedir "github.com/mitchellh/go-homedir"
	"github.com/spf13/afero"
	"github.com/spf13/cobra"
	"github.com/spf13/pflag"
	v "github.com/spf13/viper"
	lumberjack "gopkg.in/natefinch/lumberjack.v2"

	"toy/frontend"
	api "toy/pkg/api"
	"toy/pkg/auth"
	robots "toy/pkg/robots"
	"toy/pkg/settings"
	"toy/pkg/storage"
	"toy/pkg/users"
)

var (
	cfgFile        string
	dialTimeout    = 5 * time.Second
	requestTimeout = 3 * time.Second
)

func init() {

	cobra.OnInitialize(initConfig)
	cobra.MousetrapHelpText = ""

	rootCmd.SetVersionTemplate("API Server version {{printf \"%s\" .Version}}\n")

	flags := rootCmd.Flags()
	persistent := rootCmd.PersistentFlags()

	persistent.StringVarP(&cfgFile, "config", "c", "", "config file path")
	persistent.StringP("database", "d", "./toy.db", "database path")
	flags.Bool("noauth", false, "use the noauth auther when using quick setup")
	flags.String("username", "admin", "username for the first user when using quick config")
	flags.String("password", "", "hashed password for the first user when using quick config (default \"admin\")")

	addServerFlags(flags)
}

func addServerFlags(flags *pflag.FlagSet) {
	flags.StringP("address", "a", "127.0.0.1", "address to listen on")
	flags.StringP("log", "l", "stdout", "log output")
	flags.StringP("port", "p", "8080", "port to listen on")
	flags.StringP("cert", "t", "", "tls certificate")
	flags.StringP("key", "k", "", "tls key")
	flags.StringP("root", "r", ".", "root to prepend to relative paths")
	flags.String("socket", "", "socket to listen to (cannot be used with address, port, cert nor key flags)")
	flags.Uint32("socket-perm", 0666, "unix socket file permissions")
	flags.StringP("baseurl", "b", "", "base url")
	flags.StringP("id", "i", "", "device id")
	flags.String("cache-dir", "", "file cache directory (disabled if empty)")
	flags.Int("img-processors", 4, "image processors count")
	flags.Bool("disable-thumbnails", false, "disable image thumbnails")
	flags.Bool("disable-preview-resize", false, "disable resize of image previews")
	flags.Bool("disable-exec", false, "disables Command Runner feature")
	flags.Bool("disable-type-detection-by-header", false, "disables type detection by reading file headers")
}

// main
var rootCmd = &cobra.Command{
	Use:   "toy",
	Short: "A web-based interface",
	Long:  `TOY WEB CLI lets you create the database to use with`,
	Run: python(func(cmd *cobra.Command, args []string, d pythonData) {

		log.Println(cfgFile)

		if !d.hadDB {
			quickSetup(cmd.Flags(), d)
		}

		// build img service
		workersCount, err := cmd.Flags().GetInt("img-processors")
		checkErr(err)
		if workersCount < 1 {
			log.Fatal("Image resize workers count could not be < 1")
		}

		server := getRunParams(cmd.Flags(), d.store)
		log.SetPrefix("[TOY-BE]: ")

		root, err := filepath.Abs(server.Root)
		checkErr(err)
		server.Root = root

		adr := server.Address + ":" + server.Port

		var listener net.Listener

		switch {
		case server.Socket != "":
			listener, err = net.Listen("unix", server.Socket)
			checkErr(err)
			socketPerm, err := cmd.Flags().GetUint32("socket-perm") //nolint:govet
			checkErr(err)
			err = os.Chmod(server.Socket, os.FileMode(socketPerm))
			checkErr(err)
		case server.TLSKey != "" && server.TLSCert != "":
			cer, err := tls.LoadX509KeyPair(server.TLSCert, server.TLSKey) //nolint:govet
			checkErr(err)
			listener, err = tls.Listen("tcp", adr, &tls.Config{
				MinVersion:   tls.VersionTLS12,
				Certificates: []tls.Certificate{cer}},
			)
			checkErr(err)
		default:
			listener, err = net.Listen("tcp", adr)
			checkErr(err)
		}

		sigc := make(chan os.Signal, 1)
		signal.Notify(sigc, os.Interrupt, syscall.SIGTERM)
		go cleanupHandler(listener, sigc)

		assetsFs, err := fs.Sub(frontend.Assets(), "dist")
		if err != nil {
			panic(err)
		}

		handler, err := api.NewHandler(d.store, server, assetsFs)
		checkErr(err)

		defer listener.Close()

		// create web socket hub
		log.Println("Create web socket hub")
		hub := robots.NewWebSocketHub()

		InitRobots(hub, cmd.Flags(), d, server.ID)

		go robots.InitWebSocketService(hub)
		go robots.RobotThread(d.store.Robots, server, hub)

		log.Println("Listening on", listener.Addr().String())
		if err := http.Serve(listener, handler); err != nil {
			log.Fatal(err)
		}

	}, pythonConfig{allowNoDB: true}),
}

func cleanupHandler(listener net.Listener, c chan os.Signal) { //nolint:interfacer
	sig := <-c
	log.Printf("Caught signal %s: shutting down.", sig)
	listener.Close()
	os.Exit(0)
}

//nolint:gocyclo
func getRunParams(flags *pflag.FlagSet, st *storage.Storage) *settings.Server {
	server, err := st.Settings.GetServer()
	checkErr(err)

	if val, set := getParamB(flags, "root"); set {
		server.Root = val
	}

	if val, set := getParamB(flags, "baseurl"); set {
		server.BaseURL = val
	}

	if val, set := getParamB(flags, "id"); set {
		server.ID = val
	}

	if val, set := getParamB(flags, "log"); set {
		server.Log = val
	}

	isSocketSet := false
	isAddrSet := false

	if val, set := getParamB(flags, "address"); set {
		server.Address = val
		isAddrSet = isAddrSet || set
	}

	if val, set := getParamB(flags, "port"); set {
		server.Port = val
		isAddrSet = isAddrSet || set
	}

	if val, set := getParamB(flags, "key"); set {
		server.TLSKey = val
		isAddrSet = isAddrSet || set
	}

	if val, set := getParamB(flags, "cert"); set {
		server.TLSCert = val
		isAddrSet = isAddrSet || set
	}

	if val, set := getParamB(flags, "socket"); set {
		server.Socket = val
		isSocketSet = isSocketSet || set
	}

	if isAddrSet && isSocketSet {
		checkErr(errors.New("--socket flag cannot be used with --address, --port, --key nor --cert"))
	}

	// Do not use saved Socket if address was manually set.
	if isAddrSet && server.Socket != "" {
		server.Socket = ""
	}

	_, disableThumbnails := getParamB(flags, "disable-thumbnails")
	server.EnableThumbnails = !disableThumbnails

	_, disablePreviewResize := getParamB(flags, "disable-preview-resize")
	server.ResizePreview = !disablePreviewResize

	_, disableTypeDetectionByHeader := getParamB(flags, "disable-type-detection-by-header")
	server.TypeDetectionByHeader = !disableTypeDetectionByHeader

	_, disableExec := getParamB(flags, "disable-exec")
	server.EnableExec = !disableExec

	return server
}

func getParamB(flags *pflag.FlagSet, key string) (string, bool) {
	value, _ := flags.GetString(key)

	// If set on Flags, use it.
	if flags.Changed(key) {
		return value, true
	}

	// If set through viper (env, config), return it.
	if v.IsSet(key) {
		return v.GetString(key), true
	}

	// Otherwise use default value on flags.
	return value, false
}

func getParam(flags *pflag.FlagSet, key string) string {
	val, _ := getParamB(flags, key)
	return val
}

func setupLog(logMethod string) {
	switch logMethod {
	case "stdout":
		log.SetOutput(os.Stdout)
	case "stderr":
		log.SetOutput(os.Stderr)
	case "":
		log.SetOutput(ioutil.Discard)
	default:
		log.SetOutput(&lumberjack.Logger{
			Filename:   logMethod,
			MaxSize:    100,
			MaxAge:     14,
			MaxBackups: 10,
		})
	}
}

func quickSetup(flags *pflag.FlagSet, d pythonData) {
	set := &settings.Settings{
		Key:           generateKey(),
		Signup:        false,
		CreateUserDir: false,
		Defaults: settings.UserDefaults{
			Scope:       ".",
			Locale:      "en",
			SingleClick: false,
			Perm: users.Permissions{
				Admin:    false,
				Execute:  true,
				Create:   true,
				Rename:   true,
				Modify:   true,
				Delete:   true,
				Share:    true,
				Download: true,
			},
		},
	}

	var err error
	if _, noauth := getParamB(flags, "noauth"); noauth {
		set.AuthMethod = auth.MethodNoAuth
		err = d.store.Auth.Save(&auth.NoAuth{})
	} else {
		set.AuthMethod = auth.MethodJSONAuth
		err = d.store.Auth.Save(&auth.JSONAuth{})
	}

	checkErr(err)
	err = d.store.Settings.Save(set)
	checkErr(err)

	ser := &settings.Server{
		BaseURL: getParam(flags, "baseurl"),
		ID:      getParam(flags, "id"),
		Port:    getParam(flags, "port"),
		Log:     getParam(flags, "log"),
		TLSKey:  getParam(flags, "key"),
		TLSCert: getParam(flags, "cert"),
		Address: getParam(flags, "address"),
		Root:    getParam(flags, "root"),
	}

	err = d.store.Settings.SaveServer(ser)
	checkErr(err)

	username := getParam(flags, "username")
	password := getParam(flags, "password")

	if password == "" {
		password, err = users.HashPwd("admin")
		checkErr(err)
	}

	if username == "" || password == "" {
		log.Fatal("username and password cannot be empty during quick setup")
	}

	user := &users.User{
		Username:     username,
		Password:     password,
		LockPassword: false,
		Fs:           afero.NewBasePathFs(afero.NewOsFs(), "/home/kesl/temp"),
	}

	set.Defaults.Apply(user)
	user.Perm.Admin = true

	err = d.store.Users.Save(user)
	checkErr(err)
}

func initConfig() {
	if cfgFile == "" {
		home, err := homedir.Dir()
		checkErr(err)
		v.AddConfigPath(".")
		v.AddConfigPath(home)
		v.SetConfigName(".toy")
	} else {
		v.SetConfigFile(cfgFile)
	}

	v.SetEnvPrefix("FB")
	v.AutomaticEnv()
	v.SetEnvKeyReplacer(strings.NewReplacer(".", "_"))

	if err := v.ReadInConfig(); err != nil {
		if _, ok := err.(v.ConfigParseError); ok {
			panic(err)
		}
		cfgFile = "No config file used"
	} else {
		cfgFile = "Using config file: " + v.ConfigFileUsed()
	}
}
