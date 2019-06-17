#include "nRF24L01.h"
#include "RF24.h"
#include <SPI.h>
#include "printf.h"
#include "cmacConfig.h"
#include <avr/sleep.h>
#include <avr/power.h>

const uint8_t MYADDR = 0x01;

char Packet[PACKET_SIZE];

bool conectado = false;

RF24 radio(CEPIN, CSPIN);

void setup(void) {
  Serial.begin(BAUDRATE);
  printf_begin();

  Serial.println("teste receptor");

  radio.begin();
  radio.setPALevel(PALEVEL);
  radio.enableDynamicPayloads();
  //radio.setPayloadSize(PACKET_SIZE);
  radio.setDataRate(DATARATE);
  radio.setAutoAck(AUTOACK);
  radio.setChannel(DEFAULT_CHANNEL);
  radio.openWritingPipe(PIPE_TX);
  radio.openReadingPipe(1, PIPE_RX);
  radio.startListening();
  radio.stopListening();
  radio.printDetails();

  radio.startListening();
}

void wakeUp() {
  sleep_disable();
}

// Sleep helpers

//Prescaler values
// 0=16ms, 1=32ms,2=64ms,3=125ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec


bool tryConnect() {
  // Now, resume listening so we catch the next packets.

  if ( radio.available() ) {                                  // if there is data ready
    byte msg[3];
    bool waitingResponse = false;
    while (radio.available()) {                             // Dump the payloads until we've gotten everything
      radio.read( &msg, 3 );       // Get the payload, and see if this was the last one.
      // Spew it.  Include our time, because the ping_out millis counter is unreliable
      printf("%0x %0x %0x\n\r", msg[0], msg[1], msg[2]);
      if (msg[0] == BROADCAST_ADDR & msg[1] == COORDINATOR_ADDR & msg[2] == NEWCONNECTIONSMSG) {
        Serial.println("try connect");
        msg[0] = COORDINATOR_ADDR;
        msg[1] = MY_ADDR;
        radio.stopListening();
        radio.write(&msg, 3);
        waitingResponse = true;
        radio.startListening();
        //printf("Got payload %lu @ %lu...\n",got_time,millis()); // due to it sleeping
      }

    }

    unsigned long started_waiting_at = millis();    // Wait here until we get a response, or timeout (250ms)
    bool timeout = false;

    while ((!timeout) & waitingResponse)
    {
      while ( ! radio.available()  ) {
        if (millis() - started_waiting_at > 200 ) { // Break out of the while loop if nothing available
          timeout = true;
          break;
        }
      }
      if ( timeout ) {                                // Describe the results
        Serial.println("failed connect");
        return false;
      } else {
        byte msg[3];                     // Grab the response, compare, and send to debugging spew
        radio.read( &msg, 3 );
        if (msg[0] == MY_ADDR & msg[1] == COORDINATOR_ADDR & msg[2] == ACKMSG) {

          Serial.println("connected");
          return true;
        }
      }
    }
  }
  return false;
}
void conectar() {
  radio.powerUp();
  radio.startListening();
  while (! conectado) {
    Serial.println("Tentando conectar");
    //teste();
    bool conectando = tryConnect();
    if (conectando) {
      conectado = true;
      Serial.println("CONECTADO");
    }
  }
  radio.powerDown();
}
bool packMsg(String msg, uint8_t addr, uint32_t tempo)
{
  if (msg.length() > PACKET_SIZE - 6) {
    Serial.println("ERRO msg muito grande");
    return false;
  }
  for (int i = 0; i < PACKET_SIZE; i++)
  {
    Packet[i] = ' ';
  }
  for (int i = 0; i < (msg.length()); i++)
  {
    Packet[i + 6] = msg.charAt(i);
  }
  Packet[0] = char(addr);
  Packet[1] = char(MY_ADDR);
  if (tempo == 0) {
    Packet[2] = 0;
    Packet[3] = 0;
    Packet[4] = 0;
    Packet[5] = 0;
  } else {
    //uint32_t num32 = 0xabcdefgh;
    //uint8_t a = num32;       /* 0xgh */
    //uint8_t b = num32 >> 8;  /* 0xef */
    //uint8_t c = num32 >> 16; /* 0xcd */
    //uint8_t d = num32 >> 24; /* 0xab */
    Packet[2] = tempo >> 24;
    Packet[3] = tempo >> 16;
    Packet[4] = tempo >> 8;
    Packet[5] = tempo ;

  }
  return true;

}
int CalcBackoff(int mult, int tempo){
  if (mult>7){
    return(random(1,8)*tempo);
  }else{
    return(mult*tempo);
  }
}

bool sendMsg(String mensagem) {
  while (!conectado) {
    Serial.println("Não conectado, conectando");
    conectar();
  }
  if (conectado) {
    radio.powerUp();
    radio.startListening();                         // Now, continue listening
    unsigned long started_waiting_at = millis();    // Wait here until we get a response, or timeout (250ms)
    bool timeout = false;
    int mult=1;
    byte msg[3];
    msg[0] = msg[1];
    msg[1] = MY_ADDR;
    msg[2] = RTSMSG;
    radio.stopListening();
    radio.flush_rx();
    radio.startListening();
    delayMicroseconds(SIFS);
    radio.stopListening();
    while ( radio.available()) {
      radio.flush_rx();
      radio.startListening();
      delayMicroseconds(CalcBackoff(mult,SIFS));//backoff exponencial
      radio.stopListening();
    }

    radio.write( &msg, 3 );
    radio.startListening();
    timeout = false;
    unsigned long started_send_at = micros();
    while (!timeout)
    {
      while ( ! radio.available()  ) {
        if (micros() - started_send_at > 15000 ) { // Break out of the while loop if nothing available
          timeout = true;
          break;
        }
      }
      if ( timeout ) {                                // Describe the results
        timeout = false;
        radio.flush_rx();
        break;
      } else {
        radio.read( &msg, 3 );
        if (msg[0] == MY_ADDR & msg[2] == CTSMSG) {
          //printf("CTS recebido \n\r");
          radio.stopListening();
          //printf("enviando Packet \n\r");
          if (!packMsg(mensagem, COORDINATOR_ADDR, (micros() - started_waiting_at))) {
            //Serial.println("ERRO PACOTE INVÁLIDO");
            return false;
          } else {
            radio.stopListening();
            radio.flush_rx();
            radio.startListening();
            delayMicroseconds(SIFS);
            radio.stopListening();
            while ( radio.available()) {
              radio.flush_rx();
              radio.startListening();
              delayMicroseconds(SIFS);
              radio.stopListening();
            }
            radio.write( &Packet, PACKET_SIZE );
            radio.startListening(); msg[0] = msg[1];
          }
          while (!timeout)
          {
            while ( ! radio.available()  ) {
              if (micros() - started_send_at > 15000 ) { // Break out of the while loop if nothing available
                timeout = true;
                break;
              }
            }
            if ( timeout ) {                                // Describe the results
              //Serial.println("ack não recebido");
              timeout = false; //Confirmação não recebida
              radio.flush_rx();
              break;
            } else {
              radio.read( &msg, 3 );
              if (msg[0] == MY_ADDR & msg[2] == ACKMSG) {
                return true;
              } else {
                radio.flush_rx();
              }
            }
          }
        } else {
          radio.flush_rx();
        }
      }
    }
  }
}
void loop(void) {
  while (!conectado) {
    Serial.println("Não conectado, conectando");
    conectar();
  }
  randomSeed(millis());
  delay(random(10, 5000));
  while (! sendMsg("MENSAGEM DO NÓ1234567890")) {
    Serial.println("falhou em enviar");
  }


}
