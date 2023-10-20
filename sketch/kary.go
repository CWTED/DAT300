package main

import (
	"fmt"
	"log"

	"sketch/change"
	"sketch/datastream"
	"sketch/forecasting"
	"sketch/sketch"
)

func Kary(file string, h int, k int, epoch int, threshold float64, alpha float64, datapoints int, method string, forecasting_algo string) {
	// Create channel for exporting anomalies
	anomalies := make(chan Data)
	go ExportData(anomalies)
	defer close(anomalies)

	// Create channel for incoming packets
	dataChannel := make(chan datastream.Packet)
	if file != "" {
		go datastream.StreamData(file, dataChannel) // data pre-processing
	} else {
		go datastream.SyntheticDataStream(method, datapoints, dataChannel)
	}

	// List of all incoming packets during the epoch
	packetList := make([]datastream.Packet, epoch)
	//Total amount of anomalies detected
	var anomCounter int

	// Create the main sketch
	s, err := sketch.New(h, k)
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}
	
	// Forcasting variables
	var (
		forecastAlgo forecasting.Forecasting
		fSketch      *sketch.Sketch
	)
	
	// Determine which forecasting algorithm to use
	switch forecasting_algo {
		case "ewma":
			forecastAlgo = forecasting.NewEWMA(alpha)
		case "velacc":
			forecastAlgo = forecasting.NewVelAcc(epoch, s.H(), s.K())
	}

	// Variable representing how many times the algorithm has iterated
	index := 0

	for packet := range dataChannel {
		// Check if the epoch has ended
		if index%epoch == 0 && index > 0 {
			fSketch, err = forecastAlgo.Forecast(s)
			if err != nil {
				log.Fatalln("error while creating new sketch", err)
			}

			// Go through all packets of the epoch to detect anomalies
			for i, p := range packetList {
				observedChange, thresholdChange, err := change.FEDetect(s, fSketch, threshold, p.ToBytes())

				// If there is an anomaly, save it in data.csv and print it to the terminal
				if err != nil {
					anomalies <- Data{index: index - epoch + i, packet: p, observedChange: observedChange, thresholdChange: thresholdChange}
					fmt.Printf("Anomaly detected! Index: %d\t Observed change: %f, Threshold: %f\n", index-epoch+i, observedChange, thresholdChange)
					anomCounter++
				}
			}

			if forecasting_algo == "ewma" {
				// Create new EWMA observed sketch for new epoch
				s, err = sketch.New(h, k)
				if err != nil {
					log.Fatalln("error while performing sketch", err)
				}
			}
		}

		// Update sketch and forecasting
		s.Update(packet.ToBytes(), 1)
		forecastAlgo.Update(s)

		packetList[index % epoch] = packet

		index++
	}

	fmt.Printf("Total number of anomalies detected: %d\n", anomCounter)
}
