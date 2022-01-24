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

NS_LOG_COMPONENT_DEFINE ("3gppChannelFdmLteComponentCarriersExample");

int
main (int argc, char *argv[])
{

  double xMin = -50.0;
  double xMax = 50.0;
  uint16_t xRes = 50;
  double yMin = -50.0;
  double yMax = 50.0;
  uint16_t yRes = 50;
  uint16_t gNbNum = 1;
  uint16_t ueNum = 1;
  double z = 1.5;

  uint8_t numBands = 1;
  double centralFrequencyBand38 = 27.7e9;
  double centralFrequencyBwp0 =27.6e9;
  double centralFrequencyBwp1 =27.8e9;
  double centralFrequencyBwp2 =27.7e9;

  double bandwidthBand38 = 0.85e9;
  double bandwidthcc0 = 200e6;
  double bandwidthcc1 = 200e6;
  double bandwidthcc2= 400e6;
 
  uint16_t numerologyBwp0 = 3;
  uint16_t numerologyBwp1 = 3;
  uint16_t numerologyBwp2 = 3;

  double x;
  double totalBandwidth ;
  
  double distanceue = 10;

  double totalTxPower = 26;
  double totalRxPower = 23;
  std::string pattern = "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
  std::string patternDL = "DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|";
  std::string patternUL = "UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|";
  std::string operationMode = "TDD";  // TDD or FDD (mixed TDD and FDD mode)

  bool cellScan = true;
  double beamSearchAngleStep = 10.0;

  uint32_t udpPacketSizeUll = 915;
  uint32_t udpPacketSizeBe = 915;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 10000;

 
  bool enableVideo = true;
  bool enableVoice = true;
  bool enableLowLat = true;
  int channel_update = 100;
  std :: string AmcModel = "ErrorModel";
  std :: string tramissao = "DL";
  bool Rem = false ;

  bool logging = false;

  std::string simTag = "default";
  std::string outputDir = "./";

  double simTime = 1.0; // seconds
  double udpAppStartTime = 0.4; //seconds

  std::string cenario_Pathloss = "InH_OfficeMixed";
  enum BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::InH_OfficeMixed;
  

  int cenario = 1;
  bool Shadowing = false ; 

  //uint16_t sector = 0;
  //double theta = 60;

  uint16_t dlPortLowLat = 1234;
  uint16_t ulPortLowLat = 1234;
  uint16_t dlPortVoice = 1234;
  uint16_t dlPortVideo = 1236;
  uint16_t ulPortVoice = 1234;
  uint16_t ulPortVideo = 1236;
  
  uint16_t remBwpId = 0;
  
  CommandLine cmd;

  LogComponentEnable("NrRadioEnvironmentMapHelper", LOG_LEVEL_WARN);
  
  cmd.AddValue ("cenario",
   "configuration the bands", cenario);
  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per gNb in multiple-ue topology",
                ueNum);
  cmd.AddValue ("numBands",
                "Number of operation bands. More than one implies non-contiguous CC",
                numBands);
  cmd.AddValue ("centralFrequencyBand38 ",
                "Frequency central band 38",
                centralFrequencyBand38 );
  cmd.AddValue ("centralFrequencyBwp0",
                "Frequency central bwp0",
                centralFrequencyBwp0);
  cmd.AddValue ("centralFrequencyBwp1",
                "Frequency central bwp1",
                centralFrequencyBwp1);
  cmd.AddValue ("centralFrequencyBwp2",
                "Frequency central bwp2",
                centralFrequencyBwp2);
  cmd.AddValue ("bandwidthBand38",
                "Frequency central bwp2",
                bandwidthBand38);
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
  cmd.AddValue ("distanceue",
                "distancia entre ue e gNb",
                distanceue);
  cmd.AddValue ("tddPattern",
                "LTE TDD pattern to use (e.g. --tddPattern=DL|S|UL|UL|UL|DL|S|UL|UL|UL|)",
                pattern);
  cmd.AddValue ("totalTxPower",
                "LTE TDD pattern to use (e.g. --tddPattern=DL|S|UL|UL|UL|DL|S|UL|UL|UL|)",
                totalTxPower);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD (In this case it"
                "will be mixed TDD and FDD)",
                operationMode);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                "true to use cell scanning method",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("packetSizeUll",
                "packet size in bytes to be used by ultra low latency traffic",
                udpPacketSizeUll);
  cmd.AddValue ("packetSizeBe",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("lambdaUll",
                "Number of UDP packets in one second for ultra low latency traffic",
                lambdaUll);
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
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);
  cmd.AddValue ("AmcModel",
                "directory where to store simulation results",
                AmcModel);
  cmd.AddValue ("tramissao",
                "directory where to store simulation results",
               tramissao);
  cmd.AddValue ("cenario_Pathloss",
                "directory where to store simulation results",
                cenario_Pathloss);
  cmd.AddValue ("Shadowing",
                "directory where to store simulation results",
                Shadowing);
   cmd.AddValue ("Shadowing",
                "directory where to store simulation results",
                channel_update);


  cmd.Parse (argc, argv);

  std :: cout << "##################### Parâmetros de simulação: #####################" << std :: endl; 
  std :: cout << "Configuração 1 com CA / Configuração 2 sem CA, configuração escolhida : " << cenario << std :: endl;
  std :: cout << "SimTime:" << simTime << "s" << std :: endl;
  std :: cout << "centralFrequencyBand:" << centralFrequencyBand38 << "GHz" << std :: endl;
  std :: cout << "Distância entre ue e gNB:" << distanceue << "m"<<std :: endl;
  std :: cout << "centralFrequencyBwp0" << centralFrequencyBwp0 << "GHz" << std :: endl;
  std :: cout << "centralFrequencyBwp1" << centralFrequencyBwp1 << "GHz" << std :: endl;
  std :: cout << "centralFrequencyBwp2" << centralFrequencyBwp2 << "GHz" << std :: endl;
  std :: cout << "bandwidthcc0:" << bandwidthcc0 << "MHz" << std :: endl;
  std :: cout << "bandwidthcc1:" << bandwidthcc1 << "MHz" << std :: endl;
  std :: cout << "bandwidthcc2:" << bandwidthcc2 << "MHz" << std :: endl;
  std :: cout << "numerologyBwp0:" << bandwidthcc0 << "MHz" << std :: endl;
  std :: cout << "numerologyBwp1:" << bandwidthcc1 << "MHz" << std :: endl;
  std :: cout << "numerologyBwp2:" << bandwidthcc2<< "MHz" << std :: endl;
  std :: cout << "totalTxPower:" << totalTxPower << "MHz" << std :: endl;
  std :: cout << "Pattern:" << pattern << std :: endl;
  std :: cout << "PatternDL Não Utilizado:" << patternDL << std :: endl;
  std :: cout << "PatternUL Não Utilizado:" << patternUL << std :: endl;
  std :: cout << "BeamformingMethod cellScan :" << cellScan << std :: endl;
  std :: cout << "BeamSearchAngleStep:" << beamSearchAngleStep <<  std :: endl;
  std :: cout << "udpPacketSizeUll:" << udpPacketSizeUll <<  std :: endl;
  std :: cout << "udpPacketSizeBe:" << udpPacketSizeBe <<  std :: endl;
  std :: cout << "lambdaUll:" << lambdaUll << std :: endl;
  std :: cout << "lambdaBe:" << lambdaBe << std :: endl;
  std :: cout << "enableVideo:" << enableVideo << std :: endl;
  std :: cout << "enableVoice:" << enableVoice << std :: endl;
  std :: cout << "enableLowLat:" << enableLowLat << std :: endl;
  std :: cout << "AmcModel:" << AmcModel << std :: endl;
  std :: cout << "Channel Update:" << channel_update << std :: endl;
  std :: cout << "Cenario Pathloss:" << cenario_Pathloss << std :: endl;
  std :: cout << "Shadowing:" << Shadowing  << std :: endl;
  std :: cout << "############################# RESULTADOS ############################# "<< std :: endl;

  NS_ABORT_IF (numBands < 1);

 // NS_ABORT_MSG_IF (enableLowLat == false && enableVideo == false && enableVoice == false
                //   && enableGaming == false && operationMode == "TDD", "For TDD enable one of the flows");
  if (logging)
    {
//      LogComponentEnable ("Nr3gppPropagationLossModel", LOG_LEVEL_ALL);
//      LogComponentEnable ("Nr3gppBuildingsPropagationLossModel", LOG_LEVEL_ALL);
//      LogComponentEnable ("Nr3gppChannel", LOG_LEVEL_ALL);
//      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
//      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
//      LogComponentEnable ("BwpManagerGnb", LOG_LEVEL_INFO);
//      LogComponentEnable ("BwpManagerAlgorithm", LOG_LEVEL_INFO);
      LogComponentEnable ("NrGnbPhy", LOG_LEVEL_INFO);
      LogComponentEnable ("NrUePhy", LOG_LEVEL_INFO);
//      LogComponentEnable ("NrGnbMac", LOG_LEVEL_INFO);
//      LogComponentEnable ("NrUeMac", LOG_LEVEL_INFO);
    }

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNum);


 // Posição dos nós 

  Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();


  apPositionAlloc->Add (Vector (0.0, 0.0, gNbHeight));

  staPositionAlloc->Add (Vector (10.0,0.0,ueHeight));
     

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.SetPositionAllocator (apPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (staPositionAlloc);
  mobility.Install (ueNodes);

//Características do canal 3gpp
   
   if (cenario_Pathloss == "Inh")
    {
      	scenario=BandwidthPartInfo::InH_OfficeOpen;
    }
    if (cenario_Pathloss == "InH_OfficeMixed")
    {
      	scenario=BandwidthPartInfo::InH_OfficeMixed;
    }
  else if (cenario_Pathloss == "InhLos")
    {
      	 scenario=BandwidthPartInfo::InH_OfficeOpen_LoS;
    }
  else if (cenario_Pathloss == "InhNlos")
    {	
    	 scenario=BandwidthPartInfo::InH_OfficeOpen_nLoS;
    }
  else
    {
      NS_ABORT_MSG ("Not supported scenario:" << scenario);
    }
 
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (Shadowing));
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds (channel_update))); // update the channel at each iteration
  
  if (cellScan)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
      idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));
    }
  else
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ()));
    }
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  std::string errorModel = "ns3::NrLteMiErrorModel";
  // Scheduler
  nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (false));
  nrHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue (false));

  // Error Model: UE and GNB with same spectrum error model.
  nrHelper->SetUlErrorModel (errorModel);
  nrHelper->SetDlErrorModel (errorModel);

  if (AmcModel == "ErrorModel") {
  // Both DL and UL AMC will have the same model behind.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
  }
  else
  {
    nrHelper-> SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc :: ShannonModel)); // NrAmc :: ShannonModel ou NrAmc :: ErrorModel
    nrHelper-> SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc :: ShannonModel)); // NrAmc :: ShannonModel ou NrAmc :: ErrorModel
  }

  /*
   * Adjust the average number of Reference symbols per RB only for LTE case,
   * which is larger than in NR. We assume a value of 4 (could be 3 too).
   */
  nrHelper->SetGnbDlAmcAttribute ("NumRefScPerRb", UintegerValue (2));
  nrHelper->SetGnbUlAmcAttribute ("NumRefScPerRb", UintegerValue (2));
  nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
  nrHelper->SetSchedulerAttribute ("DlCtrlSymbols", UintegerValue (1));
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));



 uint8_t numCcs1 = 2;
 uint8_t numCcs2 = 1;

  BandwidthPartInfoPtrVector allBwps;

  OperationBandInfo band38;
  //OperationBandInfo band27;

  std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());
  
  std::unique_ptr<ComponentCarrierInfo> cc1 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());

  std::unique_ptr<ComponentCarrierInfo> cc2 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp2 (new BandwidthPartInfo ());
  
  if (cenario == 1 ) { 
  
  //(CC1 - BWP0)
  // Component Carrier 0

  band38.m_centralFrequency  = centralFrequencyBand38;
  band38.m_channelBandwidth = bandwidthBand38;
  band38.m_lowerFrequency = band38.m_centralFrequency - band38.m_channelBandwidth / 2;
  band38.m_higherFrequency = band38.m_centralFrequency + band38.m_channelBandwidth / 2;
 

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
  bwp0->m_scenario =scenario; 

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
  bwp1->m_scenario =scenario; 

  cc1->AddBwp (std::move (bwp1));
   
  band38.AddCc (std::move (cc0));
  band38.AddCc (std::move (cc1));

  nrHelper->InitializeOperationBand (&band38);

  allBwps = CcBwpCreator::GetAllBwps ({band38});

  }
  if (cenario == 2) {

  band38.m_centralFrequency  = centralFrequencyBand38;
  band38.m_channelBandwidth = bandwidthBand38;
  band38.m_lowerFrequency = band38.m_centralFrequency - band38.m_channelBandwidth / 2;
  band38.m_higherFrequency = band38.m_centralFrequency + band38.m_channelBandwidth / 2;
 
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
  bwp2->m_scenario =scenario; 

  cc2->AddBwp (std::move (bwp2));   

  band38.AddCc (std::move (cc2));


  nrHelper->InitializeOperationBand (&band38);
  
  allBwps = CcBwpCreator::GetAllBwps ({band38});
 
  } 

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject <ThreeGppAntennaModel> ()));
  //Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (enbNetDev.Get(0), 0);
  //Ptr<UniformPlanarArray> antenna0 =ConstCast<UniformPlanarArray> (phy0->GetSpectrumPhy ()->GetAntennaArray ());
  //antenna0->SetAttribute ("BearingAngle", DoubleValue (0));

