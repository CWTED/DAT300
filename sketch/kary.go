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

	// Create the main sketch
	s, err := sketch.New(h, k)
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}

	// Forcasting variables
	var (
		forecastAlgo *forecasting.EWMA
		fSketch *sketch.Sketch

	)
	forecastAlgo = &forecasting.EWMA{Alpha: alpha}

	// Variable representing how many times the algorithm has iterated
	index := 0

	for packet := range dataChannel {
		// Update the forecasting sketch if there is a new epoch
		if index % epoch == 0 && index > 0 {
			fSketch, err = forecastAlgo.Forecast(s)
			if err != nil {
				log.Fatalln("error while creating new sketch", err)
			}
			//fSketch.Print()

			// Here we can perform the change detection instead
			// We need to record all packets during this epoch to be able to do it, 
			// such as adding them to a list and then iterating through it

			// Create new EWMA observed sketch for new epoch
			s, err = sketch.New(h,k)
			if err != nil {
				log.Fatalln("error while performing sketch", err)
			}
		}

		// Update the sketch with the incoming packet
		s.Update(packet.ToBytes(), 1)
		//forecastAlgo.UpdateVelocity(s)

		// Change detection
		if index >= epoch {
			observedChange, thresholdChange, err := change.FEDetect(s, fSketch, threshold, packet.ToBytes())
			// If there is a change, save it in data.csv
			if err != nil {
				anomalies <- Data{index: index, packet: packet, observedChange: observedChange, thresholdChange: thresholdChange}
				fmt.Printf("Anomaly detected! Index: %d\t Observed change: %f, Threshold: %f\n", index, observedChange, thresholdChange)
			}
		}
		index++
	}

}
