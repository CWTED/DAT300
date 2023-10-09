package main

import (
	_"fmt"
	"log"

	"sketch/datastream"
	"sketch/sketch"
	"sketch/forecasting"
	"sketch/change"
)

func Kary(file string, h int, k int, epoch int, threshold float64, alpha float64) {
	anomalies := make(chan Data)
	go ExportData(anomalies)
	defer close(anomalies)

	dataChannel := make(chan datastream.Packet)
	if file != "" {
		go datastream.StreamData(file, dataChannel) // data pre-processing
	} else {
		go datastream.SyntheticDataStream(dataChannel)
	}

	// Create the main sketch
	s, err := sketch.New(h, k)
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}

	// Forcasting variables
	var (
		forecastAlgo *forecasting.AccVel
		fSketch *sketch.Sketch
	)
	forecastAlgo = &forecasting.AccVel{Window: 5}


	// Variable representing how many times the algorithm has iterated
	index := 0

	for packet := range dataChannel {
		// Update the forecasting sketch if there is a new epoch
		if index % epoch == 0 && index > 0 {
			fSketch, err = forecastAlgo.Forecast(s)
			if err != nil {
				log.Fatalln("error while forcasting", err)
			}
			fSketch.Print()
		}

		// Update the sketch with the incoming packet
		s.Update(packet.ToBytes(), 1)
		forecastAlgo.UpdateVelocity(s)

		// Change detection
		if index >= epoch {
			observedChange, thresholdChange, err := change.FEDetect(s, fSketch, threshold, packet.ToBytes())
			// If there is a change, save it in data.csv
			if err != nil {
				anomalies <- Data{index: index, packet: packet, observedChange: observedChange, thresholdChange: thresholdChange}
				//fmt.Printf("Anomaly detected! Index: %d\t Observed change: %f, Threshold: %f\n", index, observedChange, thresholdChange)
			}
		}
		index++
	}

}