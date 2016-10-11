
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
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
    unsigned int ra,rb,rc;
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
        if(rc==0)
        {
            for(ra=0;ra<4;ra++)
            {
                printf("%02X",payload[ra+8]);
            }
            printf("\n");
        }
        else
        {
            rc=0;
            if(rx!=15) rc++;
            if(payload[ 0]!=0xD5) rc++;
            if(payload[ 1]!=0x4B) rc++; //0x4A response
            if(payload[ 2]!=0x01) rc++; //Number of targets
            if(payload[ 3]!=0x01) rc++; //target number
            if(payload[ 4]!=0x00) rc++; //SENS_RES msb
            if(payload[ 5]!=0x44) rc++; //SENS_RES lsb
            if(payload[ 6]!=0x00) rc++; //SEL_RES
            if(payload[ 7]!=0x07) rc++; //NFCIDLength //ultralight are 7 bytes
            if(rc==0)
            {
                for(ra=0;ra<7;ra++)
                {
                    printf("%02X",payload[ra+8]);
                }
                printf(" : ");
                ra=0;
                cdata[ra++]=0xD4;
                cdata[ra++]=0x40; //InDataExchange
                cdata[ra++]=0x01; //target id
                cdata[ra++]=0x30; //READ
                cdata[ra++]=0x00; //addr
                send_command(ra);
                rx=test_response();
                //show_payload(rx);
                if(payload[0]==0xD5)
                if(payload[1]==0x41)
                if(payload[2]==0x00)
                {
                    for(ra=0;ra<9;ra++)
                    {
                        printf("%02X",payload[ra+3]);
                    }
                    printf("\n");
                }
            }
            else
            {
                show_payload(rx);
            }
        }
        ra=0;
        cdata[ra++]=0xD4;
        cdata[ra++]=0x44; //InRelease
        cdata[ra++]=0x00; //target id
        send_command(ra);
        rx=test_response();
        //show_payload(rx);
    }

    return(0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

