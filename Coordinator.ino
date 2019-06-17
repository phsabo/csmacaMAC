

/*
  Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

  This program is free software; you can rediPacketibute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.
*/

/**
   Channel scanner

   Example to detect interference on the various channels available.
   This is a good diagnostic tool to check whether you're picking a
   good channel for your application.

   Inspired by cpixip.
   See http://arduino.cc/forum/index.php/topic,54795.0.html
*/



#include <SPI.h>

#include "nRF24L01.h"
#include "RF24.h"

#include "printf.h"
#include <stdlib.h>
#include "cmacConfig.h"
#include <avr/sleep.h>
#include <avr/power.h>

const short sleep_cycles_per_transmission = 4;
volatile short sleep_cycles_remaining = sleep_cycles_per_transmission;

uint8_t list_nodes[int(pow(2, (sizeof(uint8_t) * 8)))];
char Packet[PACKET_SIZE];
uint8_t qtde_nodes = 0;

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 7 & 8

RF24 radio(CEPIN, CSPIN);

//
// Channel info
//


uint8_t values[NUM_CHANNELS];

//
// Setup
//

int findChannel()
{
  const int num_reps = 100;
  String canais = "";

  // Clear measurement values
  memset(values, 0, sizeof(values));

  // Scan all channels num_reps times
  int rep_counter = num_reps;
  while (rep_counter--)
  {
    int i = NUM_CHANNELS;
    while (i--)
    {
      // Select this channel
      radio.setChannel(i);

      // Listen for a little
      radio.startListening();
      delayMicroseconds(128);
      radio.stopListening();

      // Did we get a carrier?
      if ( radio.testCarrier() ) {
        ++values[i];
      }
    }
  }

  // Print out channel measurements, clamped to a single hex digit
  int i = 0;
  while ( i < NUM_CHANNELS )
  {
    printf("%x", min(0xf, values[i]));
    //Serial.print(min(0xf,values[i]));
    canais += min(0xf, values[i]);
    i++;
  }
  Serial.println();

  //Serial.println(canais);
  int channel = canais.indexOf("0000000000000000000000000000000000000000000000000000000000000000");
  //Serial.println(channel);
  if (channel >= 0) {
    //radio.setChannel(channel+32);
    return (channel + 32);
    Serial.println("utilizando canal " + String(channel + 32));
  } else if ((channel = canais.indexOf("00000000000000000000000000000000")) >= 0) {
    //  Serial.println(channel);

    //radio.setChannel(channel+16);
    return (channel + 16);
    Serial.println("utilizando canal " + String(channel + 16));
  } else if ((channel = canais.indexOf("0000000000000000")) >= 0) {
    //  Serial.println(channel);

    //radio.setChannel(channel+8);
    return (channel + 8);
    Serial.println("utilizando canal " + String(channel + 8));
  } else if ((channel = canais.indexOf("00000000")) >= 0) {
    //  Serial.println(channel);

    //radio.setChannel(channel+4);
    return (channel + 4);
    Serial.println("utilizando canal " + String(channel + 4));
  } else if ((channel = canais.indexOf("0000")) >= 0) {
    //  Serial.println(channel);

    //radio.setChannel(channel+2);
    return (channel + 2);
    Serial.println("utilizando canal " + String(channel + 2));
  } else if ((channel = canais.indexOf("00")) >= 0) {
    //  Serial.println(channel);

    //radio.setChannel(channel+1);
    return (channel + 1);
    Serial.println("utilizando canal " + String(channel + 1));
  } else {
    return -1;
  }

}

