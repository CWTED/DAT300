package main

import (
	"fmt"
	"log"
	"os"
	"strconv"

	"sketch/datastream"
	"sketch/sketch"
	"sketch/forecasting"
	"sketch/change"
)

func main() {
	// Check if file argument is provided
	if len(os.Args) < 4 {
		log.Fatalln("program usage: kary <pcap file> <# hash rows> <# elements in a hash row>")
	}
	h, err := strconv.Atoi(os.Args[2])
	if err != nil {
		log.Fatalln("program usage: kary <pcap file> <# hash rows> <# elements in a hash row>", err)
	}
	k, err := strconv.Atoi(os.Args[3]) 
	if err != nil {
		log.Fatalln("program usage: kary <pcap file> <# hash rows> <# elements in a hash row>", err)
	}

	anomalies := make(chan Data)
	go ExportData(anomalies)
	defer close(anomalies)

	dataChannel := make(chan datastream.Packet)
	go datastream.StreamData(os.Args[1], dataChannel) // data pre-processing

	// Create the main sketch
	s, err := sketch.New(h, k)
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}

	// Forcasting variables
	var (
		forecastAlgo forecasting.EWMA
		fSketch *sketch.Sketch
	)
	forecastAlgo = forecasting.EWMA{Alpha: 0.1}

	// Variable representing how many times the algorithm has iterated
	index := 0
	// Variable representing how many packets is one epoch
	epoch := 1000

	for packet := range dataChannel {
		// Update the forecasting sketch if there is a new epoch
		if index % epoch == 0 && index > 0 {
			fSketch, err = forecastAlgo.Forecast(s)
			if err != nil {
				log.Fatalln("error while forcasting", err)
			}
			//fSketch.Print()
		}

		// Update the sketch with the incoming packet
		s.Update(packet.ToBytes(), 1)
		//fmt.Println(s.EstimateF2())
		// Change detection
		if index > epoch {
			observedChange, thresholdChange, err := change.FEDetect(s, fSketch, 0.3, packet.ToBytes())
			// If there is a change, save it in data.csv
			if err != nil {
				anomalies <- Data{packet: packet, observedChange: observedChange, thresholdChange: thresholdChange}
				fmt.Printf("Anomaly detected! Index: %d\t Observed change: %f, Threshold: %f\n", index, observedChange, thresholdChange)
			}


		}
		index++
	}
}
