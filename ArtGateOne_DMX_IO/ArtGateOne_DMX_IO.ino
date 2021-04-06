/*
  ArtGateOne DMX IN/OUT v1.0
*/

#include <lib_dmx.h>  // comment/uncomment #define USE_UARTx in lib_dmx.h as needed
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define    DMX512     (0)    // (250 kbaud - 2 to 512 channels) Standard USITT DMX-512


int dmx_mode = 0; //1 = artnet-->dmx, 0 = dmx -->artnet
int dhcp = 1; // 1 = dhcp , 0 = static ip
byte intN = 0; //NET
byte intS = 0; //Subnet
byte intU = 8; //Universe

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(2, 0, 0, 10);
IPAddress dns(0, 0, 0, 0);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(255, 0, 0, 0);




unsigned int localPort = 6454;  // local port to listen on
unsigned int intUniverse = ((intS * 16) + intU);
unsigned char packetBuffer[18]; // buffer to hold incoming packet,
unsigned long myTime;
byte ArtPoolReply[239];
byte ArtDMX[530];

EthernetUDP Udp;

void setup()
{
  Ethernet.init(10);

  if (dhcp == 1) {
    Ethernet.begin(mac);  //USE DHCP
  } else {
    Ethernet.begin(mac, ip, dns, gateway, subnet); //USE STATIC IP SETTINGS}
  }
  Udp.begin(localPort);


  ArduinoDmx0.set_control_pin(2);  // Arduino output pin for MAX485 input/output control (connect to MAX485-1 pins 2-3)

  if (dmx_mode == 1) {//DMX MODE OUT ?
    ArduinoDmx0.set_tx_address(1);    // set rx1 start address
    ArduinoDmx0.set_tx_channels(512); // 2 to 2048!! channels in DMX1000K (512 in standard mode) See lib_dmx.h  *** new *** EXPERIMENTAL
    ArduinoDmx0.init_tx(DMX512);      // starts universe 1 as rx, standard DMX 512 - See lib_dmx.h, now support for DMX faster modes (DMX 1000K)
  } else {//DMX MODE INPUT
    ArduinoDmx0.set_rx_address(1);    // set rx1 start address
    ArduinoDmx0.set_rx_channels(512); // 2 to 2048!! channels in DMX1000K (512 in standard mode) See lib_dmx.h  *** new *** EXPERIMENTAL
    ArduinoDmx0.init_rx(DMX512);
  }

  makeArtPoolReply();
  makeArtDMX();

  myTime = millis();
}//end setup()

void loop()
{
  if (dmx_mode == 0) {
    if (millis() >= myTime + 33) {
      myTime = millis();
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());// send a ArtDMX to the IP address and port that sent us the packet we received
      //Udp.beginPacket(0xFFFFFFFF, 0x1936);// send ArtDMX to 255.255.255.255:6454

      int d = 18;
      for (int i = 0; i < 512; i++) {
        ArtDMX[d] = ArduinoDmx0.RxBuffer[i];
        d++;
      }

      Udp.write(ArtDMX, 530);
      Udp.endPacket();
    }
  }

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize == 14) {
    // send a ArtPoolReply to the IP address and port that sent us the packet we received
    //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());  //Unicast
    Udp.beginPacket(0xFFFFFFFF, Udp.remotePort());  //Broadcast
    Udp.write(ArtPoolReply, 239);    //  bufLen = 239;
    Udp.endPacket();
  }
  if (packetSize == 530 && dmx_mode == 1) {

    // read the packet into packetBufffer
    Udp.read(packetBuffer, 18);

    if (packetBuffer[15] == intN && packetBuffer[14] == intUniverse) { //check artnet net & universe (sub/uni)
      for (int i = 0; i <= 511; i++) {
        Udp.read(packetBuffer, 1);
        ArduinoDmx0.TxBuffer[i] = packetBuffer[0];
      }
    }
  }
}//end loop()


