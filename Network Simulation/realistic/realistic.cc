/* Network topology:
 
     Default Network Topology

            Wifi 10.1.3.0
    AP                                                     
    *    *    *    *   *    *   *   *   *    *    *    *    *
    |    |    |    |   |    |   |   |   |    |    |    |    |
    n1   n2   n3   n4   n5   n6  n7  n8  n9  n10  n11  n12  n13

*/

#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"





NS_LOG_COMPONENT_DEFINE ("wifi-tcp");

using namespace ns3;

Ptr<PacketSink> sink;                     
uint64_t lastTotalRx = 0;                 
double simulationTime = 300;
double agg=0;

void CalculateThroughput ()
{
  Time now = Simulator::Now ();                                        
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (300), &CalculateThroughput);
}

int main (int argc, char *argv[])
{
        uint32_t num_clients=51;
        uint32_t payloadSize = 1472;
   	uint32_t maxBytes = 0;                   
        std::string dataRate = "54Mbps"; 
	std::string datARate = "1Mbps";             
        std::string tcpVariant = "TcpNewReno";         
    	std::string phyMode ("ErpOfdmRate54Mbps");
        std::string phyRate = "HtMcs7";                
        bool pcapTracing = true;    
	bool flow_monitor = true; 

	uint32_t slot = 9; //slot time in microseconds
	uint32_t sifs = 10; //SIFS duration in microseconds
	uint32_t ackTimeout = 88; //ACK timeout duration in microseconds
	uint32_t ctsTimeout = 88; //CTS timeout duration in microseconds
	uint32_t rifs = 2; //RIFS duration in microseconds
	uint32_t basicBlockAckTimeout = 286; //Basic Block ACK timeout duration in microseconds
	uint32_t compressedBlockAckTimeout = 112; //Compressed Block ACK timeout duration in microseconds                

	/* 
	* SIFS and Slot time is specified and DIFS it calculates according to the formula DIFS= 2*slot time + SIFS
	* Here slot time = 20 (can be 20 or 9 acc to 802.11g)
	* And  sifs = 10, beacuse difs can be 28 or 50.
	* ye jo ack time hai nad baki cheezen hain woh maine uss code sey li hain.. toh... i d k abhi unka kya kaam hai.. but rest 
	 upar mentioned.

	*/


        CommandLine cmd;
        cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
        cmd.AddValue ("dataRate", "Application data ate", dataRate);
	cmd.AddValue ("datARate", "Application data ate", datARate);
        cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ", tcpVariant);
        cmd.AddValue ("phyRate", "Physical layer bitrate", phyRate);
        cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
        cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
	cmd.AddValue ("maxBytes","Total number of bytes for application to send", maxBytes);
	cmd.AddValue ("slot", "Slot time in microseconds", slot);
	cmd.AddValue ("sifs", "SIFS duration in microseconds", sifs);
	cmd.AddValue ("ackTimeout", "ACK timeout duration in microseconds", ackTimeout);
	cmd.AddValue ("ctsTimeout", "CTS timeout duration in microseconds", ctsTimeout);
	cmd.AddValue ("rifs", "RIFS duration in microseconds", rifs);
	cmd.AddValue ("basicBlockAckTimeoutTimeout", "Basic Block ACK timeout duration in microseconds", basicBlockAckTimeout);
	cmd.AddValue ("compressedBlockAckTimeoutTimeout", "Compressed Block ACK timeout duration in microseconds", compressedBlockAckTimeout);
	cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
        cmd.Parse (argc, argv);

        tcpVariant = std::string ("ns3::") + tcpVariant;

        Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2201"));
        Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2201"));
        Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

        Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

        NodeContainer clients;
        clients.Create (num_clients);
        NodeContainer csma;
        csma.Add (clients.Get(0));
    	csma.Create(1);

        NqosWifiMacHelper mac=NqosWifiMacHelper::Default();   // ??
        WifiHelper wifiHelper;
        wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211g);


        YansWifiChannelHelper wifiChannel;
        wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
        wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));


        YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
        wifiPhy.SetChannel (wifiChannel.Create ());
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
        wifiPhy.Set ("TxPowerStart", DoubleValue (10.0));
        wifiPhy.Set ("TxPowerEnd", DoubleValue (10.0));
        wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
        wifiPhy.Set ("TxGain", DoubleValue (0));
        wifiPhy.Set ("RxGain", DoubleValue (0));
        wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));
        wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-79));
        wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-79 + 3));
        wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
        wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (phyMode),
                                                                      "ControlMode", StringValue (phyMode));

        /* Configure AP */
        Ssid ssid = Ssid ("network");
        mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
    
        NetDeviceContainer ap_device;
        ap_device = wifiHelper.Install (wifiPhy, mac, clients.Get (0));
    
        /* Configure STA */
        mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
    
        NetDeviceContainer cli_devices;

	for(int i=1;i<=50;i++)
	{
		cli_devices.Add(wifiHelper.Install (wifiPhy, mac, clients.Get(i)));    
	}

	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Slot", TimeValue (MicroSeconds (slot)));
	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Sifs", TimeValue (MicroSeconds (sifs)));
	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/AckTimeout", TimeValue (MicroSeconds (ackTimeout)));
	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/CtsTimeout", TimeValue (MicroSeconds (ctsTimeout)));
	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Rifs", TimeValue (MicroSeconds (rifs)));
	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/BasicBlockAckTimeout", TimeValue (MicroSeconds (basicBlockAckTimeout)));
	  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/CompressedBlockAckTimeout", TimeValue (MicroSeconds (compressedBlockAckTimeout)));



 
	cli_devices.Add(ap_device.Get(0));
	MobilityHelper  mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (clients);
	mobility.Install (csma.Get(1));
 
	CsmaHelper csma_helper;
  	csma_helper.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
