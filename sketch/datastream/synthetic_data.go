package datastream

var p_A Packet = Packet{SrcIP: "1.1.1.1", SrcPort: "22", DstIP: "2.2.2.2", DstPort: "22", Protocol: "tcp"}
var p_B Packet = Packet{SrcIP: "2.2.2.2", SrcPort: "22", DstIP: "1.1.1.1", DstPort: "22", Protocol: "tcp"}
var p_C Packet = Packet{SrcIP: "3.3.3.3", SrcPort: "100", DstIP: "14.14.14.14", DstPort: "123", Protocol: "tcp"}
var p_D Packet = Packet{SrcIP: "4.4.4.4", SrcPort: "443", DstIP: "10.0.0.1", DstPort: "443", Protocol: "tcp/https"}
var p_E Packet = Packet{SrcIP: "2.2.2.2", SrcPort: "80", DstIP: "1.1.1.1", DstPort: "80", Protocol: "tcp"}
var p_F Packet = Packet{SrcIP: "10.0.1.1", SrcPort: "2543", DstIP: "10.1.0.3", DstPort: "2543", Protocol: "udp"}
var p_G Packet = Packet{SrcIP: "192.168.1.1", SrcPort: "10001", DstIP: "192.168.2.1", DstPort: "10003", Protocol: "tcp"}

func SyntheticDataStream(method string, datapoints int, channel chan Packet) {
	switch method {
	case "samePacketOneAnomaly":
		samePacketOneAnomaly(datapoints, channel)
	case "varyingPackets":
		varyingPackets(datapoints, channel)
	case "alternatingPackets":
		alternatingPackets(datapoints, channel)
	case "varyingPacketsWithAnomaly":
		varyingPacketsWithAnomaly(datapoints, channel)
	default:
		samePacket(datapoints, channel)
	}

	close(channel)
}

func samePacket(datapoints int, channel chan Packet) {
	for i := 0; i < datapoints; i++ {
		channel <- p_A
	}
}

func samePacketOneAnomaly(datapoints int, channel chan Packet) {
	samePacket(datapoints/2, channel)
	channel <- p_B
	samePacket(datapoints/2, channel)
}

func alternatingPackets(datapoints int, channel chan Packet) {
	for i := 0; i < datapoints; {
		channel <- p_A
		channel <- p_B
		i += 2
	}
}

func varyingPackets(datapoints int, channel chan Packet) {
	for i := 0; i < datapoints; {
		channel <- p_A
		channel <- p_B
		channel <- p_C
		channel <- p_D
		channel <- p_E
		channel <- p_F
		channel <- p_G
		i += 7
	}
}

func varyingPacketsWithAnomaly(datapoints int, channel chan Packet) {
	varyingPackets(datapoints/2, channel)
	for i := 0; i < 30; i++ {
		channel <- p_G
	}
	varyingPackets(datapoints/2-37, channel)
}
