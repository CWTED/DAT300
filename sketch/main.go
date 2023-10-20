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
		forecasting string
	)

	flag.StringVar(&file, "file", "", "pcap file location")
	flag.IntVar(&h, "h", 2, "# of hash rows")
	flag.IntVar(&k, "k", 50, "# of elements in each hash array")
	flag.IntVar(&epoch, "epoch", 1000, "# of packets in one epoch")
	flag.Float64Var(&threshold, "threshold", 0.25, "Change threshold")
	flag.Float64Var(&alpha, "alpha", 0.5, "Alpha used in EWMA")
	flag.StringVar(&method, "method", "", "Method for creating synthetic data")
	flag.IntVar(&datapoints, "datapoints", 100_000, "# datapoints in synthetic data")
	flag.StringVar(&forecasting, "forecast", "ewma", "Forecasting algorithm to be used")

	flag.Parse()

	Kary(file, h, k, epoch, threshold, alpha, datapoints, method, forecasting)
}
