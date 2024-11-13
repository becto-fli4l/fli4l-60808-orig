/*----------------------------------------------------------------------------
 *  FindDrive.c - list all available volumes on Darwin/Apple Mac OS X
 *
 *  Compilation by Makefile, does only build on Darwin or Apple Mac OS X
 *
 *  Copyright (c) 2004 Michael Hanselmann
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:    01.02.2004  hansmi
 *  Last update: $Id$
 *----------------------------------------------------------------------------
 */

#include <CoreServices/CoreServices.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/IOBSD.h>
#include <paths.h>
#include <sys/param.h>
#include <mach/mach_port.h>
#include <getopt.h>

char* getVolumeName(char* deviceName) {
    // This function is based on the "VolumeToBSDNode"-sample from Apple,
    // available at the following url:
    // http://developer.apple.com/samplecode/VolumeToBSDNode/listing1.html

    OSErr result = noErr;
    ItemCount volumeIndex;

    // Iterate across all mounted volumes using FSGetVolumeInfo. This
    // will return nsvErr (no such volume) when volumeIndex becomes
    // greater than the number of mounted volumes.
    for(volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++) {
        FSVolumeRefNum  actualVolume;
        HFSUniStr255    volumeName;

        // We're mostly interested in the volume reference number (actualVolume)
        result = FSGetVolumeInfo(kFSInvalidVolumeRefNum, volumeIndex,
                                 &actualVolume, kFSVolInfoFSInfo,
                                 NULL, &volumeName, NULL);

        if(result == noErr) {
            GetVolParmsInfoBuffer volumeParms;
            HParamBlockRec pb;

            // Use the volume reference number to retrieve the volume
            // parameters. See the documentation on PBHGetVolParmsSync
            // for other possible ways to specify a volume.
            pb.ioParam.ioNamePtr = NULL;
            pb.ioParam.ioVRefNum = actualVolume;
            pb.ioParam.ioBuffer = (Ptr) &volumeParms;
            pb.ioParam.ioReqCount = sizeof(volumeParms);

            // A version 4 GetVolParmsInfoBuffer contains the BSD node
            // name in the vMDeviceID field. It is actually a char * value.
            // This is mentioned in the header CoreServices/CarbonCore/Files.h.
            result = PBHGetVolParmsSync(&pb);

            if(result != noErr) {
                fprintf(stderr, "PBHGetVolParmsSync returned %d\n", result);
            }else{
                // This code is just to convert the volume name from a
                // HFSUniCharStr to a plain C string so we can print it
                // with printf. It'd be preferable to use CoreFoundation
                // to work with the volume name in its Unicode form.
                CFStringRef volNameAsCFString =
                    CFStringCreateWithCharacters(kCFAllocatorDefault,
                                                 volumeName.unicode,
                                                 volumeName.length);
                char volNameAsCString[256];

                // If the conversion to a C string fails, just treat it
                // as a null string.
                if(!CFStringGetCString(volNameAsCFString,
                                       volNameAsCString,
                                       sizeof(volNameAsCString),
                                       kCFStringEncodingUTF8)) {
                    volNameAsCString[0] = 0;
                }
                CFRelease(volNameAsCFString);

                // The last parameter of this printf call is the BSD node
                // name from the GetVolParmsInfoBuffer struct.
                // printf("%s: %s, %s\n", (char*)volumeParms.vMDeviceID,
                //        volNameAsCString, deviceName);
                if(!strcmp((char*)volumeParms.vMDeviceID, deviceName)) {
                    return strdup(volNameAsCString);
                }
            }
        }
    }

    return NULL;
}

