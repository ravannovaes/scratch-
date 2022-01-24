/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * \file cttc-3gpp-channel-example.cc
 * \ingroup examples
 * \brief Channel Example
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.900. Topology consists by default of 2 UEs and 2 gNbs, and can be
 * configured to be either mobile or static scenario.
 *
 * The output of this example are default NR trace files that can be found in
 * the root ns-3 project folder.
 */
#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/bandwidth-part-gnb.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"
#include <ns3/buildings-module.h>
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-probe.h"
#include <ns3/antenna-module.h>


using namespace ns3;

int
main (int argc, char *argv[])
{
  std::string scenario = "UMa"; //scenario
  double frequency = 28e9; // central frequency
  double bandwidth = 100e6; //bandwidth
  double mobility = false; //whether to enable mobility
  double simTime = 10; // in second
  double speed = 1; // in m/s for walking UT.
  bool logging = true; //whether to enable logging from the simulation, another option is by exporting the NS_LOG environment variable
  double hBS; //base station antenna height in meters
  double hUT; //user antenna height in meters
  double txPower = 40; // txPower
  enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::UMa;
  uint32_t PacketSize = 125;

 std::string Lambda = "1";

  CommandLine cmd;
  cmd.AddValue ("scenario",
                "The scenario for the simulation. Choose among 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen'.",
                scenario);
  cmd.AddValue ("frequency",
                "The central carrier frequency in Hz.",
                frequency);
  cmd.AddValue ("mobility",
                "If set to 1 UEs will be mobile, when set to 0 UE will be static. By default, they are mobile.",
                mobility);
  cmd.AddValue ("logging",
                "If set to 0, log components will be disabled.",
                logging);
  cmd.Parse (argc, argv);

  // enable logging
  if (logging)
    {
      //LogComponentEnable ("ThreeGppSpectrumPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("ThreeGppPropagationLossModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("ThreeGppChannelModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("ChannelConditionModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      //LogComponentEnable ("LteRlcUm", LOG_LEVEL_LOGIC);
      //LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  // set mobile device and base station antenna heights in meters, according to the chosen scenario
  if (scenario.compare ("RMa") == 0)
    {
      hBS = 35;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::RMa;
    }
  else if (scenario.compare ("UMa") == 0)
    {
      hBS = 25;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMa;
    }
  else if (scenario.compare ("UMi-StreetCanyon") == 0)
    {
      hBS = 10;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMi_StreetCanyon;
    }
  else if (scenario.compare ("InH-OfficeMixed") == 0)
    {
      hBS = 3;
      hUT = 1;
      scenarioEnum = BandwidthPartInfo::InH_OfficeMixed;
    }
  else if (scenario.compare ("InH-OfficeOpen") == 0)
    {
      hBS = 3;
      hUT = 1;
      scenarioEnum = BandwidthPartInfo::InH_OfficeOpen;
    }
  else
    {
      NS_ABORT_MSG ("Scenario not supported. Choose among 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', and 'InH-OfficeOpen'.");
    }


  // create base stations and mobile terminals
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (2);
  ueNodes.Create (2);

  // position the base stations
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, hBS));
  enbPositionAlloc->Add (Vector (0.0, 80.0, hBS));
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);

  // position the mobile terminals and enable the mobility
  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.Install (ueNodes);

  if (mobility)
    {
      ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (90, 15, hUT)); // (x, y, z) in m
      ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, speed, 0)); // move UE1 along the y axis

      ueNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (30, 50.0, hUT)); // (x, y, z) in m
      ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (-speed, 0, 0)); // move UE2 along the x axis
    }
  else
    {
      ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (90, 15, hUT));
      ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));

      ueNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (30, 50.0, hUT));
      ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
    }

  /*
   * Create NR simulation helpers
   */
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum configuration. We create a single operational band and configure the scenario.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;  // in this example we have a single band, and that band is composed of a single component carrier

  /* Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
   * a single BWP per CC and a single BWP in CC.
   *
   * Hence, the configured spectrum is:
   *
   * |---------------Band---------------|
   * |---------------CC-----------------|
   * |---------------BWP----------------|
   */
  CcBwpCreator::SimpleOperationBandConf bandConf (frequency, bandwidth, numCcPerBand, scenarioEnum);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  //Initialize channel and pathloss, plus other things inside band.
  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure ideal beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Configure scheduler
  nrHelper->SetSchedulerTypeId (NrMacSchedulerTdmaRR::GetTypeId ());

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // install nr net devices
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (enbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetTxPower (txPower);
  nrHelper->GetGnbPhy (enbNetDev.Get (1), 0)->SetTxPower (txPower);

  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);

  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

    //  UdpServerHelper dlPacketSinkHelper (dlPort);
  
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(u), dlPort));
  
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));
      

      OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(u), dlPort));
      onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"));
      onoff.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean="+Lambda+"]"));
      onoff.SetAttribute ("PacketSize", UintegerValue(PacketSize));
      onoff.SetAttribute ("StartTime", TimeValue (Seconds (0.1)));
      onoff.SetAttribute ("StopTime", TimeValue (Seconds (simTime)));
      clientApps.Add (onoff.Install (remoteHost));

      //UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      //dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (1)));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue (10));
      //dlClient.SetAttribute ("PacketSize", UintegerValue (1500));
      //clientApps.Add (dlClient.Install (remoteHost));
    }

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // start server and client apps
  serverApps.Start (Seconds (0.4));
  clientApps.Start (Seconds (0.4));
  serverApps.Stop (Seconds (simTime));
  clientApps.Stop (Seconds (simTime - 0.2));

  
 FlowMonitorHelper flowmonHelper;
 NodeContainer endpointNodes;
 endpointNodes.Add (remoteHost);
 endpointNodes.Add (ueNodes);

  //////////////////// Flow Monitor /////////////////////////
