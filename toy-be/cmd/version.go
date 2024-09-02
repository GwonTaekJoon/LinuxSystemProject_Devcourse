package cmd

import (
	"log"

	"github.com/spf13/cobra"

	"toy/version"
)

func init() {
	rootCmd.AddCommand(versionCmd)
}

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Toy robot the version number",
	Run: func(cmd *cobra.Command, args []string) {
		log.Println("Toy robot v" + version.Version + "/" + version.CommitSHA)
	},
}