//  Ptr<NetDevice> gNb = enbNetDev.Get(0);
  //Ptr<NrGnbPhy> phy = nrHelper-> GetGnbPhy(gNb,0);
  //Ptr<UniformPlanarArray> antenna = ConstCast <UniformPlanarArray> (phy->GetSpectrumPhy()->GetAntennaArray());
  //antenna ->SetAttribute ("BearingAngle", DoubleValue (0.532));



  //Assign each flow type to a BWP
  //uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVoice = 0;
  uint32_t bwpIdForVideo = 1;
  uint32_t bwpIdForLowLat = 0;


  if (cenario == 1 ) {
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  }
  if (cenario == 2) {
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  }
  if(cenario == 1) {
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  }
  if (cenario == 2) {
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  }


 NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
 NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);



//install nr net devices
  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  if (cenario == 1 ) {
  // Share the total transmission power among CCs proportionally with the BW
   if (tramissao== "DL") {
          x = pow (10, totalTxPower / 10);
         totalBandwidth = numCcs1 * bandwidthcc1;
   }
   else {
          x = pow (10, totalRxPower / 10);
         totalBandwidth = numCcs1 * bandwidthcc1;
    }
  
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp0));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band38.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("RbOverhead", DoubleValue (0.1));

  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band38.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("RbOverhead", DoubleValue (0.1));

  }
  if (cenario==2) {
     if (tramissao=="DL") {
           x = pow (10, totalTxPower / 10);
           totalBandwidth = numCcs2 * bandwidthcc2;
     }
     else {
           x = pow (10, totalRxPower / 10);
           totalBandwidth = numCcs2 * bandwidthcc2;
      }
  
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band38.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("RbOverhead", DoubleValue (0.1));

  }
 
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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);

  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // configure REM parameters
  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
  //fora

  
 if (Rem) {

  remHelper->SetMinX (xMin);
  remHelper->SetMaxX (xMax);
  remHelper->SetResX (xRes);
  remHelper->SetMinY (yMin);
  remHelper->SetMaxY (yMax);
  remHelper->SetResY (yRes);
  remHelper->SetZ (z);
  remHelper->SetSimTag (simTag);
  remHelper->SetRemMode (NrRadioEnvironmentMapHelper::COVERAGE_AREA);
 
  enbNetDev.Get (0)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (0));
  
  remHelper->SetRemMode (NrRadioEnvironmentMapHelper::COVERAGE_AREA);
  remHelper->CreateRem (enbNetDev, ueNetDev.Get (0), remBwpId);
   
 //CoverageArea

 //configure beam that will be shown in REM map
  //DynamicCast<NrGnbNetDevice> (enbNetDev.Get (0))->GetPhy (0)->GetBeamManager ()->SetSector (sector, theta);
  //DynamicCast<NrUeNetDevice> (ueNetDev.Get (0))->GetPhy (0)->GetBeamManager ()->ChangeToQuasiOmniBeamformingVector ();
  //remHelper->CreateRem (ueNetDev, enbNetDev.Get (0), 0);
  
  }

  ApplicationContainer serverApps;

  UdpClientHelper dlClientVideo;
  UdpClientHelper dlClientLowLat;
  UdpClientHelper dlClientVoice;
  UdpClientHelper ulClientVideo;
  UdpClientHelper ulClientVoice;
  UdpClientHelper ulClientLowLat;


  EpsBearer videoBearer (EpsBearer::NGBR_VIDEO_TCP_PREMIUM);
  EpsBearer voiceBearer (EpsBearer::GBR_CONV_VOICE);
  EpsBearer lowLatBearer (EpsBearer::NGBR_LOW_LAT_EMBB);
 
  Ptr<EpcTft> voiceTft = Create<EpcTft> ();
  Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
  Ptr<EpcTft> videoTft = Create<EpcTft> ();


  if (cenario == 1) { 

  	if(tramissao=="DL") { 

    	
 
  	    UdpServerHelper dlPacketSinkVoice (dlPortVoice);
  	    serverApps.Add (dlPacketSinkVoice.Install (ueNodes));

  	    UdpServerHelper dlPacketSinkVideo (dlPortVideo);
    	serverApps.Add (dlPacketSinkVideo.Install (ueNodes));

  	    dlClientVoice.SetAttribute ("RemotePort", UintegerValue (dlPortVoice));
  		dlClientVoice.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  		dlClientVoice.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  		dlClientVoice.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaUll)));

  		EpcTft::PacketFilter dlpfVoice;
        dlpfVoice.localPortStart = dlPortVoice;
        dlpfVoice.localPortEnd = dlPortVoice;
        dlpfVoice.direction = EpcTft::DOWNLINK;
        voiceTft->Add (dlpfVoice);
  
    	dlClientVideo.SetAttribute ("RemotePort", UintegerValue (dlPortVideo));
  		dlClientVideo.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  		dlClientVideo.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeUll));
  		dlClientVideo.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaUll))); 
    	
  		EpcTft::PacketFilter dlpfVideo;
  		dlpfVideo.localPortStart = dlPortVideo;
  		dlpfVideo.localPortEnd = dlPortVideo;
  		dlpfVideo.direction = EpcTft::DOWNLINK;
  		videoTft->Add (dlpfVideo);
    }
    else {

      //if ( intervalo == fixo )
      //{
        

     	  UdpServerHelper ulPacketSinkVoice (ulPortVoice);
     	  serverApps.Add (ulPacketSinkVoice.Install (remoteHost));

     	  UdpServerHelper ulPacketSinkVideo (ulPortVideo);
     	  serverApps.Add (ulPacketSinkVideo.Install (remoteHost));

	 	    ulClientVoice.SetAttribute ("RemotePort", UintegerValue (ulPortVoice));
    	  ulClientVoice.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
    	  ulClientVoice.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
    	  ulClientVoice.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));

    	  EpcTft::PacketFilter ulpfVoice;
    	  ulpfVoice.localPortStart = ulPortVoice;
    	  ulpfVoice.localPortEnd = ulPortVoice;
   	    ulpfVoice.direction = EpcTft::UPLINK;
    	  voiceTft->Add (ulpfVoice);
    	
	 	    ulClientVideo.SetAttribute ("RemotePort", UintegerValue (ulPortVideo));
    	  ulClientVideo.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
    	  ulClientVideo.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
    	  ulClientVideo.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));
 
    	   EpcTft::PacketFilter ulpfVideo;
    	   ulpfVideo.localPortStart = ulPortVideo;
    	   ulpfVideo.localPortEnd = ulPortVideo;
   	     ulpfVideo.direction = EpcTft::UPLINK;
    	   videoTft->Add (ulpfVideo);       
       /*
       else  {

        uint16_t ulPortVoice = 1234;
        uint16_t ulPortVideo = 1236;

        PacketSinkHelper ulPacketSinkHelper ("ns3 :: UdpSocketFactory", InetSocketAddress (ulPortVoice));
        serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

        OnOffHelper onoffUl ("ns3 :: UdpSocketFactory", InetSocketAddress (remoteHost, ulPortVoice));
        onoffUl.SetAttribute ("OnTime", StringValue ("ns3 :: ConstantRandomVariable [Constant = 0,001]"));
        onoffUl.SetAttribute ("OffTime", StringValue ("ns3 :: ExponentialRandomVariable [Mean =" + Lambda + "]"));
        onoffUl.SetAttribute ("PacketSize", UintegerValue (PacketSize));
        onoffUl.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
        onoffUl.SetAttribute ("StopTime", TimeValue (Seconds (simTime)));
        clientApps.Add (onoffUl.Install (gridScenario.GetUserTerminals (). Get (u)));
        
        Ptr <EpcTft> tft = Criar <EpcTft> ();
        EpcTft :: PacketFilter ulpf;
        ulpf.remotePortStart = ulPort;
        ulpf.remotePortEnd = ulPort;
        ++ ulPort;
        tft-> Adicionar (ulpf);

       }
    	*/
    } 
	
  } 
  if (cenario==2) { 

  	
  	if ( tramissao == "DL")
  	  { 
  	  
        UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);

        
    	serverApps.Add (dlPacketSinkLowLat.Install (ueNodes));

    	dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
   		dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
    	dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
    	dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));
  // The filter for the low-latency traffic
        EpcTft::PacketFilter dlpfLowLat;
        dlpfLowLat.localPortStart = dlPortLowLat;
        dlpfLowLat.localPortEnd = dlPortLowLat;
        lowLatTft->Add (dlpfLowLat);


      }
	else{

        UdpServerHelper ulPacketSinkLowLat (ulPortLowLat);

        serverApps.Add (ulPacketSinkLowLat.Install (remoteHost));

 		ulClientLowLat.SetAttribute ("RemotePort", UintegerValue (ulPortLowLat));
   		ulClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
    	ulClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
    	ulClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));
  // The filter for the low-latency traffic
        EpcTft::PacketFilter ulpfLowLat;
        ulpfLowLat.localPortStart = ulPortLowLat;
        ulpfLowLat.localPortEnd = ulPortLowLat;
        ulpfLowLat.direction = EpcTft::UPLINK;
        lowLatTft->Add (ulpfLowLat);


 	}

  }

