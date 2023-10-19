package main

import (
	"fmt"
	"log"

	"sketch/change"
	"sketch/datastream"
	"sketch/forecasting"
	"sketch/sketch"
)

func Kary(file string, h int, k int, epoch int, threshold float64, alpha float64, datapoints int, method string) {
	anomalies := make(chan Data)
	go ExportData(anomalies)
	defer close(anomalies)

	dataChannel := make(chan datastream.Packet)
	if file != "" {
		go datastream.StreamData(file, dataChannel) // data pre-processing
	} else {
		go datastream.SyntheticDataStream(method, datapoints, dataChannel)
	}

	packetList := make([]datastream.Packet, epoch)

	// Create the main sketch
	s, err := sketch.New(h, k)
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}

	// Forcasting variables
	var (
		forecastAlgo *forecasting.AccVel
		fSketch      *sketch.Sketch
	)
	forecastAlgo = forecasting.New(epoch, h, k)

	// Variable representing how many times the algorithm has iterated
	index := 0

	for packet := range dataChannel {
		// Update the forecasting sketch if there is a new epoch

		if index%epoch == 0 && index > 0 {
			fSketch, err = forecastAlgo.Forecast(s)
			if err != nil {
				log.Fatalln("error while forcasting", err)
			}
			//fSketch.Print()
			// Update the sketch with the incoming packet

			// Change detection
			for i, p := range packetList {
				observedChange, thresholdChange, err := change.FEDetect(s, fSketch, threshold, p.ToBytes())
				// If there is a change, save it in data.csv
				if err != nil {
					fmt.Println()
					s.Print()
					fSketch.Print()
					anomalies <- Data{index: index - epoch + i, packet: p, observedChange: observedChange, thresholdChange: thresholdChange}
					fmt.Printf("Anomaly detected! Index: %d\t Observed change: %f, Threshold: %f\n", index - i, observedChange, thresholdChange)
				}
			}
		}

		s.Update(packet.ToBytes(), 1)
		forecastAlgo.UpdateVelocity(s)

		packetList[index % epoch] = packet

		index++
	}

}
