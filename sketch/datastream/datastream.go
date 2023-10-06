package datastream

import (
	"log" // Import the log package to log errors to the console.
	"strings"

	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
	"github.com/google/gopacket/pcap"
)

type Packet struct {
	SrcMAC 	 string
	DstMAC	 string 
	SrcIP    string
	DstIP    string
	SrcPort  string
	DstPort  string
	Protocol string
}

func StreamData(file string, channel chan Packet) {
	// Open up the pcap file for reading
	handle, err := pcap.OpenOffline(file)
	if err != nil {
		log.Fatal(err)
	}
	defer handle.Close()

	// Packet that shall be sent in channel
	var p Packet

	// Loop through packets in file
	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
	for packet := range packetSource.Packets() {
		// Extract and print the IP layer
		ethLayer := packet.Layer(layers.LayerTypeEthernet)
		if ethLayer != nil {
			ethPacket := ethLayer.(*layers.Ethernet)
			p.SrcMAC = ethPacket.SrcMAC.String()
			p.DstMAC = ethPacket.DstMAC.String()
		} else {
			p.SrcMAC = ""
			p.DstMAC = ""
		}

		ipLayer := packet.Layer(layers.LayerTypeIPv4)
		if ipLayer != nil {
			ipPacket, _ := ipLayer.(*layers.IPv4)
			p.SrcIP = ipPacket.SrcIP.String()
			p.DstIP = ipPacket.DstIP.String()
			p.Protocol = ipPacket.Protocol.String()
		}

		/*ipLayer6 := packet.Layer(layers.LayerTypeIPv6)
		if ipLayer6 != nil {
			ipPacket6, _ := ipLayer.(*layers.IPv6)
			//fmt.Println("IP source address:", ipPacket.SrcIP)
			//	fmt.Println("IP destination address:", ipPacket.DstIP)
			packets.srcIP = ipPacket6.SrcIP.String()
			packets.dstIP = ipPacket6.DstIP.String()
			packets.protocol = ipPacket6.NextHeader.String()
		}*/

		tcpLayer := packet.Layer(layers.LayerTypeTCP)
		if tcpLayer != nil {
			tcpPacket, _ := tcpLayer.(*layers.TCP)
			p.SrcPort = tcpPacket.SrcPort.String()
			p.DstPort = tcpPacket.DstPort.String()
		}
		channel <- p
	}
	close(channel)
}

func (p *Packet) ToBytes() []byte {
	var str strings.Builder

	str.WriteString(p.SrcMAC)
	str.WriteString(p.DstMAC)
	str.WriteString(p.SrcIP)
	str.WriteString(p.DstIP)
	str.WriteString(p.SrcPort)
	str.WriteString(p.DstPort)
	str.WriteString(p.Protocol)

	return []byte(str.String())
}