AsciiTraceHelper asciiTraceHelper;
Ptr<FlowMonitor> flowMonitor;
FlowMonitorHelper flowHelper;
flowMonitor = flowHelper.Install(endpointNodes);


//////////////////////////////////////////////////////////
Simulator::Stop (Seconds (simTime));
Simulator::Run ();

////////Flow Monitor data///////////////////////////////
static bool verbose = true;
//uint32_t bytesTotal=0;
std::string dl_results,ul_results;
dl_results = "DL_Results_r_txt";
ul_results = "UL_Results_r_.txt";


Ptr<OutputStreamWrapper> DLstreamMetricsInit = asciiTraceHelper.CreateFileStream((dl_results));
*DLstreamMetricsInit->GetStream()
            << "Flow_ID, Lost_Packets, Tx_Packets, Tx_Bytes, TxOffered(Mbps),  Rx_Packets, Rx_Bytes, T_put(Mbps), Mean_Delay_Rx_Packets, Mean_Jitter, Packet_Loss_Ratio"
            << std::endl;

Ptr<OutputStreamWrapper> ULstreamMetricsInit = asciiTraceHelper.CreateFileStream((ul_results));
*ULstreamMetricsInit->GetStream()
            << "Flow_ID, Lost_Packets, Tx_Packets, Tx_Bytes, TxOffered(Mbps),  Rx_Packets, Rx_Bytes, T_put(Mbps), Mean_Delay_Rx_Packets, Mean_Jitter, Packet_Loss_Ratio"
            << std::endl;

double statDurationTX = 0;
double statDurationRX = 0;
Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());

