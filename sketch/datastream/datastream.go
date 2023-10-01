package datastream

import (
	"fmt" // Import the fmt package to print messages to the console.
	"log" // Import the log package to log errors to the console.
	"os"

	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
	"github.com/google/gopacket/pcap"
)

type Packet struct {
	srcIP    string
	dstIP    string
	srcPort  string
	dstPort  string
	protocol string
}

//var CommsCh chan tuple.T4[string, string, string, string]

func StreamData() []Packet {
	// Check if file argument is provided
	if len(os.Args) < 2 {
		fmt.Println("Please provide a pcap file to read")
		os.Exit(1)
	}

	// Open up the pcap file for reading
	handle, err := pcap.OpenOffline(os.Args[1])
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

		// Extract and print the Ethernet layer
		ethLayer := packet.Layer(layers.LayerTypeEthernet)
		if ethLayer != nil {
			//ethPacket, _ := ethLayer.(*layers.Ethernet)
			//fmt.Println("Ethernet source MAC address:", ethPacket.SrcMAC)
			//fmt.Println("Ethernet destination MAC address:", ethPacket.DstMAC)
		}

		// Extract and print the IP layer
		ipLayer := packet.Layer(layers.LayerTypeIPv4)
		if ipLayer != nil {
			ipPacket, _ := ipLayer.(*layers.IPv4)
			//fmt.Println("IP source address:", ipPacket.SrcIP)
			//	fmt.Println("IP destination address:", ipPacket.DstIP)
			packets.srcIP = ipPacket.SrcIP.String()
			packets.dstIP = ipPacket.DstIP.String()

			// add to tuple
		}
		tcpLayer := packet.Layer(layers.LayerTypeTCP)
		if tcpLayer != nil {
			tcpPacket, _ := tcpLayer.(*layers.TCP)
			//fmt.Println("SrcPrt:", tcpPacket.SrcPort)
			//fmt.Println("DstPrt:", tcpPacket.DstPort)
			// add to tuple
			packets.srcPort = tcpPacket.SrcPort.String()
			packets.dstPort = tcpPacket.DstPort.String()
		}
		//CommsCh <- netTup
		packets.protocol = ""

		packetList = append(packetList, packets)

	}
	return packetList

}
