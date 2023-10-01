package main

import (
	"fmt"
	"log"
	"sketch/datastream"
	"sketch/sketch"
)

func main() {
	receivedList := datastream.StreamData() // data pre-processing
	sketch, err := sketch.New(4, 10)
	if err != nil {
		log.Fatalln("error while creating sketch", err)
	}

	for _, packet := range receivedList {
		fmt.Println(packet)

		sketch.Update(packet.ToBytes(), 1)
	}

	sketch.Print()

	// print the estimate of the kary sketch
	//fmt.Printf("Result: %v", kary.Estimate())
}
