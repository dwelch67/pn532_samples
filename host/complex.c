
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//MOSI/SDA/HSU_TX
//NSS/SCL/HSU_RX


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ser.h"

unsigned int seq;
unsigned int ra,rb,rc,rd;
unsigned int addr;

unsigned char sdata[512];
unsigned char cdata[512];
unsigned char payload[512];
unsigned char rdata[5000];
unsigned int bindata[16384>>2];
//1 x Mifare One S50 White Card
//1 x Mifare One S50 Key Card

//ISO/IEC 14443  A  13.56  106mhz 16bit crc


unsigned char test_key[9][6]=
{
{0XD3,0XF7,0XD3,0XF7,0XD3,0XF7}, //0
{0XA0,0XA1,0XA2,0XA3,0XA4,0XA5}, //1
{0XB0,0XB1,0XB2,0XB3,0XB4,0XB5}, //2
{0X4D,0X3A,0X99,0XC3,0X51,0XDD}, //3
{0X1A,0X98,0X2C,0X7E,0X45,0X9A}, //4
{0XAA,0XBB,0XCC,0XDD,0XEE,0XFF}, //5
{0X00,0X00,0X00,0X00,0X00,0X00}, //6
{0XAB,0XCD,0XEF,0X12,0X34,0X56}, //7
{0XFF,0XFF,0XFF,0XFF,0XFF,0XFF}  //8
};

//-----------------------------------------------------------------------------
int send_command ( unsigned int len )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    len&=0xFF;

    ra=0;
    sdata[ra++]=0x00;
    sdata[ra++]=0x00;
    sdata[ra++]=0xFF;
    sdata[ra++]=len;
    sdata[ra++]=(0x100-len)&0xFF;
    rc=0;
    for(rb=0;rb<len;rb++)
    {
        sdata[ra++]=cdata[rb];
        rc+=cdata[rb];
    }
    sdata[ra++]=(0x100-rc)&0xFF;
    sdata[ra++]=0x00;
    sdata[ra++]=0x00;
//for(rb=0;rb<ra;rb++) printf("   0x%02X\n",sdata[rb]);
    ser_senddata(sdata,ra);
    return(0);
}

void InListPassiveTarget ( void )
{
    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x4A; //InListPassiveTarget
    cdata[ra++]=0x01; //number of targets
    cdata[ra++]=0x00; //106 kbps type A
    send_command(ra);
}

void show_payload ( unsigned int rx )
{
    unsigned int ra;

    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");
}


