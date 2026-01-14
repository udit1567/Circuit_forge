#include "bootanimation.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include <zephyr/storage/flash_map.h>

Arduino_LED_Matrix matrix;

bool ok = false;

void setup() {

    Serial.begin(115200);
    matrix.begin();
    matrix.textFont(Font_5x7);
    matrix.setGrayscaleBits(8);

    // Before flashing, show the animation
    for (int i = 0; i < 2; i++) {
		matrix.playVideo(bootanimation.buf_loop, bootanimation.len_loop);
	}
	matrix.playVideo((uint8_t*)(bootanimation.buf_loop + bootanimation.len_loop), bootanimation.len_end);

    const struct flash_area *fa;
	int rc;
	rc = flash_area_open(FIXED_PARTITION_ID(bootanimation), &fa);
	if (rc) {
		return;
	}

	// flash the bootanimation header
	rc = flash_area_erase(fa, 0, fa->fa_size);
	if (rc) {
		return;
	}
    int header_len = sizeof(bootanimation) - sizeof(char*);
	rc = flash_area_write(fa, 0, &bootanimation, header_len);
    if (rc) {
        return;
    }
    rc = flash_area_write(fa, header_len, bootanimation.buf_loop, bootanimation.len_loop + bootanimation.len_end);
    if (rc) {
        return;
    }
    uint32_t flash_area[256];
	flash_area_read(fa, 0, flash_area, 256);
    Serial.println(flash_area[0], HEX);
    Serial.println(flash_area[1], HEX);
    Serial.println(flash_area[2], HEX);
    Serial.println(flash_area[3], HEX);
    Serial.println(flash_area[4], HEX);
    Serial.println(flash_area[5], HEX);
    ok = true;
}

void loop() {
    matrix.beginText(0, 0, 127, 0, 0); // X, Y, then R, G, B
    if (ok) {
        matrix.print("OK :)");
    } else {
        matrix.print("KO :(");
    }
    matrix.endText();
}