//	csma_helper.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));


	NetDeviceContainer csma_devices;
	csma_devices=csma_helper.Install (csma);
/*   
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector (0.0, 0.0, 0.0));
        positionAlloc->Add (Vector (1.0, 1.0, 0.0));
    
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (clients);
 
*/    
        /* Internet stack */
        InternetStackHelper stack;
        stack.Install (clients);
	stack.Install(csma.Get(1));
    
        Ipv4AddressHelper address;
        address.SetBase ("10.0.1.0", "255.255.255.0");
        Ipv4InterfaceContainer staInterface;
        staInterface = address.Assign (cli_devices);

//	Ipv4InterfaceContainer apInterface;
//	apInterface = address.Assign (ap_device);  
        address.SetBase ("10.0.2.0", "255.255.255.0");
	Ipv4InterfaceContainer ServInterface;
        ServInterface = address.Assign (csma_devices);
    	
/*	Populate routing table */
    
/*	Install TCP Receiver on the access point */
        PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (ServInterface.GetAddress (1), 21));
	ApplicationContainer sinkApp;
	for(int i=0;i<=50;i++)
    	{
		sinkApp.Add(sinkHelper.Install (clients.Get(i)));

	} 
 sinkApp.Add(sinkHelper.Install (csma.Get (1)));
	  
        sinkApp.Start (Seconds (0.0));

	ApplicationContainer serverApp1;
	
	for(int j=0;j<=50;j++)
	{
		BulkSendHelper source1 ("ns3::TcpSocketFactory",(InetSocketAddress (staInterface.GetAddress (j), 21)));
//		std::cout<< staInterface.GetAddress (j)<<std::endl;
		source1.SetAttribute ("MaxBytes", UintegerValue (1048760));	 //10mb
		serverApp1.Add(source1.Install (csma.Get (1)));
//		std::cout<< ServInterface.GetAddress (1) << std::endl;



		
	}
        serverApp1.Start (Seconds (2.0));

	ApplicationContainer serverApp2;
	
		OnOffHelper source2 ("ns3::TcpSocketFactory", (InetSocketAddress (ServInterface.GetAddress (1), 21)));
		source2.SetAttribute ("PacketSize", UintegerValue (payloadSize));
		source2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		source2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		source2.SetAttribute ("DataRate", DataRateValue (DataRate ("5Mbps")));  // 1mbps
		source2.SetAttribute ("MaxBytes", UintegerValue (1024));              // 5mb   5242880
		serverApp2.Add(source2.Install (clients));
		
	
	
	 serverApp2.Start (Seconds (1.0));

	
	
