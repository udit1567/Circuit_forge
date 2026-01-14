/*
   Flash Format

   The sketch formats the board flash storage as follows:

 * Partition 1  1MB: used for network certificates
 * Partition 2  5MB: OTA
 * Partition 3  1MB: Provisioning KVStore
 * Partition 4  7MB: User data

 This example code is in the public domain.
 */

#include <Arduino.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/flash_map.h>
#include "certificates.h"

// MBR structures
struct __attribute__((packed)) mbrEntry {
    uint8_t status;
    uint8_t chsStart[3];
    uint8_t type;
    uint8_t chsStop[3];
    uint32_t lbaOffset;
    uint32_t lbaSize;
};

struct __attribute__((packed)) mbrTable {
    mbrEntry entries[4];
    uint8_t signature[2];
};

bool waitResponse() {
    bool proceed = false;
    bool confirmation = false;

    while (confirmation == false) {
        if (!Serial.available()) {
            continue;
        }

       switch (Serial.read()) {
           case 'y':
           case 'Y':
               confirmation = true;
               proceed = true;
               break;
           case 'n':
           case 'N':
               confirmation = true;
               proceed = false;
               break;
           default:
               continue;
       }
    }

    return proceed;
}

int formatPartition(unsigned int partition_id, int fs_type) {
    Serial.print("Formatting partition as ");
    Serial.println(fs_type == FS_FATFS ? "FAT..." : "LittleFS...");

    int rc = 0;
    if (fs_type == FS_FATFS) {
        rc = fs_mkfs(FS_FATFS, partition_id, NULL, 0);
    } else {
        rc = fs_mkfs(FS_LITTLEFS, partition_id, NULL, 0);
    }

    if (rc < 0) {
        Serial.print("Error formatting partition: ");
        Serial.println(rc);
        return rc;
    }

    Serial.print("Partition formatted successfully!");
    return 0;
}

int flashCertificates() {
    Serial.print("Certificate size: ");
    Serial.print(cacert_pem_len);
    Serial.println(" bytes");

    struct fs_file_t file;
    fs_file_t_init(&file);

    Serial.println("Opening /wlan:/cacert.pem for writing...");
    int rc = fs_open(&file, "/wlan:/cacert.pem", FS_O_CREATE | FS_O_WRITE);
    if (rc != 0) {
        Serial.print("Error opening cacert.pem: ");
        Serial.println(rc);
        return rc;
    }

    Serial.println("Writing certificates...");
    ssize_t ret = fs_write(&file, cacert_pem, cacert_pem_len);
    if (ret < 0) {
        Serial.print("Error writing certificates: ");
        Serial.println(ret);
        fs_close(&file);
        return rc;
    }

    rc = fs_sync(&file);
    if (rc != 0) {
        Serial.print("Warning: fs_sync failed: ");
        Serial.println(rc);
        fs_close(&file);
        return rc;
    }

    fs_close(&file);
    Serial.println("Certificates written successfully!");
    return 0;
}

