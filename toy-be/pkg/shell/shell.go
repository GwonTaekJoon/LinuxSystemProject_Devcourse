package shell

import (
	"bytes"
	"log"
	"os/exec"
	"strconv"
	"syscall"
)

const (
	exit = ";echo $?"
	sync = "sync;sync"
)

const defaultFailedCode = 1

func RunCommand(command string) (stdout string, stderr string, exitCode int) {
	log.Println("run command:", command)
	var outbuf, errbuf bytes.Buffer
	cmd := exec.Command("sh", "-c", command)
	cmd.Stdout = &outbuf
	cmd.Stderr = &errbuf

	err := cmd.Run()
	stdout = outbuf.String()
	stderr = errbuf.String()

	if err != nil {
		// try to get the exit code
		if exitError, ok := err.(*exec.ExitError); ok {
			ws := exitError.Sys().(syscall.WaitStatus)
			exitCode = ws.ExitStatus()
		} else {
			// This will happen (in OSX) if `name` is not available in $PATH,
			// in this situation, exit code could not be get, and stderr will be
			// empty string very likely, so we use the default fail code, and format err
			// to string and set to stderr
			log.Printf("Could not get exit code for failed program: %v", command)
			exitCode = defaultFailedCode
			if stderr == "" {
				stderr = err.Error()
			}
		}
	} else {
		// success, exitCode should be 0 if go is ok
		ws := cmd.ProcessState.Sys().(syscall.WaitStatus)
		exitCode = ws.ExitStatus()
	}
	log.Printf("command result, stdout: %v, stderr: %v, exitCode: %v", stdout, stderr, exitCode)
	return
}

// Run execute shell command and return "echo $?" int value.
func Run(command string) int {
	var ret int
	if len(command) > 0 {
		cmd := exec.Command("sh", "-c", command+exit)
		stdout, _ := cmd.CombinedOutput()
		ret, _ = strconv.Atoi(string(stdout))
	} else {
		ret = -1
	}
	return ret
}

// RunRead execute shell command and return output string.
func RunRead(command string) string {
	cmd := exec.Command("sh", "-c", command)
	stdout, _ := cmd.CombinedOutput()
	ret := string(stdout)
	return ret
}

// Sync execute sync command in shell.
func Sync() {
	cmd := exec.Command("sh", "-c", sync)
	cmd.Run()
}
