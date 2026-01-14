/*
   Partition Info

   The sketch reads the MBR (Master Boot Record) partition table
   from flash storage and prints the results on the serial monitor.

   This example code is in the public domain.
 */

#include <Arduino.h>
#include <zephyr/storage/flash_map.h>

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

unsigned long allocatedSpace = 0;

void setup() {
    Serial.begin(9600);
    for (unsigned long timeout = millis() + 2500; !Serial && millis() < timeout; delay(250));

    Serial.println();
    Serial.print("Looking for Partitions on the Flash Memory... ");

    // Open the MBR partition to read the partition table
    const struct flash_area *fa;
    int ret = flash_area_open(FIXED_PARTITION_ID(mbr_partition), &fa);
    if (ret) {
        Serial.println("ERROR! Unable to open flash device.");
        return;
    }

    // Read MBR from sector 0
    // MBR is in the last 66 bytes of sector 0 (at offset 446)
    uint8_t buffer[512];
    ret = flash_area_read(fa, 0, buffer, 512);
    if (ret) {
        Serial.println("ERROR! Unable to read the Master Boot Record");
        flash_area_close(fa);
        return;
    }

    // MBR partition table is at offset 446, signature at 510
    mbrTable *table = (mbrTable*)&buffer[446];

    if (table->signature[0] != 0x55 || table->signature[1] != 0xAA) {
        Serial.println("MBR Not Found");
        Serial.println("Flash Memory doesn't have partitions.");
    } else {
        Serial.println("MBR Found");
        Serial.print("Boot Signature: 0x");
        Serial.print(table->signature[0], HEX);
        Serial.println(table->signature[1], HEX);

        Serial.println();
        Serial.println("Printing Partitions Table and Info...");

        uint32_t part = 1;
        for (const auto& entry : table->entries) {
            Serial.println("================================");
            Serial.print("Partition: ");
            Serial.println(part++);

            Serial.print("Bootable: ");
            Serial.println(entry.status == 0 ? "No" : "Yes");

            Serial.print("Type: 0x");
            if (entry.type < 0x10)
                Serial.print(0);
            Serial.println(entry.type, HEX);

            if (entry.type == 0x00)
                continue;

            Serial.print("Size [KBytes]: ");
            Serial.println((entry.lbaSize * 4096) >> 10);

            allocatedSpace += entry.lbaSize * 4096;

            Serial.print("Start [C/H/S]: ");
            Serial.print(entry.chsStart[0]);
            Serial.print("/");
            Serial.print(entry.chsStart[1]);
            Serial.print("/");
            Serial.println(entry.chsStart[2]);

            Serial.print("Stop [C/H/S]: ");
            Serial.print(entry.chsStop[0]);
            Serial.print("/");
            Serial.print(entry.chsStop[1]);
            Serial.print("/");
            Serial.println(entry.chsStop[2]);

            Serial.println();
        }

        Serial.println();
        Serial.println("No more partitions are present.");
    }

    Serial.println();
    Serial.print("Total Space [KBytes]:         ");
    Serial.println(fa->fa_size >> 10);
    Serial.print("Allocated Space [KBytes]:     ");
    Serial.println(allocatedSpace >> 10);
    Serial.print("Unallocated Space [KBytes]:   ");
    Serial.println((fa->fa_size - allocatedSpace) >> 10);

    flash_area_close(fa);
}

void loop() {
    delay(10000);
}
