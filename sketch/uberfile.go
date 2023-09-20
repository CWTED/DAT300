package main

import (
	"sketch/datastream"
)

func main() {
	go datastream.StreamData()
}
