#include "mbed.h"
#include "nRF24L01P.h"
#include <cstdio>
 
nRF24L01P my_nrf24l01p(D11, D12, D13,D9,D10,D7);    // mosi, miso, sck, csn, ce, irq
//pin used by frdm k64f platform
DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
int TrCounter=0;
// main() runs in its own thread in the OS
DigitalOut out1(D2);//ptb9 //left motor back
DigitalOut out2(D3);//pta1  left motor forward
DigitalOut out3(D4);//ptb23 //right motor back
DigitalOut out4(D5);//pta2  //right motor forward
PwmOut ENA(PTC10); //left motor
PwmOut ENB(PTC11); //right motor
 
bool robotOn = true;
bool initialStart = true;
bool mission = true;
 
Timer eightInch;
Timer tokyoDrift;
 
 int grid[8][8] = {{0,1,2,3,4,5,6,7},   // W
                   {1,0,0,0,0,0,0,0}, //S    //N
                   {2,0,0,0,0,0,0,0},    //E 
                   {3,0,0,0,0,0,0,0},
                   {4,0,0,0,0,0,0,0},
                   {5,0,0,0,0,0,0,0},
                   {6,0,0,0,0,0,0,0},
                   {7,0,0,0,0,0,0,0}};  //key 0:Not path 9:Path 10:turn left 11:turn right 12:destination 8:start

//to later be read from wirless module
int x1 = 0;
int y1 = 0;


//to later be read from wireless module
int x2 = 0;
int y2 = 0;

int x3;
int y3;


void vroom();
void skrtRight();
void skrtLeft();

void pathCreation();
void pathTraversing();
bool isWest = true;;
int xIterator = 1;
int yIterator = 0;
bool dataRetrieved = false;
 
int main()
{
    #define TRANSFER_SIZE   4
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
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
    while(dataRetrieved != true){
        if ( my_nrf24l01p.readable() ) {
            rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
            printf("receiving data");
            // Display the receive buffer contents via the host serial link
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) printf( "%c",rxData[i] );
            printf("\n");
            myled1 = 0;
            wait_us(100000);
            myled1 = 1;
            myled1 = 0;
            wait_us(100000);
            myled1 = 1;
            myled2 = !myled2;
            x1 = int(rxData[0]);
            y1 = int(rxData[1]);
            x2 = int(rxData[2]);
            y2 = int(rxData[3]);

            x3= x1;
            y3 = y1;
            printf("%c", x2);
            dataRetrieved = true;
        }
    }
    ENA.period(0.5);
    ENB.period(0.5);
    grid[x1][y1] = 8; //indicates a start
    grid[x2][y2] = 12; //indicates an end
    pathCreation();
    while (true) 
    {
        
       
        if(initialStart)
        {
            wait_us(2000000); //gives time to put robot on ground
            initialStart = false;
        }
        if(mission)
        {
            pathTraversing();
            mission = false;
        }
    }
}
 
void vroom()
{
  ENA = .7; //left
  ENB = .69; //right
  while(robotOn)
  {  //for later when continuing to move after turn.  
    eightInch.start();
    if(eightInch.read() > .54)
    {
        out2 = 0;
        out4 = 0;
        eightInch.stop();
        eightInch.reset();
        robotOn = false; // to be turned false in another function
        
    }
    else
    {
        out2 = 1;
        out4 = 1;
    }
  }
}
 
void skrtRight()
{
    ENA = 1;
    ENB = 1;
    while(!robotOn)
    {
        tokyoDrift.start();
        if(tokyoDrift.read() > .15)
        {
            out2 = 0;
            out3 = 0;
            tokyoDrift.stop();
            tokyoDrift.reset();
            robotOn = true;
        }
        else 
        {
            out2 = 1;
            out3 = 1;
        }
    }
}

void skrtLeft()
{
    ENA = 1;
    ENB = 1;
    while(!robotOn)
    {
        
        tokyoDrift.start();
        if(tokyoDrift.read() > .13)
        {
            out1 = 0;
            out4 = 0;
            tokyoDrift.stop();
            tokyoDrift.reset();
            robotOn = true;
        }
        else 
        {
            out1 = 1;
            out4 = 1;
        }
    }
}

void pathCreation()
{
    if (x1 >= x2) //start West
    {
        while (x3 != x2)
        {
            x3--;
            if (y3 != y2)
            {
                grid[x3][y1] = 9; //valid path int
            }
            else 
            {
                break; // destination reached do not change this value
            }
        }
        if (y3 > y2) //turn left if West
        {
            grid[x2][y3] = 10; //turn left int
            while (y3 != y2)
            {
                y3--;
                if (y3 == y2)
                {
                    break; //desination is reached;
                }
                else 
                {
                    grid[x2][y3] = 9; //valid path int
                }
            }
        }
        else if(y3 < y2) //turn right if West
        {
            grid[x2][y3] = 11; //turn right int
            while (y3 != y2)
            {
                y3++;
                if (y3 == y2)
                {
                    break; //desination is reached;
                }
                else
                {
                    grid[x2][y3] = 9; //valid path int
                }
            }
        }
    }
    else if (x1 < x2) //start East
    {
        while (x3 != x2)
        {
            x3++;
            if (x3 != x2)
            {
                grid[x3][y1] = 9; //valid path int
            }
            else
            {
                break; // destination reached do not change this value
            }
        }
        if (y3 > y2) //turn right if East
        {
            grid[x2][y3] = 11; //turn right int
            while (y3 != y2)
            {
                y3--;
                if (y3 == y2)
                {
                    break; //desination is reached;
                }
                else
                {
                    grid[x2][y3] = 9; //valid path int
                }
            }
        }
        else if (y3 < y2) //turn left if East
        {
            grid[x2][y3] = 10; //turn left int
            while (y3 != y2)
            {
                y3++;
                if (y3 == y2)
                {
                    break; //desination is reached;
                }
                else
                {
                    grid[x2][y3] = 9; //valid path int
                }
            }
        }
    }
}
void pathTraversing()
{
    if(x1 >= x2)
    {
        isWest = true;
        xIterator = -1;
        yIterator = 0;
    }
    else 
    {
        isWest = false;
        xIterator = 1;
        yIterator = 0;
    }

    x3 = x1;
    y3 = y1;

    while(mission)
    {
        int val = grid[x3][y3];
        switch (val) 
        {
            case 0:
                mission = false;
                break;
            case 8:
                robotOn = true;
                vroom();
                //wait_us(1000000);
                x3 += xIterator;
                y3 += yIterator;
                break;
            
            case 9:
                robotOn = true;
                vroom();
                //wait_us(1000000);
                x3 += xIterator;
                y3 += yIterator;
                break;

            case 10:
                robotOn = false;
                skrtLeft();
                wait_us(1000000);
                vroom();
                //wait_us(1000000);
                if(isWest)
                {
                    xIterator = 0;
                    yIterator = -1;
                }
                else 
                {
                    xIterator = 0;
                    yIterator = 1;
                }
                x3 += xIterator;
                y3 += yIterator;
                break;

            case 11:
                robotOn = false;
                skrtRight();
                wait_us(1000000);
                vroom();
                //wait_us(1000000);
                if(isWest)
                {
                    xIterator = 0;
                    yIterator = 1;
                }
                else 
                {
                    xIterator = 0;
                    yIterator = -1;
                }
                x3 += xIterator;
                y3 += yIterator;
                break;
            
            case 12:
                mission = false;
                break;
        }
        
    }
}