//	ApplicationContainer serverApp;
//	BulkSendHelper source ("ns3::TcpSocketFactory",(InetSocketAddress (ServInterface.GetAddress (0), 21)));
//	source.SetAttribute ("MaxBytes", UintegerValue (60000)); //total number of packets to be sent. 0 is for unlimited
//	serverApp.Add(source.Install (p2p.Get (1)));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
//	std::cout<<ServInterface.GetAddress (1);
/*	Install TCP/UDP Transmitter on the station */
/*	ApplicationContainer serverApp;
	OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 9)));
	server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
	server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
	server.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
	serverApp = server.Install (p2p.Get(1));
*/
        

/*	Start Applications */

//    serverApp.Start (Seconds (1.0));

	/*AnimationInterface anim ("anim10.xml");
	anim.SetConstantPosition (csma.Get(0), 1.0, 2.0);
	anim.SetConstantPosition (clients.Get(0), 15.0, 12.0);
	anim.SetConstantPosition (clients.Get(1), 22.0, 20.0);
	anim.SetConstantPosition (clients.Get(2), 29.0, 28.0);
	anim.SetConstantPosition (clients.Get(3), 36.0, 36.0);
	anim.SetConstantPosition (clients.Get(4), 43.0, 44.0);
	anim.SetConstantPosition (clients.Get(5), 50.0, 52.0);
	anim.SetConstantPosition (clients.Get(6), 57.0, 60.0);
	anim.SetConstantPosition (clients.Get(7), 64.0, 68.0);
	anim.SetConstantPosition (clients.Get(8), 71.0, 76.0);
	anim.SetConstantPosition (clients.Get(9), 78.0, 84.0);
	anim.SetConstantPosition (clients.Get(10), 85.0, 92.0);
	anim.SetConstantPosition (clients.Get(11), 92.0, 97.0);
	anim.SetConstantPosition (clients.Get(12), 8.0, 10.0);


*/
        /* Enable Traces */
        if (pcapTracing)
        {
            wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
            wifiPhy.EnablePcap ("XYZ", ap_device);
            wifiPhy.EnablePcap ("client", cli_devices);
        }

	FlowMonitorHelper flowHelper;
	Ptr<FlowMonitor> monitor;
	if (flow_monitor)
        {
              monitor=flowHelper.InstallAll ();
        }
	/* Start Simulation */
        Simulator::Stop (Seconds (simulationTime + 1));
        Simulator::Run ();

	for(int i=0;i<=51;i++)
	{
		sink = DynamicCast<PacketSink> (sinkApp.Get(i));
		lastTotalRx =0;
		simulationTime=100;
		while(1)
		{
			if(sink->GetTotalRx ()==lastTotalRx)
			{
			//simulationTime=start.GetSeconds ()-0.1;
				break;
			}
			lastTotalRx = sink->GetTotalRx ();
		}
//		std::cout << "Total Bytes Received by Client  " << i+1<< " = " << sink->GetTotalRx () << std::endl;
	}
	monitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
	FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
	{
//	first 2 FlowIds are for ECHO apps, we don't want to display them
//	Duration for throughput measurement is 9.0 seconds, since 
//	StartTime of the OnOffApplication is at about "second 1"
//	and 
//	Simulator::Stops at "second 10".
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	      	if ((t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.1")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.2")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.3")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.4")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.5")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.6")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.7")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.8")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.9")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.10")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.11")||(t.sourceAddress=="10.0.2.2" && t.destinationAddress == "10.0.1.12")||/**/(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.1")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.2")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.3")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.4")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.5")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.6")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.7")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.8")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.9")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.10")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.11")||(t.destinationAddress=="10.0.2.2" && t.sourceAddress == "10.0.1.12"))
      		{
        		std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        	  	std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
        	  	std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      		  	std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->			 second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";	
			agg+=i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->			second.timeFirstTxPacket.GetSeconds())/1024/1024;
			std::cout << "Completion Time: "<< i->second.timeLastRxPacket.GetSeconds() << "\n";
		}
	} 
	std::cout << "Aggregate Throughput: "<< agg << "\n";
	if(flow_monitor)
	{
		flowHelper.SerializeToXmlFile ("FlowMon.xml", true, true);
	}
        Simulator::Destroy ();
        return 0;
}

