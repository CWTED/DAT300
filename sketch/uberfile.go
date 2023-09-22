package main

import (
	"fmt"
	"sketch/datastream"
)

func main() {
	datastream.StreamData()

	select {
	case receivedList := <-datastream.CommsCh:
		fmt.Println(receivedList)
	default:
		break
	}

}
