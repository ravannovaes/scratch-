/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * \file rem-example.cc
 * \ingroup examples
 * \brief REM Creation Example
 *
 * This example describes how to setup a simulation using NrRadioEnvironmentMapHelper.
 *
 * We provide a number of simulation parameters that can be configured in the
 * command line, such as the number of UEs per cell or the number of rows and
 * columns of the gNB and Ue antennas.
 * Please have a look at the possible parameters to know what you can configure
 * through the command line.
 * The user can also specify the type of REM map (BeamShape/CoverageArea/UeCoverage)
 * he wishes to generate with some of the following commands:
 * \code{.unparsed}
   $ ./waf --run "rem-example --simTag=d --remMode=CoverageArea"
   $ ./waf --run "rem-example --simTag=u --remMode=UeCoverage"
   $ ./waf --run "rem-example --simTag=b1 --remMode=BeamShape --typeOfRem=DlRem"
   $ ./waf --run "rem-example --simTag=b2 --remMode=BeamShape --typeOfRem=UlRem"
   \endcode
 *
 * DL or UL REM map can be selected by passing to the rem helper the desired
 * transmitting device(s) (RTD(s)) and receiving device (RRD), which for the DL
 * case correspond to gNB(s) and UE and for the UL case to UE(s) and gNB.
 *
 * The output of the REM includes a map with the SNR values, a map with the SINR
 * and a map with IPSD values (aggregated rx Power in each rem point).
 * Note that in case there is only one gNB configured, the SNR/SINR maps will be the same.
 *
 * The output of this example are REM csv files from which can be generated REM
 * figures with the following command:
 * \code{.unparsed}
   $ gnuplot -p nr-rem-{simTag}-gnbs.txt nr-rem-{simTag}-ues.txt nr-rem-{simTag}-buildings.txt nr-rem-{simTag}-plot-rem.gnuplot
   \endcode
 *
 * If no simTag is specified then to plot run the following command:
 *
 * \code{.unparsed}
   $ gnuplot -p nr-rem--gnbs.txt nr-rem--ues.txt nr-rem--buildings.txt nr-rem--plot-rem.gnuplot
   \endcode
 *
 * And the following files will be generated (in the root project folder if not specified
 * differently): nr-rem--sinr.png, nr-rem--snr.png and nr-rem--ipsd.png
 *
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
  std::string remMode = "CoverageArea";
  std::string simTag = "";
 
  std::string beamforming = "search-omni"; //beamforming at gNB and UE, the first is gNB and the second is UE
  enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::UMa;
   //enum BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::InH_OfficeMixed;

  //Rem parameters
  double xMin = -40.0;
  double xMax = 80.0;
  uint16_t xRes = 50;
  double yMin = -70.0;
  double yMax = 50.0;
  uint16_t yRes = 50;
  double z = 1.5;
 
  bool Rem = false;

  std::string outputDir = "./";

  uint16_t gNbNum = 1;
  uint16_t ueNum = 1;
  std::string deploymentScenario = "SingleGnb";
  std::string typeOfRem = "DlRem";

  double gNB1x = 0.0;
  double gNB1y = 0.0;
 

  double uex = 10.0;
  double ue1y = 00.0;

  //Largura de banda e frequência central utilizada para 27 GHz.
  double bandwidthBand27 = 0.85e9;
  double centralFrequencyBand27 = 27.7e9;

  //Configurações cenário 1: com c.a constiuido de duas Bwps. 
  double centralFrequencyBwp0 =27.6e9;
  double centralFrequencyBwp1 =27.8e9;
  double bandwidthcc0 = 200e6;
  double bandwidthcc1 = 200e6;
  uint16_t numerologyBwp0 = 3;
  uint16_t numerologyBwp1 = 3;

  //Configurações cenário 2: sem c.a e uma unica uma Bwp. 
  double centralFrequencyBwp2 =27.7e9;
  double bandwidthcc2= 400e6;
  uint16_t numerologyBwp2 = 3;

  //Escolha do cenario 
  int cenario = 1;
 
  std::string pattern = "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
  std::string patternDL = "DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|";
  std::string patternUL = "UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|";
  std::string operationMode = "TDD";  // TDD or FDD (mixed TDD and FDD mode)

  //Caracterização do Canal 
  int channel_update = 0;
  std::string scenario = "InH-OfficeMixed"; //scenario
  bool Shadowing = false ;

 //Tipo de trafego na aplição
  std :: string trafego = "intervalofixo"; // ou poisson

  //Antenna Parameters
  double hBS;   //Depend on the scenario (no input parameters)
  double hUT;
  uint32_t numRowsUe = 1; //2
  uint32_t numColumnsUe = 1; //2
  uint32_t numRowsGnb = 1; //4
  uint32_t numColumnsGnb = 1; //4
  bool isoUe = true;
  bool isoGnb = false; //false

  double simTime = 1; // in seconds
  bool logging = false;
  bool enableTraces = false;

  //Potência de transmissão das antenas  
  double totalTxPower = 26;
  
  //building parameters in case of buildings addition
  bool enableBuildings = false; //Depends on the scenario (no input parameter)
  uint32_t numOfBuildings = 1;
  uint32_t apartmentsX = 2;
  uint32_t nFloors = 1;

  double x;
  double totalBandwidth ;
   