unsigned int test_response ( void )
{
    unsigned int rb;
    unsigned int ra;
    unsigned int rx;


    unsigned int state;
    unsigned char len;
    unsigned char sum;

    state=0;
    rx=0;
    len=0;
    sum=0;
    while(1)
    {
        rb=ser_copystring(rdata);
        if(rb)
        {
            for(ra=0;ra<rb;ra++)
            {
                switch(state)
                {
                    case 0:
                    {
                        if(rdata[ra]==0x00) state++;
                        break;
                    }
                    case 1:
                    {
                        if(rdata[ra]==0xFF) state++;
                        else
                        if(rdata[ra]!=0x00) state=0;
                        break;
                    }
                    case 2:
                    {
                        len=rdata[ra];
                        state++;
                        break;
                    }
                    case 3:
                    {
                        if(rdata[ra]==((0x100-len)&0xFF))
                        {
                            if(len==0) state=0;
                            state++;
                            rx=0;
                            sum=0;
                        }
                        else
                        {
                            state=0;
                        }
                        break;
                    }
                    case 4:
                    {
                        payload[rx++]=rdata[ra];
                        sum+=rdata[ra];
                        if((--len)==0) state++;
                        break;
                    }
                    case 5:
                    {
                        sum+=rdata[ra];
                        sum&=0xFF;
                        if(sum)
                        {
                            state=0;
                        }
                        else
                        {
                            //test_payload(rx);
                            state=0;
                            ser_dump(rb);
                            return(rx);
                        }
                        break;
                    }
                }
                //printf("0x%02X %u %u 0x%02X\n",rdata[ra],state,len,payload[0]);
            }
            ser_dump(rb);
        }
    }
}
unsigned int dokey ( void )
{
    //hmmm this should take 1000 to 2000 years to complete if my
    //math is right.  Wait maybe it is over 700,000 years to complete.
    unsigned int ra;
    unsigned int rc;
    unsigned int rx;
    unsigned int keytest[6];

    keytest[0]=0x00;
    keytest[1]=0x00;
    keytest[2]=0x00;
    keytest[3]=0x00;
    keytest[4]=0x00;
    keytest[5]=0x00;

    while(1)
    {
        InListPassiveTarget();
        rx=test_response();
        //show_payload(rx);

        rc=0;
        if(rx!=12) rc++;
        if(payload[ 0]!=0xD5) rc++;
        if(payload[ 1]!=0x4B) rc++; //0x4A response
        if(payload[ 2]!=0x01) rc++; //Number of targets
        if(payload[ 3]!=0x01) rc++; //target number
        if(payload[ 4]!=0x00) rc++; //SENS_RES msb
        if(payload[ 5]!=0x04) rc++; //SENS_RES lsb
        if(payload[ 6]!=0x08) rc++; //SEL_RES
        if(payload[ 7]!=0x04) rc++; //NFCIDLength //ultralight are 7 bytes
        if(rc)
        {
            show_payload(rx);
            printf("goodbye\n");
            return(1);
        }
        for(ra=0;ra<6;ra++) printf("%02X",keytest[ra]); printf("\n");

        ra=0;
        cdata[ra++]=0xD4;
        cdata[ra++]=0x40; //InDataExchange
        cdata[ra++]=0x01; //target id
        cdata[ra++]=0x60; //Key A authenticate
        cdata[ra++]=0x00; //block address
        cdata[ra++]=keytest[0];
        cdata[ra++]=keytest[1];
        cdata[ra++]=keytest[2];
        cdata[ra++]=keytest[3];
        cdata[ra++]=keytest[4];
        cdata[ra++]=keytest[5];
        cdata[ra++]=payload[8];
        cdata[ra++]=payload[9];
        cdata[ra++]=payload[10];
        cdata[ra++]=payload[11];
        send_command(ra);
        rx=test_response();
        //show_payload(rx);
        //0xD5 0x41 0x00

        if(payload[0]==0xD5)
        if(payload[1]==0x41)
        if(payload[2]==0x14)
        {

            keytest[5]++;
            if(keytest[5]>0xFF)
            {
                keytest[5]=0;
                keytest[4]++;
            }
            if(keytest[4]>0xFF)
            {
                keytest[4]=0;
                keytest[3]++;
            }
            if(keytest[3]>0xFF)
            {
                keytest[3]=0;
                keytest[2]++;
            }
            if(keytest[2]>0xFF)
            {
                keytest[2]=0;
                keytest[1]++;
            }
            if(keytest[1]>0xFF)
            {
                keytest[1]=0;
                keytest[0]++;
            }
            if(keytest[0]>0xFF)
            {
                printf("tried them all!\n");
                return(1);
            }
            ra=0;
            cdata[ra++]=0xD4;
            cdata[ra++]=0x44; //InRelease
            cdata[ra++]=0x00; //target id
            send_command(ra);
            rx=test_response();
            //show_payload(rx);
            continue;
        }
        if(payload[0]==0xD5)
        if(payload[1]==0x41)
        if(payload[2]==0x00)
        {
            printf("GOT IT!!!\n");
            for(ra=0;ra<6;ra++)
            {
                printf("key[%u]=0x%02X\n",ra,keytest[ra]);
            }
            printf("\n");
            break;
        }
        show_payload(rx);
        return(1);
    }

    return(0);
}
//-----------------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
    unsigned int ra,rb;
    unsigned int rx;

    if(argc<2)
    {
        printf("progstm /dev/ttyXYZ\n");
        return(1);
    }

    if(ser_open(argv[1]))
    {
        printf("ser_open() failed\n");
        return(1);
    }
    printf("port opened\n");

    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x14; //SAMConfiguration
    cdata[ra++]=0x01; //Normal mode SAM not used
    send_command(ra);

    //------------------------
    //WHY!!??
    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x14; //SAMConfiguration
    cdata[ra++]=0x01; //Normal mode SAM not used
    send_command(ra);
    for(ra=0;ra<10000;ra++)
    {
        rb=ser_copystring(rdata);
        if(rb)
        {
            //for(ra=0;ra<rb;ra++) printf("0x%02X\n",rdata[ra]);
            ser_dump(rb);
        }
    }
    //WHY?!
    //------------------------

    //GetFirmwareVersion
    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x02;
    send_command(ra);
    rx=test_response();
    show_payload(rx);
    ////0xD5 0x03 0x32 0x01 0x06 0x07
    ////IC 0x32  PN532
    ////Ver 1
    ////Rev 6
    ////Support ISO18092, ISO/IEC 14443 Type B, ISO/IEC 14443 TypeA

    //GetGeneralStatus
    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x04;
    send_command(ra);
    rx=test_response();
    show_payload(rx);
    ////0xD5 0x05 0x00 0x00 0x00 0x80

    //if(dokey()) return(1);

    //ra=0;
    //cdata[ra++]=0xD4;
    //cdata[ra++]=0x40; //InDataExchange
    //cdata[ra++]=0x01; //target id
    //cdata[ra++]=0x60; //Key A authenticate
    //cdata[ra++]=0x00; //block address
    //cdata[ra++]=0xFF; //key
    //cdata[ra++]=0xFF; //key
    //cdata[ra++]=0xFF; //key
    //cdata[ra++]=0xFF; //key
    //cdata[ra++]=0xFF; //key
    //cdata[ra++]=0xFF; //key
    //cdata[ra++]=payload[8];
    //cdata[ra++]=payload[9];
    //cdata[ra++]=payload[10];
    //cdata[ra++]=payload[11];
    //send_command(ra);
    //rx=test_response();
    //show_payload(rx);
    ////0xD5 0x41 0x00

    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x40; //InDataExchange
    cdata[ra++]=0x01; //target id
    cdata[ra++]=0x60; //GET_VERSION
    send_command(ra);
    rx=test_response();
    show_payload(rx);

    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x40; //InDataExchange
    cdata[ra++]=0x01; //target id
    cdata[ra++]=0x30; //read block a
    cdata[ra++]=0; //block address
    send_command(ra);
    rx=test_response();
    show_payload(rx);


    //for(block=0;block<4;block++)
    //{
        //ra=0;
        //cdata[ra++]=0xD4;
        //cdata[ra++]=0x40; //InDataExchange
        //cdata[ra++]=0x01; //target id
        //cdata[ra++]=0x30; //read block a
        //cdata[ra++]=block; //block address
        //send_command(ra);
        //rx=test_response();

        //if(payload[0]==0xD5)
        //if(payload[1]==0x41)
        //if(payload[2]==0x00)
        //{
            //printf("block 0x%02X ",block);
            //for(ra=0;ra<rx;ra++)
            //{
                //printf("0x%02X ",payload[ra]);
            //}
            //printf("\n");
        //}
    //}


    return(0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

