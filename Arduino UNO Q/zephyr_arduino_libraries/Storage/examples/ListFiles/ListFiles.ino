/*
  List Files

  This sketch demonstrates how to list all mounted filesystems
  and their contents on Arduino boards with Zephyr RTOS.

  It will:
  - Display all mounted filesystems (FAT and LittleFS)
  - List the contents of each filesystem
  - Show file sizes and types

  This example code is in the public domain.
*/

#include <Arduino.h>
#include <zephyr/fs/fs.h>

void listDir(const char *path) {
    struct fs_dir_t dir = {0};
    struct fs_dirent entry;

    fs_dir_t_init(&dir);
    int res = fs_opendir(&dir, path);

    if (res) {
        Serial.print("Error opening directory ");
        Serial.print(path);
        Serial.print(" [error code: ");
        Serial.print(res);
        Serial.println("]");
        return;
    }

    Serial.print("\nContents of ");
    Serial.print(path);
    Serial.println(":");

    bool empty = true;
    while (true) {
        res = fs_readdir(&dir, &entry);
        if (res || entry.name[0] == '\0') {
            break;
        }

        empty = false;
        if (entry.type == FS_DIR_ENTRY_FILE) {
            Serial.print("  [FILE] ");
            Serial.print(entry.name);
            Serial.print(" (");
            Serial.print((size_t) entry.size);
            Serial.println(" bytes)");
        } else if (entry.type == FS_DIR_ENTRY_DIR) {
            Serial.print("  [DIR ] ");
            Serial.println(entry.name);
        }
    }

    if (empty) {
        Serial.println("  <empty>");
    }

    fs_closedir(&dir);
}

void listMounts() {
    const char *mnt_point;
    int idx = 0;
    int res;
    bool found = false;

    while (true) {
        res = fs_readmount(&idx, &mnt_point);
        if (res < 0) {
            break;
        }

        Serial.print("Mount point ");
        Serial.print(idx - 1);
        Serial.print(": ");
        Serial.print(mnt_point);

        // Detect filesystem type by mount point naming convention
        // FAT mount points typically end with ':', LittleFS mount points don't
        size_t len = strlen(mnt_point);
        if (len > 0 && mnt_point[len - 1] == ':') {
            Serial.print(" (FAT)");
        } else {
            Serial.print(" (LittleFS)");
        }
        Serial.println();

        found = true;
    }

    if (!found) {
        Serial.println("No mounted filesystems found!");
    }
}

void listAllMountDirs() {
    const char *mnt_point;
    int idx = 0;
    int res;

    while (true) {
        res = fs_readmount(&idx, &mnt_point);
        if (res < 0) {
            break;
        }
        listDir(mnt_point);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("\n=== Arduino Filesystem Information ===\n");

    Serial.println("=== Mounted Filesystems ===");
    listMounts();

    Serial.println("\n=== Filesystem Contents ===");
    listAllMountDirs();

    Serial.println("\nDone!");
}

void loop() {
    delay(1000);
}
