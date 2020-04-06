#ifndef _ISA_DEFS_H
#define _ISA_DEFS_H

typedef struct adapter_info_t {
    char *devname;
    char *venname;
    unsigned device_count;
}TAdapterInfo;

typedef struct dev_info_t
{
    TAdapterInfo common;
    unsigned channel_count;
}TDevInfo;

#endif