int main(int argc, const char * argv[]) {
    bool bWholeMedia = false;
    bool bEjectableMedia = false;
    bool bRemoveableMedia = false;
    bool bWritable = false;
    bool bRaw = false;
    bool bName = false;
    bool bHeader = false;

#define OPTION_RAW 200
#define OPTION_NAME 201
#define OPTION_HEADER 202

    char const short_options[] = "hoerw";
    struct option long_options[] = {
        {"whole", no_argument, NULL, 'o'},
        {"ejectable", no_argument, NULL, 'e'},
        {"removable", no_argument, NULL, 'r'},
        {"writable", no_argument, NULL, 'w'},
        {"raw", no_argument, NULL, OPTION_RAW},
        {"name", no_argument, NULL, OPTION_NAME},
        {"header", no_argument, NULL, OPTION_HEADER},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };
    char const usage[] =
        "List all known volumes\n"
        "\n"
        "  -h, --help       print this text\n"
        "      --raw        print raw-device\n"
        "      --name       try to figure out the volume-name\n"
        "      --header     print column-header\n"
        "\n"
        "Filter:\n"
        "  -o, --whole      only print whole disks, not partitions\n"
        "  -e, --ejectable  only print ejectable volumes\n"
        "  -r, --removable  only print removable volumes\n"
        "  -w, --writable   only print writable volumes\n"
        "\n";

    int ch;
    while((ch = getopt_long(argc, (char**)argv, short_options,
                            long_options, NULL)) != -1) {
        switch(ch) {
            case 'o':
                bWholeMedia = true;
                break;
            case 'e':
                bEjectableMedia = true;
                break;
            case 'r':
                bRemoveableMedia = true;
                break;
            case 'w':
                bWritable = true;
                break;
            case OPTION_RAW:
                bRaw = true;
                break;
            case OPTION_NAME:
                bName = true;
                break;
            case OPTION_HEADER:
                bHeader = true;
                break;
            case 'h':
            default:
                printf("%s", usage);
                return 1;
        }
    }

    mach_port_t mpMasterPort;

    kern_return_t kernResult = IOMasterPort(MACH_PORT_NULL, &mpMasterPort);
    if(kernResult != KERN_SUCCESS) {
        fprintf(stderr, "IOMasterPort returned %d\n", kernResult);
    }else{
        CFMutableDictionaryRef mdMatchDictionary = IOServiceMatching(kIOMediaClass);
        if(!mdMatchDictionary) {
            fprintf(stderr, "IOServiceMatching returned a NULL dictionary.\n");
        }else{
            if(bWholeMedia) {
                CFDictionarySetValue(mdMatchDictionary,
                                     CFSTR(kIOMediaWholeKey),
                                     kCFBooleanTrue);
            }
            if(bEjectableMedia) {
                CFDictionarySetValue(mdMatchDictionary,
                                     CFSTR(kIOMediaEjectableKey),
                                     kCFBooleanTrue);
            }
            if(bRemoveableMedia) {
                CFDictionarySetValue(mdMatchDictionary,
                                     CFSTR(kIOMediaRemovableKey),
                                     kCFBooleanTrue);
            }
            if(bWritable) {
                CFDictionarySetValue(mdMatchDictionary,
                                     CFSTR(kIOMediaWritableKey),
                                     kCFBooleanTrue);
            }

            io_iterator_t mediaIterator;
            kernResult = IOServiceGetMatchingServices(mpMasterPort,
                                                      mdMatchDictionary,
                                                      &mediaIterator);
            if(bHeader) {
                printf("%-20s %-30s ", "Device", "Type");

                if(bName) {
                    printf("%s\n", "Name");
                }else{
                    printf("\n");
                }
                printf("--------------------------------------------------");
                if(bName) {
                    printf("--------------------\n");
                }else{
                    printf("\n");
                }
            }

            if((kernResult != KERN_SUCCESS) || mediaIterator == NULL) {
                fprintf(stderr, "No drive found. kernResult = %d\n", kernResult);
            }else{
                io_object_t device;
                IOIteratorReset(mediaIterator);

                while((device = IOIteratorNext(mediaIterator))) {
                    // MAXPATHLEN is defined in paths.h
                    char deviceFilePath[MAXPATHLEN];
                    deviceFilePath[0] = 0;

                    CFTypeRef deviceNameAsCFString =
                        IORegistryEntryCreateCFProperty(device,
                                                        CFSTR(kIOBSDNameKey),
                                                        kCFAllocatorDefault,0);
                    if(deviceNameAsCFString) {
                        CFStringGetCString(deviceNameAsCFString,
                                           deviceFilePath + strlen(deviceFilePath),
                                           MAXPATHLEN - strlen(deviceFilePath),
                                           kCFStringEncodingASCII);
                        CFRelease(deviceNameAsCFString);
                    }

                    io_name_t deviceName;

                    IORegistryEntryGetName(device, deviceName);

                    char tmp[MAXPATHLEN];
                    tmp[0] = 0;
                    snprintf(tmp, MAXPATHLEN,
                             "%s%s%s",
                             _PATH_DEV,
                             (bRaw?"r":""),
                             deviceFilePath);

                    char* volumeName = (bName?getVolumeName(deviceFilePath):NULL);

                    // print the found drive
                    printf("%-20s ", tmp);
                    if(volumeName) {
                        printf("%-30s %s\n", deviceName, volumeName);
                    }else{
                        printf("%s\n", deviceName);
                    }
                }
                IOObjectRelease(mediaIterator);
            }
        }

        mach_port_deallocate(mach_host_self(), mpMasterPort);
    }

    return 0;
}