void startRadio()
{
  radio.setPALevel(PALEVEL);
  radio.openReadingPipe(1, PIPE_TX);//node tx
  radio.openWritingPipe(PIPE_RX);//node rx
  radio.setDataRate(DATARATE);

  radio.setAutoAck(AUTOACK);
  radio.setChannel(DEFAULT_CHANNEL);
  radio.enableDynamicPayloads();
  //radio.setPayloadSize(PACKET_SIZE);
  radio.powerUp();              // NOTE: The radio MUST be powered back up again manually


}

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(BAUDRATE);
  printf_begin();
  //Serial.println(F("\n\rRF24/examples/scanner/"));

  //
  // Setup and configure rf radio
  //

  radio.begin();
  startRadio();
  //  setup_watchdog(wdt_128ms);


  //Serial.println("=============================== SELECTING CHANNEL =============================");
  /*Serial.println("Procurando Canal Livre");
    int canal = findChannel();
    if (canal>=0){

    radio.setChannel(canal);
    Serial.println("Melhor canal "+String(canal));
    }else{
    Serial.println("erro - banda ocupada selecione canal Manualmente");
    radio.setChannel(32);
    }*/


  // Get into standby mode
  radio.startListening();
  delay(100);
  radio.stopListening();
#ifdef DEBUG
  radio.printDetails();
#endif


}

//
// Loop
//

void packMsg(String msg, uint8_t addr)
{
  for (int i = 0; i < PACKET_SIZE; i++)
  {
    Packet[i] = ' ';
  }
  for (int i = 0; i < (msg.length() - 2); i++)
  {
    Packet[i + 2] = msg.charAt(i);
  }
  Packet[0] = char(addr);
  Packet[0] = char(MY_ADDR);

}

