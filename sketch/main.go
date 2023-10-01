package main

import (
	"fmt"
	"log"
	"os"
	"strconv"

	"sketch/datastream"
	"sketch/sketch"
	"sketch/forecasting"
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


	receivedList := datastream.StreamData(os.Args[1]) // data pre-processing

	// Create the main sketch
	s, err := sketch.New(uint(h), uint(k))
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}

	// Forcasting variables
	var (
		forecastAlgo forecasting.Forecasting
		fSketch sketch.Sketch
	)
	forecastAlgo = forecasting.EWMA{Alpha: 0.5}

	for index, packet := range receivedList {
		// Update the forecasting sketch if there is a new epoch
		if index % 10 == 0 {
			fSketch, err = forecastAlgo.Forecast(s)
			if err != nil {
				log.Fatalln("error while forcasting", err)
			}
			fmt.Println("Forecasted: ")
			fSketch.Print()
			fmt.Println()
		}

		// Update the sketch with the incoming packet
		s.Update(packet.ToBytes(), 1)
	}

	s.Print()
}
