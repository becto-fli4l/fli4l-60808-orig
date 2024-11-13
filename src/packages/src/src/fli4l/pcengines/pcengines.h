//Device to read Memory from - normally /dev/mem
//if 'biosfile' is defined, a file 'bios' is read - for debug-purposes
#ifndef biosfile
#define memdev "/dev/mem"
#else
#define memdev "bios"
#endif

//String definitions
#define enabled "enabled"
#define disabled "disabled"


#define baudstring "Baud-Rate "
#define lba_string "LBA thres "
#define pxe_string "PXE-boot  "
#define consstring "Console   "
#define usb_string "USB Port  "
#define hd2_string "HD Slave  "
#define hdw_string "HD Wait   "
#define mfgpstring "MFGPT fix "
#define udmastring "UDMA      "
#define lpcistring "Late PCI  "
#define wd__string "Watchdog  "
#define dramstring "DRAM divid"
#define biosstring "BIOS      "

#define foundstring "found     "
#define guessstring "guessing  "
#define identstring "Ident     "

#define openerror "Cannot open %s for reading\n"

#define alix "ALIX"
#define wrap "WRAP"

#define integerout "%s: %d\n"
#define stringout "%s: %s\n"

#define tinybios "tinyBIOS"
#define alixdetect "PC Engines ALIX"
#define wrapdetect "PC Engines WRAP"

struct alixbios {
    unsigned char baud;
    unsigned char baud_;
    unsigned char lba_lsb;
    unsigned char lba_msb;
    unsigned char pxe;
    unsigned char pxe_;
    unsigned char cons;
    unsigned char cons_;
    unsigned char hd2;
    unsigned char hd2_;
    unsigned char hwa;
    unsigned char hwa_;
    unsigned char mfg;
    unsigned char mfg_;
    unsigned char udma;
    unsigned char udma_;
    unsigned char lpci;
    unsigned char lpci_;
    unsigned char wdog;
    unsigned char wdog_;
};

struct wrapbios {
    unsigned char baud;
    unsigned char baud_;
    unsigned char lba_lsb;
    unsigned char lba_msb;
    unsigned char usb;
    unsigned char usb_;
    unsigned char pxe;
    unsigned char pxe_;
    unsigned char cons;
    unsigned char cons_;
    unsigned char dram;
    unsigned char dram_;
};