ApplicationContainer clientApps;


if (cenario == 1 ) { 

     for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Node> ue = ueNodes.Get (i);
      Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
      Address ueAddress = ueIpIface.GetAddress (i);
      if(tramissao=="DL") {

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
           		 	ulClientVideo.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
            		clientApps.Add (ulClientVideo.Install (ue));

            		nrHelper->ActivateDedicatedEpsBearer (ueDevice, videoBearer, videoTft);
          		}
        	if (enableVoice)
         	 	{
            	    ulClientVoice.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
            	    clientApps.Add (ulClientVoice.Install (ue));

           		    nrHelper->ActivateDedicatedEpsBearer (ueDevice, voiceBearer, voiceTft);
        		}	
     }
   
    }
  }

if (cenario==2){

  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Node> ue = ueNodes.Get (i);
      Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
      Address ueAddress = ueIpIface.GetAddress (i);
      if (tramissao=="DL") {

        if (enableLowLat)
         {
            dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
            clientApps.Add (dlClientLowLat.Install (remoteHost));

            nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
         } 
       }
       else {

			ulClientLowLat.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
            clientApps.Add (ulClientLowLat.Install (ue));

            nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
       }



     }

  }

  // start UDP server and client apps
  serverApps.Start (Seconds (udpAppStartTime));
  clientApps.Start (Seconds (udpAppStartTime));
  serverApps.Stop (Seconds (simTime));
  clientApps.Stop (Seconds (simTime));

  // enable the traces provided by the nr module
  nrHelper->EnableTraces ();

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

         if ( (( t.destinationPort == dlPortLowLat) || (t.destinationPort == dlPortVoice) || 
         	(t.destinationPort == dlPortVideo) ) && (t.sourceAddress == internetIpIfaces.GetAddress(1)) )
           {
             streamMetricsInit = DLstreamMetricsInit;
           }
         // Get file pointer for UL, if UL flow (using port and IP address to assure correct result))
         else if ( (( t.destinationPort == ulPortLowLat) || (t.destinationPort == ulPortVoice) || 
         	(t.destinationPort == ulPortVideo) ) && (t.destinationAddress == t.destinationAddress))          
           {
             streamMetricsInit = ULstreamMetricsInit;
           }
         //
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
                 std::cout << "DLport: " << dlPortLowLat << std::endl;
                 std::cout << "ULport: " << ulPortLowLat  << std::endl;
               }
           }
        //  bytesTotal =+ iter->second.rxPackets;
  }

flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);

/////////////
Simulator::Destroy ();
  Simulator::Destroy ();
  return 0;
}


