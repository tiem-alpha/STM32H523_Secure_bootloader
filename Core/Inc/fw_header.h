#ifndef FW_HEADER_H
#define FW_HEADER_H

#include <stdint.h>

#define FW_HEADER_MAGIC            0x53544248u    /* 'STBH' */
#define FW_HEADER_VERSION          0x0001u
#define FW_HEADER_HASH_SIZE        32u
#define FW_HEADER_SIGNATURE_SIZE   256u

#pragma pack(push, 1)
typedef struct
{
    uint32_t magic;                    /* Unique identifier for firmware header */
    uint16_t header_version;           /* Firmware header format version */
    uint32_t header_size;               /* size of the header */
    uint16_t image_version;            /* Firmware image version */
    uint32_t image_size;               /* Size of the firmware image in bytes */
    uint32_t device_id;               /* Vendor identifier for authenticity */
    // uint32_t load_address;             /* Destination address for the firmware */
    // uint32_t entry_point;              /* Firmware entry point address */
    uint32_t flags;                    /* Secure boot control flags */
    uint32_t timestamp;                /* Build or release timestamp */
    uint8_t  reserved[8];              /* Reserved for alignment/future use */
    uint8_t  hash[FW_HEADER_HASH_SIZE];
    uint8_t  signature[FW_HEADER_SIGNATURE_SIZE];
} fw_header_t;
#pragma pack(pop)

#endif /* FW_HEADER_H */
