package main

import (
	"flag"
)

func main() {
	var (
		file string
		h int
		k int 
		epoch int
		threshold float64
		alpha float64
		method string
		datapoints int
	)

	flag.StringVar(&file, "file", "", "pcap file location")
	flag.IntVar(&h, "h", 5, "# of hash rows")
	flag.IntVar(&k, "k", 8192, "# of elements in each hash array")
	flag.IntVar(&epoch, "epoch", 1000, "# of packets in one epoch")
	flag.Float64Var(&threshold, "threshold", 0.25, "Change threshold")
	flag.Float64Var(&alpha, "alpha", 0.5, "Alpha used in EWMA")
	flag.StringVar(&method, "method", "", "Method for creating synthetic data")
	flag.IntVar(&datapoints, "datapoints", 100_000, "# datapoints in synthetic data")

	flag.Parse()

	Kary(file, h, k, epoch, threshold, alpha, datapoints, method)
}