uint32_t u8tou32(uint8_t b[4]) {
  uint32_t u;
  u = b[0];
  u = (u  << 8) + b[1];
  u = (u  << 8) + b[2];
  u = (u  << 8) + b[3];
  return u;
}
uint32_t u8tou16(uint8_t b[2]) {
  uint32_t u;
  u = b[0];
  u = (u  << 8) + b[1];
  return u;
}
void teste()
{
  // Power up the radio after sleeping
  radio.stopListening();                          // First, stop listening so we can talk.

  unsigned long time = millis();                  // Take the time, and send it.
  Serial.print(F("Now sending... "));
  Serial.println(time);

  radio.write( &time, sizeof(unsigned long) );

  radio.startListening();                         // Now, continue listening

  unsigned long started_waiting_at = millis();    // Wait here until we get a response, or timeout (250ms)
  bool timeout = false;
  while ( ! radio.available()  ) {
    if (millis() - started_waiting_at > 250 ) { // Break out of the while loop if nothing available
      timeout = true;
      break;
    }
  }

  if ( timeout ) {                                // Describe the results
    Serial.println(F("Failed, response timed out."));
  } else {
    unsigned long got_time;                     // Grab the response, compare, and send to debugging spew
    radio.read( &got_time, sizeof(unsigned long) );

    printf("Got response %lu, round-trip delay: %lu\n\r", got_time, millis() - got_time);
  }


}
void printList() {
  Serial.println("Nodes list");
  for (int i = 0; i < qtde_nodes; i++) {
    printf("%0x \n\r", list_nodes[i]);

  }
}
void addNodeToList(uint8_t node) {
  bool naLista = false;
  for (int i = 0; i < qtde_nodes; i++) {
    if ( list_nodes[i] == node) {
      naLista = true;

    }
  }
  if (! naLista) {
    list_nodes[qtde_nodes] = node;
    qtde_nodes += 1;
  }
}
void newConnections()
{
  radio.stopListening();
  byte msg[3] = {BROADCAST_ADDR, MY_ADDR, NEWCONNECTIONSMSG};
  //printf("%0x %0x %0x\n\r", msg[0], msg[1], msg[2]);
  radio.write( &msg, 3 );
  radio.startListening();                         // Now, continue listening
  unsigned long started_waiting_at = millis();    // Wait here until we get a response, or timeout (250ms)
  bool timeout = false;
  while (! timeout)
  {
    while ( ! radio.available()  ) {
      if (millis() - started_waiting_at > TIMENEWCONNECTIONS ) { // Break out of the while loop if nothing available
        timeout = true;
        break;
      }
    }
    if ( timeout ) {                                // Describe the results
#ifdef DEBUG
      Serial.println("Sem novas conecções");
#endif

    } else {
      // Grab the response, compare, and send to debugging spew
      radio.read( &msg, 3 );
      if (msg[0] == MY_ADDR & msg[2] == NEWCONNECTIONSMSG) {
#ifdef DEBUG
        printf("novo nó: %d\n\r", int(msg[1]));
#endif

        addNodeToList(msg[1]);
        msg[0] = msg[1];
        msg[1] = MY_ADDR;
        msg[2] = ACKMSG;
        radio.stopListening();
        radio.write( &msg, 3 );
        radio.startListening();
      }

    }
  }
}
void sendToken(uint8_t dest)
{
  radio.stopListening();
  byte msg[3] = {dest, MY_ADDR, TOKENMSG};
#ifdef DEBUG
  printf("%0x %0x %0x\n\r", msg[0], msg[1], msg[2]);
#endif

  radio.write( &msg, 3 );
  radio.startListening();                         // Now, continue listening
  unsigned long started_waiting_at = millis();    // Wait here until we get a response, or timeout (250ms)
  bool timeout = false;
  while (! timeout)
  {
    while ( ! radio.available()  ) {
      if (millis() - started_waiting_at > MAXTIMEWAITINGPACKET ) { // Break out of the while loop if nothing available
        timeout = true;
        break;
      }
    }
    if ( timeout ) {                                // Describe the results
#ifdef DEBUG
      printf("sem resposta de nó %0x", dest);
#endif
    } else {
      // Grab the response, compare, and send to debugging spew
      radio.read( &Packet, PACKET_SIZE );
      printPacket();
      msg[0] = dest;
      msg[1] = MY_ADDR;
      msg[2] = ACKMSG;
      radio.stopListening();
      radio.write( &msg, 3 );
      radio.startListening();


    }
  }
}
void waitPackets()
{

  radio.startListening();                         // Now, continue listening
  //unsigned long started_waiting_at = millis();    // Wait here until we get a response, or timeout (250ms)
  bool timeout = false;


  while (radio.available())
  {
    byte msg[3];
    radio.read( &msg, 3 );
    if (msg[0] == MY_ADDR & msg[2] == RTSMSG) {
      //printf("RTS recebido \n\r");
      radio.stopListening();
      //printf("enviando CTS \n\r");
      msg[0] = msg[1];
      msg[1] = MY_ADDR;
      msg[2] = CTSMSG;
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
          //Serial.println("payload não recebido");
          timeout = false; //Confirmação não recebida

          radio.flush_rx();
          break;
          //return false;

        } else {
          radio.read( &Packet, PACKET_SIZE );
          if (Packet[0] == MY_ADDR & Packet[8] == 'M') {
            printPacket();
            byte msg[3];
            msg[0] = Packet[1];
            msg[1] = MY_ADDR;
            msg[2] = ACKMSG;
            radio.stopListening();
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
            radio.write( &msg, 3 );
            radio.flush_rx();
            radio.startListening();
            return;

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








void printPacket() {
  //printf("\n\r ============== pacote recebido ==============\n\r");
  //printf("origem:%0x destino:%0x ", Packet[1], Packet[0]);
  printf("%0x ", Packet[1]);
  uint8_t index[2] = {Packet[2], Packet[3]};
  //printf("index:%u ", u8tou16(index));
  printf("%u ", u8tou16(index));
  uint8_t tempo[4] = {Packet[4], Packet[5], Packet[6], Packet[7]};
  //printf("atraso:%lu msg:", u8tou32(tempo));
  printf("%lu ", u8tou32(tempo));
  //for (int i = 8; i < PACKET_SIZE; i++) {
  //  printf("%c", char(Packet[i]));
  //}
  printf("%u ", Packet[9] ); //perdidos
  Serial.println();
}

void loop(void)
{
  //newConnections();
#ifdef DEBUG
  printList();
#endif

  /*  for (int i = 0; i < qtde_nodes; i++) {
      sendToken(list_nodes[i]);
      //imprime pacote

    }
  */
  waitPackets();
#ifdef DEBUG
  Serial.println("enviou todos os tokens");
#endif
  //delay(500);
  //startRadio()
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