//Aplicação 
  uint16_t dlPortVoice = 1234;
  uint16_t dlPortVideo = 1236;
  uint16_t dlPortLowLat = 1234;

 
  uint32_t udpPacketSizeUll = 915;
  uint32_t udpPacketSizeBe = 915;
  uint32_t lambdaull = 10000;
  uint32_t lambdaBe = 66266; 

  bool enableVideo = true;
  bool enableVoice = true;
  bool enableLowLat = true;
  uint64_t rngRun = 1;
 
  uint32_t PacketSize = 925;
  std::string Lambda = "0.0000000000001";

  CommandLine cmd;
  //Banda
  cmd.AddValue ("outputDir",
   "configuration the bands", outputDir);
  
  cmd.AddValue ("cenario",
   "configuration the bands", cenario);

   cmd.AddValue ("simTime",
   "tempo de simulaçao", simTime);
  cmd.AddValue ("centralFrequencyBand27",
                "Frequency central band 27",
                centralFrequencyBand27 );
  cmd.AddValue ("centralFrequencyBwp0",
                "Frequency central bwp0",
                centralFrequencyBwp0);
  cmd.AddValue ("centralFrequencyBwp1",
                "Frequency central bwp1",
                centralFrequencyBwp1);
  cmd.AddValue ("centralFrequencyBwp2",
                "Frequency central bwp2",
                centralFrequencyBwp2);
  cmd.AddValue ("bandwidthBand27",
                "Frequency central bwp2",
                bandwidthBand27);
   cmd.AddValue ("bandwidthcc0",
                "The bandwidth of the CCs ",
                bandwidthcc1);
  cmd.AddValue ("bandwidthcc1",
                "The bandwidth of the CCs ",
                bandwidthcc1);
  cmd.AddValue ("bandwidthcc2",
                "The bandwidth of the CCs ",
                bandwidthcc2);
  cmd.AddValue ("numerologyBwp0",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp0);
  cmd.AddValue ("numerologyBwp1",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp1);
  cmd.AddValue ("numerologyBwp2",
                "The numerology to be used in bandwidth part 2",
                numerologyBwp2);
  //Canal 
  cmd.AddValue ("deploymentScenario",
                "The deployment scenario for the simulation. Choose among "
                "'SingleGnb', 'TwoGnbs'.",
                deploymentScenario);
  cmd.AddValue ("Shadowing",
                "directory where to store simulation results",
                Shadowing);
  cmd.AddValue ("channel_update",
                "directory where to store simulation results",
                channel_update );
  //REM
  cmd.AddValue ("remMode",
                "What type of REM map to use: BeamShape, CoverageArea, UeCoverage."
                "BeamShape shows beams that are configured in a user's script. "
                "Coverage area is used to show worst-case SINR and best-case SNR maps "
                "considering that at each point of the map the best beam is used "
                "towards that point from the serving gNB and also of all the interfering"
                "gNBs in the case of worst-case SINR."
                "UeCoverage is similar to the previous, just that it is showing the "
                "uplink coverage.",
                remMode);
  cmd.AddValue ("simTag",
                "Simulation string tag that will be concatenated to output file names",
                simTag);
  cmd.AddValue ("scenario",
                "The scenario for the simulation. Choose among 'RMa', 'UMa', "
                "'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen'"
                "'UMa-Buildings', 'UMi-Buildings'.",
                scenario);
  cmd.AddValue ("gNB1x",
                "gNb 1 x position",
                gNB1x);
  cmd.AddValue ("gNB1y",
                "gNb 1 y position",
                gNB1y);
  cmd.AddValue ("uex",
                "ue 1 x position",
                uex);
  cmd.AddValue ("typeOfRem",
                "The type of Rem to generate (DL or UL) in the case of BeamShape option. Choose among "
                "'DlRem', 'UlRem'.",
                typeOfRem);
  cmd.AddValue ("numRowsUe",
                "Number of rows for the UE antenna",
                numRowsUe);
  cmd.AddValue ("numColumnsUe",
                "Number of columns for the UE antenna",
                numColumnsUe);
  cmd.AddValue ("isoUe",
                "If true (set to 1), use an isotropic radiation pattern in the Ue ",
                isoUe);
  cmd.AddValue ("numRowsGnb",
                "Number of rows for the gNB antenna",
                numRowsGnb);
  cmd.AddValue ("numColumnsGnb",
                "Number of columns for the gNB antenna",
                numColumnsGnb);
  cmd.AddValue ("isoGnb",
                "If true (set to 1), use an isotropic radiation pattern in the gNB ",
                isoGnb);
  cmd.AddValue ("numOfBuildings",
                "The number of Buildings to deploy in the scenario",
                numOfBuildings);
  cmd.AddValue ("beamforming",
                "If dir-dir configure direct-path at both gNB and UE; "
                "if dir-omni configure direct-path at gNB and quasi-omni at UE;"
                "if omni-dir configure quasi-omni at gNB and direct-path at UE;",
                beamforming);
  cmd.AddValue ("logging",
                "Enable logging"
                "another option is by exporting the NS_LOG environment variable",
                logging);
  cmd.AddValue ("xMin",
                "The min x coordinate of the rem map",
                xMin);
  cmd.AddValue ("xMax",
                "The max x coordinate of the rem map",
                xMax);
  cmd.AddValue ("xRes",
                "The resolution on the x axis of the rem map",
                xRes);
  cmd.AddValue ("yMin",
                "The min y coordinate of the rem map",
                yMin);
  cmd.AddValue ("yMax",
                "The max y coordinate of the rem map",
                yMax);
  cmd.AddValue ("yRes",
                "The resolution on the y axis of the rem map",
                yRes);
  cmd.AddValue ("z",
                "The z coordinate of the rem map",
                z);
  //Aplicação
   cmd.AddValue ("packetSizeUll",
                "packet size in bytes to be used by ultra low latency traffic",
                udpPacketSizeUll);
  cmd.AddValue ("packetSizeBe",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("lambdaull",
                "Number of UDP packets in one second for ultra low latency traffic",
                lambdaull);
  cmd.AddValue ("lambdaBe",
                "Number of UDP packets in one second for best effor traffic",
                lambdaBe);
  cmd.AddValue ("enableLowLat",
                "If true, enables low latency traffic transmission (DL)",
                enableLowLat);
  cmd.AddValue ("enableVideo",
                "If true, enables video traffic transmission (DL)",
                enableVideo);
  cmd.AddValue ("enableVoice",
                "If true, enables voice traffic transmission (UL)",
                enableVoice);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  //onoff
  cmd.AddValue ("PacketSize",
                "directory where to store simulation results",
                PacketSize);
  cmd.AddValue ("Lambda",
                "directory where to store simulation results",
                 Lambda);
  cmd.Parse (argc, argv);
  cmd.AddValue ("RngRun",
                "Rng run random number.",
                rngRun);


  //Banda
  std :: cout << "##################### Parâmetros de simulação: #####################" << std :: endl; 
  std :: cout << "Configuração 1 com CA / Configuração 2 sem CA, configuração escolhida : " <<"Cenario "<< cenario <<" com CA"<< std :: endl;
  std :: cout << "SimTime: " <<simTime << "s" << std :: endl;
  std :: cout << "REM: " << Rem << std :: endl;
  std :: cout << "centralFrequencyBand: " << centralFrequencyBand27 << "GHz" << std :: endl;
  std :: cout << "centralFrequencyBwp0 " << centralFrequencyBwp0 << "GHz" << std :: endl;
  std :: cout << "centralFrequencyBwp1 " << centralFrequencyBwp1 << "GHz" << std :: endl;
  std :: cout << "centralFrequencyBwp2 " << centralFrequencyBwp2 << "GHz" << std :: endl;
  std :: cout << "Distância entre ue e gNB: " << uex <<" m"<<std :: endl; 
  std :: cout << "bandwidthcc0: " << bandwidthcc0 << "MHz" << std :: endl;
  std :: cout << "bandwidthcc1: " << bandwidthcc1 << "MHz" << std :: endl;
  std :: cout << "bandwidthcc2: " << bandwidthcc2 << "MHz" << std :: endl;
  std :: cout << "numerologyBwp0: " << bandwidthcc0 << "MHz" << std :: endl;
  std :: cout << "numerologyBwp1: " << bandwidthcc1 << "MHz" << std :: endl;
  std :: cout << "numerologyBwp2: " << bandwidthcc2<< "MHz" << std :: endl;
  std :: cout << "totalTxPower: " << totalTxPower << "MHz" << std :: endl;
  std :: cout << "Pattern: " << pattern << std :: endl;
  std :: cout << "PatternDL Não Utilizado: " << patternDL << std :: endl;
  std :: cout << "PatternUL Não Utilizado: " << patternUL << std :: endl;
  std :: cout << "BeamformingMethod : " << beamforming << std :: endl;
  //std :: cout << "BeamSearchAngleStep:" << beamSearchAngleStep <<  std :: endl;
  std :: cout << "PacketSize onoff: " << PacketSize << std :: endl;
  std :: cout << "Lambda onoff : " << Lambda << std :: endl;
  std :: cout << "udpPacketSizeUll: " << udpPacketSizeUll <<  std :: endl;
  std :: cout << "udpPacketSizeBe: " << udpPacketSizeBe <<  std :: endl;
  std :: cout << "lambdaUll: " << lambdaull << std :: endl;
  std :: cout << "lambdaBe: " << lambdaBe << std :: endl;
  std :: cout << "enableVideo: " << enableVideo << std :: endl;
  std :: cout << "enableVoice: " << enableVoice << std :: endl;
  std :: cout << "enableLowLat: " << enableLowLat << std :: endl;
  std :: cout << "Channel Update: " << channel_update << std :: endl;
  std :: cout << "Cenario Pathloss: " << scenario << std :: endl;
  //antena
  std :: cout <<"numRowsUe " << numRowsUe << std :: endl;
  std :: cout <<"numColumnsUe " << numColumnsUe << std :: endl;
  std :: cout <<"numRowsGnb " << numRowsGnb << std :: endl;
  std :: cout <<" numColumnsGnb  " << numColumnsGnb << std :: endl;
  std :: cout << "Shadowing:" << Shadowing  << std :: endl;
  std :: cout << "############################# RESULTADOS ############################# "<< std :: endl;

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
  std::string errorModel = "ns3::NrLteMiErrorModel";


  if (scenario.compare ("InH-OfficeMixed") == 0)
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
  else if (scenario.compare ("UMa") == 0)
    {
      //hBS = 25;
      hBS = 1.5;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMa;
    }
  else
    {
      NS_ABORT_MSG ("Scenario not supported. Choose among 'RMa', 'UMa', "
                    "'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen',"
                    "'UMa-Buildings', and 'UMi-Buildings'.");
    }

  if (deploymentScenario.compare ("SingleGnb") == 0)
    {
      gNbNum = 1;
      ueNum = 1;
    }
    
  //double offset = 80;

  // create base stations and mobile terminals
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  gnbNodes.Create (gNbNum);
  ueNodes.Create (ueNum);

  // position the base stations
  Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator> ();
  gnbPositionAlloc->Add (Vector (gNB1x, gNB1y, hBS));
  
  MobilityHelper gnbMobility;
  gnbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gnbMobility.SetPositionAllocator (gnbPositionAlloc);
  gnbMobility.Install (gnbNodes);

  // position the mobile terminals
  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ueMobility.Install (ueNodes);

  ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (uex, ue1y, hUT));
 
  if (enableBuildings)
    {
      Ptr<GridBuildingAllocator> gridBuildingAllocator;
      gridBuildingAllocator = CreateObject<GridBuildingAllocator> ();
      gridBuildingAllocator->SetAttribute ("GridWidth", UintegerValue (numOfBuildings));
      gridBuildingAllocator->SetAttribute ("LengthX", DoubleValue (2 * apartmentsX));
      gridBuildingAllocator->SetAttribute ("LengthY", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("DeltaX", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("DeltaY", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("Height", DoubleValue (3 * nFloors));
      gridBuildingAllocator->SetBuildingAttribute ("NRoomsX", UintegerValue (apartmentsX));
      gridBuildingAllocator->SetBuildingAttribute ("NRoomsY", UintegerValue (2));
      gridBuildingAllocator->SetBuildingAttribute ("NFloors", UintegerValue (nFloors));
      gridBuildingAllocator->SetAttribute ("MinX", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("MinY", DoubleValue (10));
      gridBuildingAllocator->Create (numOfBuildings);

      BuildingsHelper::Install (gnbNodes);
      BuildingsHelper::Install (ueNodes);
    }

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  
  nrHelper->SetDlErrorModel (errorModel);


  nrHelper->SetGnbDlAmcAttribute ("NumRefScPerRb", UintegerValue (2));
  nrHelper->SetGnbUlAmcAttribute ("NumRefScPerRb", UintegerValue (2));
  nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
  nrHelper->SetSchedulerAttribute ("DlCtrlSymbols", UintegerValue (1));
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));


  uint8_t numCcs1 = 2;
  uint8_t numCcs2 = 1;

  BandwidthPartInfoPtrVector allBwps;

  OperationBandInfo band27;
  //OperationBandInfo band27;

  std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());
  
  std::unique_ptr<ComponentCarrierInfo> cc1 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());

  std::unique_ptr<ComponentCarrierInfo> cc2 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp2 (new BandwidthPartInfo ());
  
  //(CC1 - BWP0)
  // Component Carrier 0

  if (cenario == 1) {

  band27.m_centralFrequency  = centralFrequencyBand27;
  band27.m_channelBandwidth = bandwidthBand27;
  band27.m_lowerFrequency = band27.m_centralFrequency - band27.m_channelBandwidth / 2;
  band27.m_higherFrequency = band27.m_centralFrequency + band27.m_channelBandwidth / 2;
 

  cc0->m_ccId = 0;
  cc0->m_centralFrequency = centralFrequencyBwp0;
  cc0->m_channelBandwidth = bandwidthcc0;
  cc0->m_lowerFrequency = cc0->m_centralFrequency - cc0->m_channelBandwidth / 2;
  cc0->m_higherFrequency = cc0->m_centralFrequency + cc0->m_channelBandwidth / 2;

  // BWP 0
  
  bwp0->m_bwpId = 0;
  bwp0->m_centralFrequency = cc0->m_centralFrequency;
  bwp0->m_channelBandwidth = cc0->m_channelBandwidth;
  bwp0->m_lowerFrequency = cc0->m_lowerFrequency;
  bwp0->m_higherFrequency = cc0->m_higherFrequency;
  bwp0->m_scenario =scenarioEnum; 

  cc0->AddBwp (std::move (bwp0));

  // Component Carrier 1
 
  cc1->m_ccId = 1;
  cc1->m_centralFrequency = centralFrequencyBwp1;
  cc1->m_channelBandwidth = bandwidthcc1;
  cc1->m_lowerFrequency = cc1->m_centralFrequency - cc1->m_channelBandwidth / 2;
  cc1->m_higherFrequency = cc1->m_centralFrequency + cc1->m_channelBandwidth / 2;

  bwp1->m_bwpId = 1;
  bwp1->m_centralFrequency = cc1->m_centralFrequency;
  bwp1->m_channelBandwidth = cc1->m_channelBandwidth;
  bwp1->m_lowerFrequency = cc1->m_lowerFrequency;
  bwp1->m_higherFrequency = cc1->m_higherFrequency;
  bwp1->m_scenario = scenarioEnum; 

  cc1->AddBwp (std::move (bwp1));
   
  band27.AddCc (std::move (cc0));
  band27.AddCc (std::move (cc1));


  }

   if (cenario == 2) {

  band27.m_centralFrequency  = centralFrequencyBand27;
  band27.m_channelBandwidth = bandwidthBand27;
  band27.m_lowerFrequency = band27.m_centralFrequency - band27.m_channelBandwidth / 2;
  band27.m_higherFrequency = band27.m_centralFrequency + band27.m_channelBandwidth / 2;
 
  cc2->m_ccId = 0;
  cc2->m_centralFrequency = centralFrequencyBwp2;
  cc2->m_channelBandwidth = bandwidthcc2;
  cc2->m_lowerFrequency = cc2->m_centralFrequency - cc2->m_channelBandwidth / 2;
  cc2->m_higherFrequency = cc2->m_centralFrequency + cc2->m_channelBandwidth / 2;

  bwp2->m_bwpId = 0;
  bwp2->m_centralFrequency = cc2->m_centralFrequency;
  bwp2->m_channelBandwidth = cc2->m_channelBandwidth;
  bwp2->m_lowerFrequency = cc2->m_lowerFrequency;
  bwp2->m_higherFrequency = cc2->m_higherFrequency;
  bwp2->m_scenario =scenarioEnum; 

  cc2->AddBwp (std::move (bwp2));   

  band27.AddCc (std::move (cc2));

  } 

 
 // epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));


  //Initialize channel and pathloss, plus other things inside band.
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (channel_update)));
  //checar
 // nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (Shadowing));
  

  nrHelper->InitializeOperationBand (&band27);
  
  allBwps = CcBwpCreator::GetAllBwps ({band27});



  // Configure beamforming method
  if (beamforming.compare ("dir-dir") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ())); // dir at gNB, dir at UE
    }
  else if (beamforming.compare ("dir-omni") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathQuasiOmniBeamforming::GetTypeId ())); // dir at gNB, q-omni at UE
    }
  else if (beamforming.compare ("omni-dir") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ())); // q-omni at gNB, dir at UE
    }
  else if (beamforming.compare ("search-omni") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId())); // q-omni at gNB, dir at UE
    }
  else
    {
      NS_FATAL_ERROR ("Beamforming does not exist:" << beamforming);
    }

  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (numRowsUe));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (numColumnsUe));
  //Antenna element type for UEs
  if (isoUe)
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }
  // Antennas for the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (numRowsGnb));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (numColumnsGnb));
  //Antenna element type for gNBs
  if (isoGnb)
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }
 
  //Assign each flow type to a BWP
  //uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVoice = 0;
  uint32_t bwpIdForVideo = 1;
  uint32_t bwpIdForLowLat = 0;

  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));

  if (cenario == 1 ) {
  	nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  	nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  	nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  	nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  }
  if (cenario == 2) {
  	nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  	nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  }
  
  // install nr net devices
  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  
  int64_t randomStream = rngRun;
  randomStream += nrHelper->AssignStreams (gnbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);
  

 if (cenario==1) {
    
    x = pow (10, totalTxPower / 10);
    totalBandwidth = numCcs1 * bandwidthcc1;
  
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp0));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band27.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  //nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetTxPower (txPower);
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("RbOverhead", DoubleValue (0.1));

  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
  //nrHelper->GetGnbPhy (gnbNetDev.Get (0), 1)->SetTxPower (txPower);
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 1)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band27.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 1)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 1)->SetAttribute ("RbOverhead", DoubleValue (0.1));

 }
 else {
 	
    x = pow (10, totalTxPower / 10);
    totalBandwidth = numCcs2 * bandwidthcc2;
     
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band27.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("RbOverhead", DoubleValue (0.1));

 }

  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = gnbNetDev.Begin (); it != gnbNetDev.End (); ++it)
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


