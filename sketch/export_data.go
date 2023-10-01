package main

import (
	"encoding/csv"
	"log"
	"os"
	"strconv"

	"sketch/datastream"
)

func ExportData(channel chan Data) {
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
		
		row := []string{data.packet.SrcIP, data.packet.DstIP, data.packet.SrcPort, data.packet.DstPort, 
						data.packet.Protocol, strconv.FormatUint(data.observedChange, 10), strconv.FormatUint(data.thresholdChange, 10)}
		if err := w.Write(row); err != nil {
			log.Fatalln("error writing tuple to file", err)
		}
		w.Flush()
	}
}

type Data struct {
	packet datastream.Packet
	observedChange uint64
	thresholdChange uint64
}