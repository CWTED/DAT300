package datastream

import (

)

var p_A Packet = Packet{SrcIP: "1.1.1.1", SrcPort: "22", DstIP: "2.2.2.2", DstPort: "22", Protocol: "tcp"}

func SyntheticDataStream(channel chan Packet) {
	for i := 0; i < 100_000; i++ {
		channel <- p_A
	}	
	close(channel)
}