//main.cpp file+++++++++++++++++++++++++++++++++++++++++
#include "mbed.h"
#include "nRF24L01P.h"
#include <cstdio>
 
nRF24L01P my_nrf24l01p(D11, D12, D13,D9,D10,D7);    // mosi, miso, sck, csn, ce, irq
//pin used by frdm k64f platform
DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
int TrCounter=0;
int main() {

    
    #define TRANSFER_SIZE   4
    char rxData[TRANSFER_SIZE];
    
    char txData[TRANSFER_SIZE] = {'3', '5', '7','3'};
    int txDataCnt = 0;
    int rxDataCnt = 0;
    my_nrf24l01p.powerUp();
    my_nrf24l01p.setRfOutputPower(-6);
    my_nrf24l01p.setTxAddress((0x1F22676D90),DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    my_nrf24l01p.setRxAddress((0x1F22676D90),DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    my_nrf24l01p.setAirDataRate(2000);
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );
    my_nrf24l01p.setTransferSize( TRANSFER_SIZE );
    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();
     
    while (1) {
 
        // If we've received anything over the host serial link...
            // ...add it to the transmit buffer
           // If the transmit buffer is full
           txDataCnt = 4;
           if ( txDataCnt >= sizeof( txData ) )
            {
               // Send the transmitbuffer via the nRF24L01+
              
                TrCounter=my_nrf24l01p.write( NRF24L01P_PIPE_P0, txData, txDataCnt );
                
               // wait_us(500000);
                
               myled1 = !myled1;
            }
 
          // If we've received anything in the nRF24L01+...
          if ( my_nrf24l01p.readable() ) {
          // ...read the data into the receive buffer
          rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
          printf("receiving data of size: %d",rxDataCnt);
          // Display the receive buffer contents via the host serial link
          printf(", Data=");
          for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) printf( "%c",rxData[i] );
          printf("\n");
          //myled2 = !myled2;
        }
    }
}    