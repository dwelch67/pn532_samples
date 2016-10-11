
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
//int test_payload ( unsigned int rx )
//{
    //unsigned int ra;

    //for(ra=0;ra<rx;ra++)
    //{
        //printf("0x%02X ",payload[ra]);
    //}
    //printf("\n");
//}


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
    unsigned int ra,rb,rc,rd;
    unsigned int rx;
    unsigned int binlen;
    unsigned int block;
    FILE *fp;

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
    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");
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
    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");
    ////0xD5 0x05 0x00 0x00 0x00 0x80

    InListPassiveTarget();
    rx=test_response();
    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");

    //So I have read if you cant auth, and getversion doesnt work
    //then it is probably an ultralight, not the C nor EV1.
    //read_sig doesnt work either, so not a nano this is an ultralight

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
    if(rc)
    {
        printf("goodbye\n");
        return(1);
    }

    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x40; //InDataExchange
    cdata[ra++]=0x01; //target id
    cdata[ra++]=0x60; //GET_VERSION
    send_command(ra);
    rx=test_response();
    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");


    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x40; //InDataExchange
    cdata[ra++]=0x01; //target id
    cdata[ra++]=0x3C; //READ_SIG
    send_command(ra);
    rx=test_response();
    for(ra=0;ra<rx;ra++)
    {
        printf("0x%02X ",payload[ra]);
    }
    printf("\n");

    if(0)
    {
        unsigned int block;
        unsigned int zz;
        zz=11;
        for(block=0x04;block<0x10;block++)
        {
            ra=0;
            cdata[ra++]=0xD4;
            cdata[ra++]=0x40; //InDataExchange
            cdata[ra++]=0x01; //target id
            cdata[ra++]=0xA2; //WRITE (one (4 byte) page)
            cdata[ra++]=block; //address
            cdata[ra++]=zz; zz++;
            cdata[ra++]=zz; zz++;
            cdata[ra++]=zz; zz++;
            cdata[ra++]=zz; zz++;
            send_command(ra);
            rx=test_response();
            for(ra=0;ra<rx;ra++)
            {
                printf("0x%02X ",payload[ra]);
            }
            printf("\n");
            if(block==0) break;
        }
    }

    {
        unsigned int block;

        for(block=0x00;block<0x10;block+=4)
        {
            ra=0;
            cdata[ra++]=0xD4;
            cdata[ra++]=0x40; //InDataExchange
            cdata[ra++]=0x01; //target id
            cdata[ra++]=0x30; //READ
            cdata[ra++]=block; //addr
            send_command(ra);
            rx=test_response();
            for(ra=0;ra<rx;ra++)
            {
                printf("0x%02X ",payload[ra]);
            }
            printf("\n");
        }
    }

    return(0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

