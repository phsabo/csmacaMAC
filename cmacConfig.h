


//cmac config
//#define DEBUG

#define BAUDRATE     230400

#define CEPIN     7
#define CSPIN     8

#define DEFAULT_CHANNEL   32
#define DATARATE     RF24_250KBPS  //RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS        
#define PALEVEL       RF24_PA_LOW //RF24_PA_MIN,RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX,
#define AUTOACK     false
#define AUTOCHANNEL   false
#define NUM_CHANNELS     126
#define PACKET_SIZE     128//PAYLOAD


#define PIPE_TX       0xE8E8F0F0E1LL
#define PIPE_RX       0xE8E8F0F0D2LL

//endereços 
//#define COORDINATOR_ADDR   0X00
#define COORDINATOR_ADDR   0X00
#define BROADCAST_ADDR   0XFF
#define MY_ADDR   0X00


//coordinator mac 
#define TIMETORECONNECT 60000 // TEMPO MAXIMO DE ESPERA DO TOKEN

#define MAXTIMEWAITINGPACKET 2700//us
#define TIMENEWCONNECTIONS 5000//us
#define QTDELOOPS 100
#define PIFS 100// microseconds



//csma mac
#define SLOTTIME 52 //http://www.sciencedirect.com/science/article/pii/S2405959516300650

#define SIFS 160// microseconds
#define DIFS 264
#define BACKOFF 320
#define CCA 168
#define SUPERFRAMETIME 3000

#define SLEEPTIMEMIN 399//MILISEGUNDOS
#define SLEEPTIMEMAX 400//MILISEGUNDOS



//msgs

#define ACKMSG   0X01
#define NEWCONNECTIONSMSG   0X02

#define TOKENMSG   0X03
#define RTSMSG   0X04
#define CTSMSG   0X05
#define DATAMSG 0X06
#define OUTMSG 0X07


byte waitingConnections[3]={BROADCAST_ADDR,MY_ADDR,NEWCONNECTIONSMSG};
byte rts[3]={COORDINATOR_ADDR,MY_ADDR,RTSMSG};



typedef enum { wdt_16ms = 0, wdt_32ms, wdt_64ms, wdt_128ms, wdt_250ms, wdt_500ms, wdt_1s, wdt_2s, wdt_4s, wdt_8s } wdt_prescalar_e;





