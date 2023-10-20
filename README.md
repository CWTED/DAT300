# Sketch-based anomaly detection
 Project for Data-Driven Support For Cyber-Physical Systems at Chalmers

## Build or run
```
go build -o <output name> .
```
```
go run . <optional flags>
```
For example, run the program with Velocity/Acceleration model and window size 10 on synthetic data type *varyingPacketsWithAnomaly*:
```
go run . --forecast velacc --epoch 10 --method varyingPacketsWithAnomaly
```

## Flags 

 - **h**: int, # of hash rows (default 2)
 - **k**: int, # of elements in each hash array (default 50)
 - **threshold**: float, Change threshold (default 0.25)
 - **file**: string, pcap file location
 - **method**: string, Method for creating synthetic data (default samePacket)
 - **datapoints**: int, # of datapoints in synthetic data (default 100000)
 - **alpha**: float, Alpha used in EWMA (default 0.5)
 - **forecast**: string, Forecasting algorithm to be used (default "ewma")
 - **epoch**: int, # of packets in one epoch (default 1000)