std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter =
    stats.begin(); iter != stats.end(); ++iter)
  {
    // some metrics calculation
    statDurationRX = iter->second.timeLastRxPacket.GetSeconds()
                  - iter->second.timeFirstTxPacket.GetSeconds();
    statDurationTX = iter->second.timeLastTxPacket.GetSeconds()
                  - iter->second.timeFirstTxPacket.GetSeconds();

    double meanDelay, meanJitter, packetLossRatio, txTput, rxTput;
    if (iter->second.rxPackets > 0)
      {
        meanDelay = (iter->second.delaySum.GetSeconds()
            / iter->second.rxPackets);
      }
    else // this value is set to zero because the STA is not receiving any packet
      {
        meanDelay = 0;
      }
    //
    if (iter->second.rxPackets > 1)
      {
        meanJitter = (iter->second.jitterSum.GetSeconds()
            / (iter->second.rxPackets - 1));
      }
    else // this value is set to zero because the STA is not receiving any packet
      {
        meanJitter = 0;
      }
    //
    if (statDurationTX > 0)
      {
        txTput = iter->second.txBytes * 8.0 / statDurationTX / 1000 / 1000;
      }
    else
      {
        txTput = 0;
      }
    //
    if (statDurationRX > 0)
      {
        rxTput = iter->second.rxBytes * 8.0 / statDurationRX / 1000 / 1000;
      }
    else
      {
        rxTput = 0;
      }
    //
    if ((iter->second.lostPackets > 0) & (iter->second.rxPackets > 0))
      {
        packetLossRatio = (double) (iter->second.lostPackets
            / (double) (iter->second.rxPackets + iter->second.lostPackets));
      }
    else
      {
        packetLossRatio = 0;
      }
    //
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);
    //
    if (verbose)
           {
             // Print information only if enabled
             //
             //          std::cout << "Flow ID: " << iter->first << ", Source Port: "
             //              << t.sourcePort << ", Destination Port: " << t.destinationPort
             //              << " (" << t.sourceAddress << " -> " << t.destinationAddress
             //              << ")" << std::endl;
             //
             NS_LOG_UNCOND( "Flow ID: " << iter->first << ", Source Port: "
                 << t.sourcePort << ", Destination Port: " << t.destinationPort
                 << " (" << t.sourceAddress << " -> " << t.destinationAddress
                 << ")");
             //
             NS_LOG_UNCOND( "Lost Packets = " << iter->second.lostPackets);
             //
             NS_LOG_UNCOND( "Tx Packets = " << iter->second.txPackets);
             //
             NS_LOG_UNCOND( "Tx Bytes = " << iter->second.txBytes);
             //
             NS_LOG_UNCOND( "TxOffered = " << txTput << " Mbps");
             //std::cout << "TxOffered = " << txTput << " Mbps" << std::endl;
             //
             NS_LOG_UNCOND( "Rx Packets = " << iter->second.rxPackets);
             //
             NS_LOG_UNCOND( "Rx Bytes = " << iter->second.rxBytes);
             //
             NS_LOG_UNCOND( "T-put = " << rxTput << " Mbps");
             //std::cout << "T-put = " << rxTput << " Mbps" << std::endl;
             //
             NS_LOG_UNCOND( "Mean Delay Rx Packets = " << meanDelay << " s");
             //std::cout << "Mean Delay Rx Packets = " << meanDelay << " s"
             //    << std::endl;
             //
             NS_LOG_UNCOND( "Mean jitter = " << meanJitter << " s");
             //std::cout << "Mean jitter = " << meanJitter << " s" << std::endl;
             //
             NS_LOG_UNCOND( "Packet loss ratio = " << packetLossRatio);
             //std::cout << "Packet loss ratio = " << packetLossRatio << std::endl;
             //
           }
         //
         Ptr<OutputStreamWrapper> streamMetricsInit = NULL;
         // Get file pointer for DL, if DL flow (using port and IP address to assure correct result)
         std::cout << "t destination port " << t.destinationPort  <<std::endl;
         //std::cout << "source address " << interfaces.GetAddress(0)  <<std::endl;
         std::cout << "source address " << t.sourceAddress  <<std::endl;
         std::cout << "t destination port " << t.destinationPort  <<std::endl;
         //std::cout << "sink address " << sinkAddress  <<std::endl;
         std::cout << "destination address " << t.destinationAddress  <<std::endl;
           if ((t.destinationPort == dlPort)
            && (t.destinationAddress== ueIpIface.GetAddress(0)))

             {
               streamMetricsInit = DLstreamMetricsInit;
               
             }
        // if ( (( t.destinationPort == dlPortLowLat) || (t.destinationPort == dlPortVoice) || 
         // (t.destinationPort == dlPortVideo) ) && (t.sourceAddress == internetIpIfaces.GetAddress(1)) )
           //{
             //streamMetricsInit = DLstreamMetricsInit;
           //}
         // Get file pointer for UL, if UL flow (using port and IP address to assure correct result))
         //else if ( (( t.destinationPort == ulPortLowLat) || (t.destinationPort == ulPortVoice) || 
          //(t.destinationPort == ulPortVideo) ) && (t.destinationAddress == t.destinationAddress))          
           //{
             //streamMetricsInit = ULstreamMetricsInit;
           //}
     if (streamMetricsInit)
           {
             *streamMetricsInit->GetStream() << (iter->first ) << ", "
                 << (iter->second.lostPackets) << ", "
                 //
                 << (iter->second.txPackets) << ", "
                 //
                 << (iter->second.txBytes) << ", "
                 //
                 << txTput << ", "
                 //
                 << (iter->second.rxPackets) << ", "
                 //
                 << (iter->second.rxBytes) << ", "
                 //
                 << rxTput << ", "
                 //
                 << meanDelay << ", "
                 //
                 << meanJitter << ", "
                 //
                 << packetLossRatio 
                 //
                 << std::endl;
           }
         else
           {
             //TODO: chance for an ASSERT
             if ( true )
               {
                 std::cout << "Some problem to save metrics" << std::endl;
                 std::cout << "Flow ID: " << iter->first << ", Source Port: "
                     << t.sourcePort << ", Destination Port: " << t.destinationPort
                     << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")" << std::endl;
                 //std::cout << "AP Address: " << i.GetAddress(0) << std::endl;
                   std::cout << "DLport: " << dlPort<< std::endl;
                 //std::cout << "ULport: " << ulPortLowLat  << std::endl;
               }
           }
        //  bytesTotal =+ iter->second.rxPackets;
  }

flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);

  Simulator::Destroy ();
  return 0;
}


