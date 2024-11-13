#include <stdio.h>
#include <string.h>

#include "pcengines.h"

int biostobaud(char baud)
{
  if (baud == 1) return 115200;
  if (baud == 2) return 57600;
  if (baud == 3) return 38400;
  return 9600;
}

int alixdecode (struct alixbios *alixbios)
{
    printf(integerout,baudstring,biostobaud(alixbios->baud));
    printf(integerout,lba_string,alixbios->lba_msb<<8+alixbios->lba_lsb);
    printf(stringout,pxe_string,alixbios->pxe?enabled:disabled);    
    printf(stringout,consstring,alixbios->cons?enabled:disabled);    
    printf(stringout,hd2_string,alixbios->hd2?enabled:disabled);    
    printf(stringout,hdw_string,alixbios->hwa?enabled:disabled);    
    printf(stringout,mfgpstring,alixbios->mfg?enabled:disabled);    
    printf(stringout,udmastring,alixbios->udma?enabled:disabled);    
    printf(stringout,lpcistring,alixbios->lpci?enabled:disabled);    
    printf(stringout,wd__string,alixbios->wdog?enabled:disabled);
    return alixbios->mfg?4:0;
}

int wrapdecode ( struct wrapbios *wrapbios )
{
    printf(integerout,baudstring,biostobaud(wrapbios->baud));
    printf(integerout,lba_string,wrapbios->lba_msb<<8+wrapbios->lba_lsb);
    printf(stringout,pxe_string,wrapbios->pxe?enabled:disabled);    
    printf(stringout,consstring,wrapbios->cons?enabled:disabled);      
    printf(stringout,usb_string,wrapbios->usb?enabled:disabled);    
    printf(stringout,dramstring,wrapbios->dram?"3.0":"2.5");
    return wrapbios->usb?2:0;
}

main ()
{
  FILE *mem;
  char tinybuf[100];
  char sysbuf[100];
  int i;
  struct alixbios alixbios;
  struct wrapbios wrapbios;
  int retval=0;
  if (mem=fopen(memdev,"r"))
  {
    fseek(mem,0xf9100,SEEK_SET);
    fread(tinybuf,1,64,mem);
    for (i=0;i<64,tinybuf[i]!=0x0d;i++);
    tinybuf[i]=0;
    if (!memcmp(tinybuf,tinybios,sizeof(tinybios)-1))
    {
      retval+=128;
      printf(stringout,biosstring,tinybuf);
      fseek(mem,0xf9003,SEEK_SET);
      fread(sysbuf,1,64,mem);
      // remove CR/LF - convert them to space
      for (i=0;i<64,sysbuf[i];i++) if (sysbuf[i]==0x0d||sysbuf[i]==0x0a) sysbuf[i]=' ';
      sysbuf[i]=0;				// get sure that end of string exists
      if (sysbuf[i-2]==0x0d) sysbuf[i-2]=0;	// remove trailing CR/LF
      for (i=0;i<64,sysbuf[i]==' ';i++);	// skip leading spaces
      printf(stringout,identstring,sysbuf+i);	// print out ident string
      if (!memcmp(sysbuf,alixdetect,sizeof(alixdetect)-1))
      {
        retval+=64;
        fseek(mem,0xf8000,SEEK_SET);
        fread((char *) &alixbios,1,sizeof(struct alixbios),mem);
        printf(stringout,foundstring,alix);
	retval+=alixdecode(&alixbios);
      }
      else if (!memcmp(sysbuf,wrapdetect,sizeof(wrapdetect)-1))
      {
        retval+=16;
        fseek(mem,0xf8000,SEEK_SET);
        fread((char *) &wrapbios,1,sizeof(struct wrapbios),mem);
        printf(stringout,foundstring,wrap);
	retval+=wrapdecode(&wrapbios);
      }	
      else if (tinybuf[0x1a] == '5')
      {
        retval+=8;
        fseek(mem,0xf8000,SEEK_SET);
        fread((char *) &wrapbios,1,sizeof(struct wrapbios),mem);
        printf(stringout,guessstring,wrap);
	retval+=wrapdecode(&wrapbios);
      }
      else if (tinybuf[0x1a] == '7')
      {
        retval+=32;
        fseek(mem,0xf8000,SEEK_SET);
        fread((char *) &alixbios,1,sizeof(struct alixbios),mem);
        printf(stringout,guessstring,alix);
	retval+=alixdecode(&alixbios);
      }
    }
    fclose(mem);
  }
  else
  {
    printf(openerror,memdev);
    return 255;
  }
  return retval;
}
