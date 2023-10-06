package main

import (
	"encoding/csv"
	"log"
	"os"
	"fmt"

	"sketch/datastream"
)

func ExportData(channel chan Data) {
	f, err := os.OpenFile("data.csv", os.O_RDWR|os.O_CREATE, 0755)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	
	w := csv.NewWriter(f)


	for data := range channel {
		row := []string{data.packet.SrcMAC, data.packet.DstMAC, data.packet.SrcIP, data.packet.DstIP, data.packet.SrcPort, data.packet.DstPort, 
						data.packet.Protocol, fmt.Sprintf("%f", data.observedChange), fmt.Sprintf("%f", data.thresholdChange)}
		if err := w.Write(row); err != nil {
			log.Fatalln("error writing tuple to file", err)
		}
		w.Flush()
	}
}

type Data struct {
	packet datastream.Packet
	observedChange float64
	thresholdChange float64
}