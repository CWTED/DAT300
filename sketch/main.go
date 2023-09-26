package main

import (
	"fmt"
	"sketch/datastream"
	"sketch/kary"
)

func main() {
	receivedList := datastream.StreamData() // data pre-processing

	for index := range receivedList {
		kary.Update(receivedList[index]) // on demand, give next data entry to sketch

		// time the sketch phases
	}

	// print the estimate of the kary sketch
	fmt.Printf("Result: %v", kary.Estimate())
}
