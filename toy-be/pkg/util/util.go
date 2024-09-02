package util

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"regexp"
	"strconv"
	"toy/pkg/shell"
)

// IsFile returns file existence.
//
// If file exists, return 0. Else return -1.
func IsFile(path string) bool {
	ret := false
	if _, err := os.Stat(path); err == nil {
		ret = true
	}
	return ret
}

// ReadJSONBody is not close body's buffer.
func ReadJSONBody(r *http.Request, f interface{}) error {
	recv, _ := ioutil.ReadAll(r.Body)
	r.Body = ioutil.NopCloser(bytes.NewBuffer(recv))
	err := json.Unmarshal(recv, f)
	return err
}

// GetAvailSize return path's avail size.
func GetAvailSize(path string) int {
	size := 0
	cmd := fmt.Sprintf("df '%s' | tail -1", path)
	column := shell.RunRead(cmd)
	r := regexp.MustCompile("[^\\s]+")
	s := r.FindAllString(column, -1)
	size, _ = strconv.Atoi(s[len(s)-3])
	return size
}
