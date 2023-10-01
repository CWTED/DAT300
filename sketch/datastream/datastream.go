package datastream

import (
	"log" // Import the log package to log errors to the console.
	"strings"

	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
	"github.com/google/gopacket/pcap"
)

type Packet struct {
	SrcIP    string
	DstIP    string
	SrcPort  string
	DstPort  string
	Protocol string
}

//var CommsCh chan tuple.T4[string, string, string, string]

func StreamData(file string) []Packet {
	// Open up the pcap file for reading
	handle, err := pcap.OpenOffline(file)
	if err != nil {
		log.Fatal(err)
	}
	defer handle.Close()

	// Define channel for comms
	//CommsCh = make(chan tuple.T4[string, string, string, string], 1)

	// Define tuple list
	//var tupleList []tuple.T4[string, string, string, string]
	var packets Packet
	var packetList []Packet

	// Loop through packets in file
	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
	for packet := range packetSource.Packets() {

		// per packet tuple creation
		//netTup := tuple.New4("", "", "", "")

		// Print the packet details
		//fmt.Println(packet.String())

		// Extract and print the IP layer
		ipLayer := packet.Layer(layers.LayerTypeIPv4)
		if ipLayer != nil {
			ipPacket, _ := ipLayer.(*layers.IPv4)
			//fmt.Println("IP source address:", ipPacket.SrcIP)
			//	fmt.Println("IP destination address:", ipPacket.DstIP)
			packets.SrcIP = ipPacket.SrcIP.String()
			packets.DstIP = ipPacket.DstIP.String()
			packets.Protocol = ipPacket.Protocol.String()

			// add to tuple
		}

		/*ipLayer6 := packet.Layer(layers.LayerTypeIPv6)
		if ipLayer6 != nil {
			ipPacket6, _ := ipLayer.(*layers.IPv6)
			//fmt.Println("IP source address:", ipPacket.SrcIP)
			//	fmt.Println("IP destination address:", ipPacket.DstIP)
			packets.srcIP = ipPacket6.SrcIP.String()
			packets.dstIP = ipPacket6.DstIP.String()
			packets.protocol = ipPacket6.NextHeader.String()

			// add to tuple
		}*/

		tcpLayer := packet.Layer(layers.LayerTypeTCP)
		if tcpLayer != nil {
			tcpPacket, _ := tcpLayer.(*layers.TCP)
			//fmt.Println("SrcPrt:", tcpPacket.SrcPort)
			//fmt.Println("DstPrt:", tcpPacket.DstPort)
			// add to tuple
			packets.SrcPort = tcpPacket.SrcPort.String()
			packets.DstPort = tcpPacket.DstPort.String()
		}
		//CommsCh <- netTup

		packetList = append(packetList, packets)

	}
	return packetList

}

func (p *Packet) ToBytes() []byte {
	var str strings.Builder

	str.WriteString(p.SrcIP)
	str.WriteString(p.DstIP)
	str.WriteString(p.SrcPort)
	str.WriteString(p.DstPort)
	str.WriteString(p.Protocol)

	return []byte(str.String())
}