int flashMBR() {
    Serial.println("Creating MBR partition table...");

    // Open the MBR partition
    const struct flash_area *fa;
    int ret = flash_area_open(FIXED_PARTITION_ID(mbr_partition), &fa);
    if (ret) {
        Serial.print("Error opening MBR partition: ");
        Serial.println(ret);
        return ret;
    }

    // Prepare 512-byte sector with MBR
    uint8_t sector[512];
    memset(sector, 0, 512);

    // MBR partition table starts at offset 446
    mbrTable *table = (mbrTable*)&sector[446];

    // Note: lbaSize is in units of 4096-byte blocks (not standard 512-byte sectors)
    // This matches the original Arduino implementation which uses 4KB sectors

    // Partition 1: WLAN (FAT32) - starts at 0x1000, size ~1020KB (255 blocks of 4KB)
    table->entries[0].status = 0x00;       // Not bootable
    table->entries[0].chsStart[0] = 0;
    table->entries[0].chsStart[1] = 2;
    table->entries[0].chsStart[2] = 0;
    table->entries[0].type = 0x0B;         // FAT32
    table->entries[0].chsStop[0] = 4;
    table->entries[0].chsStop[1] = 0;
    table->entries[0].chsStop[2] = 0;
    table->entries[0].lbaOffset = 1;       // 0x1000 / 4096 = 1
    table->entries[0].lbaSize = 255;       // (1MB - 4KB) / 4096 = 255

    // Partition 2: OTA (FAT32) - starts at 0x100000, size 5MB (1280 blocks of 4KB)
    table->entries[1].status = 0x00;
    table->entries[1].chsStart[0] = 4;
    table->entries[1].chsStart[1] = 1;
    table->entries[1].chsStart[2] = 0;
    table->entries[1].type = 0x0B;         // FAT32
    table->entries[1].chsStop[0] = 24;
    table->entries[1].chsStop[1] = 0;
    table->entries[1].chsStop[2] = 0;
    table->entries[1].lbaOffset = 256;     // 0x100000 / 4096 = 256
    table->entries[1].lbaSize = 1280;      // 5MB / 4096 = 1280

    // Partition 3: KVS (FAT32) - starts at 0x600000, size 1MB (256 blocks of 4KB)
    table->entries[2].status = 0x00;
    table->entries[2].chsStart[0] = 24;
    table->entries[2].chsStart[1] = 1;
    table->entries[2].chsStart[2] = 0;
    table->entries[2].type = 0x0B;         // FAT32
    table->entries[2].chsStop[0] = 28;
    table->entries[2].chsStop[1] = 0;
    table->entries[2].chsStop[2] = 0;
    table->entries[2].lbaOffset = 1536;    // 0x600000 / 4096 = 1536
    table->entries[2].lbaSize = 256;       // 1MB / 4096 = 256

    // Partition 4: Storage (LittleFS/FAT32) - starts at 0x700000, size 7MB (1792 blocks of 4KB)
    table->entries[3].status = 0x00;
    table->entries[3].chsStart[0] = 28;
    table->entries[3].chsStart[1] = 1;
    table->entries[3].chsStart[2] = 0;
    table->entries[3].type = 0x0B;         // FAT32 (could be 0x83 for LittleFS)
    table->entries[3].chsStop[0] = 56;
    table->entries[3].chsStop[1] = 0;
    table->entries[3].chsStop[2] = 0;
    table->entries[3].lbaOffset = 1792;    // 0x700000 / 4096 = 1792
    table->entries[3].lbaSize = 1792;      // 7MB / 4096 = 1792

    // MBR signature
    table->signature[0] = 0x55;
    table->signature[1] = 0xAA;

    // Erase the MBR partition
    ret = flash_area_erase(fa, 0, fa->fa_size);
    if (ret) {
        Serial.print("Error erasing MBR partition: ");
        Serial.println(ret);
        flash_area_close(fa);
        return ret;
    }

    // Write the MBR sector
    ret = flash_area_write(fa, 0, sector, 512);
    if (ret) {
        Serial.print("Error writing MBR: ");
        Serial.println(ret);
        flash_area_close(fa);
        return ret;
    }

    flash_area_close(fa);
    Serial.println("MBR created successfully!");
    return 0;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("\nWARNING! Running the sketch all the content of the flash storage will be erased.");
    Serial.println("The following partitions will be created:");
    Serial.println("Partition 1: Network certificates 1MB");
    Serial.println("Partition 2: OTA 5MB");
    Serial.println("Partition 3: Provisioning KVStore 1MB");
    Serial.println("Partition 4: User data 7MB");
    Serial.println("Do you want to proceed? Y/[n]");

    if (!waitResponse()) {
        return;
    }

    // Create MBR partition table FIRST before formatting
    if (flashMBR()) {
        return;
    }

    // Format Partition 1: WLAN
    Serial.println("\nPartition 1 (/wlan:) will be formatted as FAT.");
    Serial.println("Do you want to format it? Y/[n]");
    bool format_wlan = waitResponse();

    if (format_wlan) {
        if (formatPartition((uintptr_t) "wlan:", FS_FATFS)) {
            return;
        }

        Serial.println("\nDo you want to restore the TLS certificates? Y/[n]");
        if (waitResponse() && flashCertificates()) {
            return;
        }
    }

    // Format Partition 2: OTA
    Serial.println("\nPartition 2 (/ota:) will be formatted as FAT.");
    Serial.println("Do you want to format it? Y/[n]");
    if (waitResponse()) {
        if (formatPartition((uintptr_t) "ota:", FS_FATFS)) {
            return;
        }
    }

    // Format Partition 4: Storage
    Serial.println("\nDo you want to use LittleFS to format user data partition? Y/[n]");
    Serial.println("If No, FatFS will be used to format user partition.");
    bool useLittleFS = waitResponse();

    Serial.println("\nPartition 4 (/storage) will be formatted.");
    Serial.println("Do you want to format it? Y/[n]");
    if (waitResponse()) {
        if (useLittleFS) {
            unsigned int partition_id = DT_FIXED_PARTITION_ID(DT_PROP(DT_NODELABEL(storage_fs), partition));
            if (formatPartition(partition_id, FS_LITTLEFS)) {
                return;
            }
        } else {
            Serial.println("FatFS for storage partition not implemented in this example.");
            Serial.println("Please use LittleFS or update device tree configuration.");
        }
    }

    Serial.println("\nFlash storage formatted!");
    Serial.println("It's now safe to reboot or disconnect your board.");
}

void loop() {
    delay(1000);
}
