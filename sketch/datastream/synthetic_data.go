package datastream

import (

)

var p_A Packet = Packet{SrcIP: "1.1.1.1", SrcPort: "22", DstIP: "2.2.2.2", DstPort: "22", Protocol: "tcp"}
var p_B Packet = Packet{SrcIP: "2.2.2.2", SrcPort: "22", DstIP: "1.1.1.1", DstPort: "22", Protocol: "tcp"}

func SyntheticDataStream(channel chan Packet) {
	for i := 0; i < 500_000; i++ {
		channel <- p_A
	}	
	//channel <- p_B
	//channel <- p_B
	close(channel)
}