package main

import (
	"encoding/csv"
	"os"
	"log"
	"github.com/barweiss/go-tuple"
)

func ExportData(channel chan tuple.T4[string,string,string,string]) {
	f, err := os.OpenFile("data.csv", os.O_RDWR|os.O_CREATE, 0755)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	
	w := csv.NewWriter(f)


	for {
		data, ok := <- channel
		if !ok {
			break
		}
		
		a, b, c, d := data.Values()
		row := []string{a, b, c, d}
		if err := w.Write(row); err != nil {
			log.Fatalln("error writing tuple to file", err)
		}
		w.Flush()
	}
}