//  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // assign IP address to UEs, and install UDP downlink applications
  //uint16_t dlPort = 1234;
  //ApplicationContainer clientApps;
 // ApplicationContainer serverApps;


 for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //UdpServerHelper dlPacketSinkHelper (dlPort);
      //serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

      //UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      //dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (1)));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
     // dlClient.SetAttribute ("MaxPackets", UintegerValue (10));
      //dlClient.SetAttribute ("PacketSize", UintegerValue (1500));
      //clientApps.Add (dlClient.Install (remoteHost));
    }

  ApplicationContainer serverApps;

  UdpClientHelper dlClientVideo;
  UdpClientHelper dlClientLowLat;
  UdpClientHelper dlClientVoice;
  
  EpsBearer videoBearer (EpsBearer::NGBR_VIDEO_TCP_PREMIUM);
  EpsBearer voiceBearer (EpsBearer::GBR_CONV_VOICE);
  EpsBearer lowLatBearer (EpsBearer::NGBR_LOW_LAT_EMBB);
 
  Ptr<EpcTft> voiceTft = Create<EpcTft> ();
  Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
  Ptr<EpcTft> videoTft = Create<EpcTft> ();

 OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(0), dlPortLowLat));
 OnOffHelper onoffVoice ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(0), dlPortVoice));
 OnOffHelper onoffVideo ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(0), dlPortVideo));
                
  if (cenario == 1) { 

    if(trafego == "intervalofixo") {

  	    UdpServerHelper dlPacketSinkVoice (dlPortVoice);
  	    serverApps.Add (dlPacketSinkVoice.Install (ueNodes));

  	    UdpServerHelper dlPacketSinkVideo (dlPortVideo);
    	  serverApps.Add (dlPacketSinkVideo.Install (ueNodes));

  	      dlClientVoice.SetAttribute ("RemotePort", UintegerValue (dlPortVoice));
  		  dlClientVoice.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  		  dlClientVoice.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  		  dlClientVoice.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaull)));

  		  EpcTft::PacketFilter dlpfVoice;
          dlpfVoice.localPortStart = dlPortVoice;
          dlpfVoice.localPortEnd = dlPortVoice;
          dlpfVoice.direction = EpcTft::DOWNLINK;
          voiceTft->Add (dlpfVoice);
  
          dlClientVideo.SetAttribute ("RemotePort", UintegerValue (dlPortVideo));
  		  dlClientVideo.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  		  dlClientVideo.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeUll));
  		  dlClientVideo.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaull))); 
    	
  	      EpcTft::PacketFilter dlpfVideo;
  		  dlpfVideo.localPortStart = dlPortVideo;
  		  dlpfVideo.localPortEnd = dlPortVideo;
  		  dlpfVideo.direction = EpcTft::DOWNLINK;
  		  videoTft->Add (dlpfVideo);
      }
      else{
           
             PacketSinkHelper dlPacketSinkVoice ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(0),dlPortVoice));

             PacketSinkHelper dlPacketSinkVideo ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(0),dlPortVideo));

             serverApps.Add (dlPacketSinkVoice.Install (ueNodes));

             serverApps.Add (dlPacketSinkVideo.Install (ueNodes));

             onoffVoice.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"));
             onoffVoice.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean="+Lambda+"]"));
             onoffVoice.SetAttribute ("PacketSize", UintegerValue(PacketSize));
             onoffVoice.SetAttribute ("StartTime", TimeValue (Seconds (0.1)));
             onoffVoice.SetAttribute ("StopTime", TimeValue (Seconds (simTime)));
             
             onoffVideo.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"));
             onoffVideo.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean="+Lambda+"]"));
             onoffVideo.SetAttribute ("PacketSize", UintegerValue(PacketSize));
             onoffVideo.SetAttribute ("StartTime", TimeValue (Seconds (0.1)));
             onoffVideo.SetAttribute ("StopTime", TimeValue (Seconds (simTime)));
             

             //clientApps.Add (onoff.Install (remoteHost));

             EpcTft::PacketFilter dlpfVoice;
             dlpfVoice.localPortStart = dlPortVoice;
             dlpfVoice.localPortEnd = dlPortVoice;
             voiceTft->Add (dlpfVoice);


            
             //clientApps.Add (onoff.Install (remoteHost));

             EpcTft::PacketFilter dlpfVideo;
             dlpfVideo.localPortStart = dlPortVideo;
             dlpfVideo.localPortEnd = dlPortVideo;
             videoTft->Add (dlpfVideo);
         }
    }
   
   if (cenario==2) { 

  
  	   if (trafego == "intervalofixo") {

        UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);

        
    	   serverApps.Add (dlPacketSinkLowLat.Install (ueNodes));

    	   dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
   		   dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
    	   dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
    	   dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaull)));
  // The filter for the low-latency traffic
         EpcTft::PacketFilter dlpfLowLat;
         dlpfLowLat.localPortStart = dlPortLowLat;
         dlpfLowLat.localPortEnd = dlPortLowLat;
         lowLatTft->Add (dlpfLowLat);
      }
      else {
             
             PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(0),dlPortLowLat));

             serverApps.Add (dlPacketSinkHelper.Install (ueNodes));

             
             onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"));
             onoff.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean="+Lambda+"]"));
             onoff.SetAttribute ("PacketSize", UintegerValue(PacketSize));
             onoff.SetAttribute ("StartTime", TimeValue (Seconds (0.1)));
             onoff.SetAttribute ("StopTime", TimeValue (Seconds (simTime)));
             
             //clientApps.Add (onoff.Install (remoteHost));

             EpcTft::PacketFilter dlpfLowLat;
             dlpfLowLat.localPortStart = dlPortLowLat;
             dlpfLowLat.localPortEnd = dlPortLowLat;
             lowLatTft->Add (dlpfLowLat);
      }

  }

	
  ApplicationContainer clientApps;

  if (cenario == 1 ) { 

     for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Node> ue = ueNodes.Get (i);
      Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
      Address ueAddress = ueIpIface.GetAddress (i);
      if(trafego == "intervalofixo") {
            if (enableVideo)
        		{
          			dlClientVideo.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
          			clientApps.Add (dlClientVideo.Install (remoteHost));

          			nrHelper->ActivateDedicatedEpsBearer (ueDevice, videoBearer, videoTft);
        		}
			     if (enableVoice)
              {
           	 		dlClientVoice.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
            		clientApps.Add (dlClientVoice.Install (remoteHost));

            		nrHelper->ActivateDedicatedEpsBearer (ueDevice, voiceBearer, voiceTft);
        		 }  
     }    
     else {     
           if (enableVideo)
             {
                 // dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
                 //clientApps.Add (dlClientLowLat.Install (remoteHost));
                 clientApps.Add (onoffVoice.Install (remoteHost));

                 nrHelper->ActivateDedicatedEpsBearer (ueDevice, videoBearer, videoTft);
              }
           if(enableVoice) {

                 //dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
                 //clientApps.Add (dlClientLowLat.Install (remoteHost));
                 clientApps.Add (onoffVoice.Install (remoteHost));

                 nrHelper->ActivateDedicatedEpsBearer (ueDevice, voiceBearer, voiceTft);
           }  
      }      
    }
  }


