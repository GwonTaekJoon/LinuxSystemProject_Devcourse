package logger

import (
	"fmt"
	"os"
	"time"
)

// Prelog struct.
type Prelog struct {
	DateView bool
	Prefix	 string
}

// SetPrefix set the log user's prefix.
func (l *Prelog) SetPrefix(s string) {
	l.Prefix = s
}

// RemovePrefix init the log user's prefix
func (l *Prelog) RemovePrefix() {
	l.Prefix = ""
}

// SetDate set the log date prefix.
func (l *Prelog) SetDate(is bool) {
	l.DateView = is
}

func (l *Prelog) printPrefix() {
	if l.DateView == true {
		t := time.Now()
		fmt.Printf("[%d/%d/%d %02d:%02d:%02d] ",
				   t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second())
	}

	if l.Prefix != "" {
		fmt.Printf("%s: ", l.Prefix)
	}
}

// Logf print argv string.
func (l *Prelog) Logf(format string, s ...interface{}) {
	l.printPrefix()
	fmt.Fprintf(os.Stdout, format, s...)
}

// Log print argv string.
func (l *Prelog) Log(s ...interface{}) {
	l.printPrefix()
	fmt.Fprint(os.Stdout, s...)
}

// Logln print argv string and auto line jump.
func (l *Prelog) Logln(s ...interface{}) {
	l.printPrefix()
	fmt.Fprintln(os.Stdout, s...)
}