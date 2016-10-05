
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000-2015
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

//ISO/IEC 14443 aA  13.56  106mhz 16bit crc

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

//-----------------------------------------------------------------------------
int new_code ( unsigned int code )
{
    printf("%08X\n",code);
}
//-----------------------------------------------------------------------------
int new_payload ( unsigned int rx )
{
    unsigned int rc;
    unsigned int code;

    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");
    rc=0;
    if(rx!=12) rc++;
    if(payload[ 0]!=0xD5) rc++;
    if(payload[ 1]!=0x4B) rc++;
    if(payload[ 2]!=0x01) rc++;
    if(payload[ 3]!=0x01) rc++;
    if(payload[ 4]!=0x00) rc++;
    if(payload[ 5]!=0x04) rc++;
    if(payload[ 6]!=0x08) rc++;
    if(payload[ 7]!=0x04) rc++;
    //if(payload[ 8]!=0x65) rc++;
    //if(payload[ 9]!=0x79) rc++;
    //if(payload[10]!=0xD2) rc++;
    //if(payload[11]!=0x65) rc++;
    code=0;
    code<<=8; code|=payload[ 8];
    code<<=8; code|=payload[ 9];
    code<<=8; code|=payload[10];
    code<<=8; code|=payload[11];
    if(rc==0)
    {
        new_code(code);
    }
    if(rx>1)
    if(payload[0]==0xD5);
    if(payload[1]==0x4B);

    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x44;
    cdata[ra++]=0x01;
    send_command(ra);
    InListPassiveTarget();
    return(0);
}
//-----------------------------------------------------------------------------
int parse_response ( void )
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
                        if(rdata[ra]==(0x100-len)&0xFF)
                        {
                            if(len==0) state==0;
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
                            new_payload(rx);
                            state=0;
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
//-----------------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
    unsigned int ra,rb,rc,rd;
    unsigned int binlen;
    FILE *fp;

    if(argc<2)
    {
        printf("progstm /dev/ttyXYZ\n");
        return(1);
    }



//InListPassivTarget, to initialise one (several) cards (maximum two cards at the
//same time)
//- InDataExchange, to send Mifare commands
//- InSelect, InDeselect, and InRelease to select, and release the card (this is
//optional, see paragraph 3.3.7.3 on page 56).


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

    //ra=0;
    //cdata[ra++]=0xD4;
    //cdata[ra++]=0x4A;
    //cdata[ra++]=0x01;
    //cdata[ra++]=0x00;
    //send_command(ra);
    InListPassiveTarget();
    parse_response();
    return(0);
}
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000-2015
//-----------------------------------------------------------------------------