if (cenario==2) {

  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
        Ptr<Node> ue = ueNodes.Get (i);
        Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
        Address ueAddress = ueIpIface.GetAddress (i);
          if(trafego=="intervalofixo")
           {   
              if (enableLowLat)
                {
                    dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
                    clientApps.Add (dlClientLowLat.Install (remoteHost));

                    nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
                }   
           }
          else 
          {  
             if (enableLowLat)
             {
                 // dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
                 //clientApps.Add (dlClientLowLat.Install (remoteHost));
                 clientApps.Add (onoff.Install (remoteHost));

                  nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
              } 
          }
  }
}
  // attach UEs to the closest gNB
  nrHelper->AttachToEnb (ueNetDev.Get (0), gnbNetDev.Get (0));
  
  // enable the traces provided by the nr module
  if (enableTraces)
    {
      nrHelper->EnableTraces ();
    }

  uint16_t remBwpId = 0;
  //Radio Environment Map Generation for ccId 0
  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();

  if (Rem) {

  remHelper->SetMinX (xMin);
  remHelper->SetMaxX (xMax);
  remHelper->SetResX (xRes);
  remHelper->SetMinY (yMin);
  remHelper->SetMaxY (yMax);
  remHelper->SetResY (yRes);
  remHelper->SetZ (z);
  remHelper->SetSimTag (simTag);

  gnbNetDev.Get (0)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (0));

  if (remMode == "BeamShape")
    {
      remHelper->SetRemMode (NrRadioEnvironmentMapHelper::BEAM_SHAPE);

      if (typeOfRem.compare ("DlRem") == 0)
        {
          remHelper->CreateRem (gnbNetDev, ueNetDev.Get (0), remBwpId);
        }
      else if (typeOfRem.compare ("UlRem") == 0)
        {
          remHelper->CreateRem (ueNetDev, gnbNetDev.Get (0), remBwpId);
        }
      else
        {
          NS_ABORT_MSG ("typeOfRem not supported. "
                        "Choose among 'DlRem', 'UlRem'.");
        }
    }
  else if (remMode == "CoverageArea")
    {
      remHelper->SetRemMode (NrRadioEnvironmentMapHelper::COVERAGE_AREA);
      remHelper->CreateRem (gnbNetDev, ueNetDev.Get (0), remBwpId);
    }
  else if (remMode == "UeCoverage")
    {
      remHelper->SetRemMode (NrRadioEnvironmentMapHelper::UE_COVERAGE);
      remHelper->CreateRem (ueNetDev, gnbNetDev.Get (0), remBwpId);
    }
  else
    {
      NS_ABORT_MSG ("Not supported remMode.");
    }

   } 

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

int xue ;
xue= uex;

std::string uexstr = std::to_string(xue);
std::string lambdaustr = std::to_string(lambdaull);


dl_results = outputDir+"/"+"DL_Results_ca_distance_"+uexstr+"_lambda_"+lambdaustr+".txt";

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
         if (trafego == "intervalofixo") {

          if ( (( t.destinationPort == dlPortLowLat) || (t.destinationPort == dlPortVoice) || 
            (t.destinationPort == dlPortVideo) ) && (t.sourceAddress == internetIpIfaces.GetAddress(1)) )
            {
              streamMetricsInit = DLstreamMetricsInit;
             }
          // Get file pointer for UL, if UL flow (using port and IP address to assure correct result))
          }
         else {

            if ((t.destinationPort == dlPortLowLat) 
               && ( t.destinationAddress  == ueIpIface.GetAddress(0)))
                { 
                     streamMetricsInit = DLstreamMetricsInit;
               }
          }
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
                   std::cout << "DLport: " << dlPortVideo << std::endl;
                 //std::cout << "ULport: " << ulPortLowLat  << std::endl;
               }
           }
        //  bytesTotal =+ iter->second.rxPackets;
  }

flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);

  Simulator::Destroy ();
  return 0;
}


