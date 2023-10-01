package main

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"sketch/datastream"
	"sketch/sketch"
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

	sketch, err := sketch.New(uint(h), uint(k))
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