void makeArtDMX() {
  ArtDMX[0] = byte('A'); // A
  ArtDMX[1] = byte('r'); // r
  ArtDMX[2] = byte('t'); // t
  ArtDMX[3] = byte('-'); // -
  ArtDMX[4] = byte('N'); // N
  ArtDMX[5] = byte('e'); // e
  ArtDMX[6] = byte('t'); // t
  ArtDMX[7] = 0x00;      // 0x00

  ArtDMX[8] = 0x00;      // OpCode[0]
  ArtDMX[9] = 0x50;      // OpCode[1]

  ArtDMX[10] = 0x00; // ProtVerHi
  ArtDMX[11] = 0x14; // ProtVerLo
  ArtDMX[12] = 0x00; // Sequence
  ArtDMX[13] = 0x00; // Phisical
  ArtDMX[14] = intUniverse; // SubUni
  ArtDMX[15] = intN; // Net
  ArtDMX[16] = 0x02; // LenghtHi
  ArtDMX[17] = 0x00; // Lenght
  return;
}

void makeArtPoolReply() {
  ArtPoolReply[0] = byte('A');  // A
  ArtPoolReply[1] = byte('r');  // r
  ArtPoolReply[2] = byte('t');  // t
  ArtPoolReply[3] = byte('-');  // -
  ArtPoolReply[4] = byte('N');  // N
  ArtPoolReply[5] = byte('e');  // e
  ArtPoolReply[6] = byte('t');  // t
  ArtPoolReply[7] = 0x00;       // 0x00

  ArtPoolReply[8] = 0x00; // OpCode[0]
  ArtPoolReply[9] = 0x21; // OpCode[1]

  ArtPoolReply[10] = Ethernet.localIP()[0]; // IPV4 [0]
  ArtPoolReply[11] = Ethernet.localIP()[1]; // IPV4 [1]
  ArtPoolReply[12] = Ethernet.localIP()[2]; // IPV4 [2]
  ArtPoolReply[13] = Ethernet.localIP()[3]; // IPV4 [3]

  ArtPoolReply[14] = 0x36; // IP Port Low
  ArtPoolReply[15] = 0x19; // IP Port Hi

  ArtPoolReply[16] = 0x01; // High byte of Version
  ArtPoolReply[17] = 0x00; // Low byte of Version

  ArtPoolReply[18] = intN; // NetSwitch
  ArtPoolReply[19] = intS; // Net Sub Switch
  ArtPoolReply[20] = 0x00; // OEMHi
  ArtPoolReply[21] = 0xFF; // OEMLow
  ArtPoolReply[22] = 0x00; // Ubea Version
  ArtPoolReply[23] = 0xF0; // Status1
  ArtPoolReply[24] = 0x00; // ESTA LO 0x41; //
  ArtPoolReply[25] = 0x00; // ESTA HI  0x4D; //

  ArtPoolReply[26] = byte('A');  // A  //Short Name
  ArtPoolReply[27] = byte('r');  // r
  ArtPoolReply[28] = byte('t');  // t
  ArtPoolReply[29] = byte('G');  // G
  ArtPoolReply[30] = byte('a');  // a
  ArtPoolReply[31] = byte('t');  // t
  ArtPoolReply[32] = byte('e');  // e
  ArtPoolReply[33] = byte('O');  // O
  ArtPoolReply[34] = byte('n');  // n
  ArtPoolReply[35] = byte('e');  // e

  for (int i = 36; i <= 43; i++) {// Short Name
    ArtPoolReply[i] = 0x00;
  }

  ArtPoolReply[44] = byte('A');  // A  //Long Name
  ArtPoolReply[45] = byte('r');  // r
  ArtPoolReply[46] = byte('t');  // t
  ArtPoolReply[47] = byte('G');  // G
  ArtPoolReply[48] = byte('a');  // a
  ArtPoolReply[49] = byte('t');  // t
  ArtPoolReply[50] = byte('e');  // e
  ArtPoolReply[51] = byte('O');  // O
  ArtPoolReply[52] = byte('n');  // n
  ArtPoolReply[53] = byte('e');  // e
  ArtPoolReply[54] = byte(' ');  //
  ArtPoolReply[55] = byte('D');  // D
  ArtPoolReply[56] = byte('M');  // M
  ArtPoolReply[57] = byte('X');  // X
  ArtPoolReply[58] = byte(' ');  //
  ArtPoolReply[59] = byte('I');  // I
  ArtPoolReply[60] = byte('O');  // O

  for (int i = 61; i <= 107; i++) { //Long Name
    ArtPoolReply[i] = 0x00;
  }

  for (int i = 108; i <= 171; i++) {  //NodeReport
    ArtPoolReply[i] = 0x00;
  }

  ArtPoolReply[172] = 0x00; // NumPorts Hi
  ArtPoolReply[173] = 0x01; // NumPorts Lo
  ArtPoolReply[174] = 0x00; // Port 0 Type
  ArtPoolReply[175] = 0x00; // Port 1 Type
  ArtPoolReply[176] = 0x00; // Port 2 Type
  ArtPoolReply[177] = 0x00; // Port 3 Type
  ArtPoolReply[178] = 0x00; // GoodInput 0
  ArtPoolReply[179] = 0x00; // GoodInput 1
  ArtPoolReply[180] = 0x00; // GoodInput 2
  ArtPoolReply[181] = 0x00; // GoodInput 3
  ArtPoolReply[182] = 0x00; // GoodOutput 0
  ArtPoolReply[183] = 0x00; // GoodOutput 1
  ArtPoolReply[184] = 0x00; // GoodOutput 2
  ArtPoolReply[185] = 0x00; // GoodOutput 3
  ArtPoolReply[186] = 0x00; // SwIn 0
  ArtPoolReply[187] = 0x00; // SwIn 1
  ArtPoolReply[188] = 0x00; // SwIn 2
  ArtPoolReply[189] = 0x00; // SwIn 3
  ArtPoolReply[190] = 0x00; // SwOut 0
  ArtPoolReply[191] = 0x00; // SwOut 1
  ArtPoolReply[192] = 0x00; // SwOut 2
  ArtPoolReply[193] = 0x00; // SwOut 3
  ArtPoolReply[194] = 0x01; // SwVideo
  ArtPoolReply[195] = 0x00; // SwMacro
  ArtPoolReply[196] = 0x00; // SwRemote
  ArtPoolReply[197] = 0x00; // Spare
  ArtPoolReply[198] = 0x00; // Spare
  ArtPoolReply[199] = 0x00; // Spare
  ArtPoolReply[200] = 0x00; // Style

  if (dmx_mode == 1) {
    ArtPoolReply[174] = 0x80; // Port 0 Type
    ArtPoolReply[190] = intU; // SwOut 0
    ArtPoolReply[182] = 0x80; // GoodOutput 0
  } else {
    ArtPoolReply[174] = 0x40; // Port 0 Type
    ArtPoolReply[186] = intU; // SwIn 0
    ArtPoolReply[178] = 0x80; // GoodInput 0
  }



  // MAC ADDRESS
  ArtPoolReply[201] = mac[0]; // MAC HI
  ArtPoolReply[202] = mac[1]; // MAC
  ArtPoolReply[203] = mac[2]; // MAC
  ArtPoolReply[204] = mac[3]; // MAC
  ArtPoolReply[205] = mac[4]; // MAC
  ArtPoolReply[206] = mac[5]; // MAC LO

  ArtPoolReply[207] = 0x00; // BIND IP 0
  ArtPoolReply[208] = 0x00; // BIND IP 1
  ArtPoolReply[209] = 0x00; // BIND IP 2
  ArtPoolReply[210] = 0x00; // BIND IP 3
  ArtPoolReply[211] = 0x00; // BInd Index

  if(dhcp == 1){
    ArtPoolReply[212] = 0x06; // Status2
  }else{
    ArtPoolReply[212] = 0x04; // Status2
    }

  return;
}
