#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

Arduino_LED_Matrix matrix;

void setup() {
  // put your setup code here, to run once:
  matrix.begin();
  matrix.textFont(Font_5x7);
  matrix.textScrollSpeed(100);
  matrix.clear();
	Serial.begin(115200);
}

uint8_t shades[104] = {
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,
	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,
	6,	6,	6,	6,	6,	6,	6,	6,	6,	6,	6,	6,	6,
	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,
};

const uint32_t animation[][5] = {
	{ 0x38022020,
	  0x810408a0,
	  0x2200e800,
	  0x20000000,
	  66 },
	{ 0x1c011010,
	  0x40820450,
	  0x11007400,
	  0x10000000,
	  66 },
	{ 0x0e008808,
	  0x20410228,
	  0x08803a00,
	  0x08000000,
	  66 },
	{ 0x07004404,
	  0x10208114,
	  0x04401d00,
	  0x04000000,
	  66 },
	{ 0x03802202,
	  0x0810408a,
	  0x02200e80,
	  0x02000000,
	  66 },
	{ 0x01c01101,
	  0x04082045,
	  0x01100740,
	  0x01000000,
	  66 },
	{ 0x00e00880,
	  0x82041022,
	  0x808803a0,
	  0x00000000,
	  66 },
	{ 0x00700440,
	  0x40020011,
	  0x004401c0,
	  0x00000000,
	  66 },
	{ 0x00380200,
	  0x20010008,
	  0x802000e0,
	  0x00000000,
	  66 },
	{ 0x00180100,
	  0x10008004,
	  0x00100060,
	  0x00000000,
	  66 },
	{ 0x00080080,
	  0x08004002,
	  0x00080020,
	  0x00000000,
	  66 },
	{ 0x00000040,
	  0x04002001,
	  0x00040000,
	  0x00000000,
	  66 },
	{ 0x00000000,
	  0x02001000,
	  0x80000000,
	  0x00000000,
	  66 },
	{ 0x00000000,
	  0x00000000,
	  0x00000000,
	  0x00000000,
	  66 }
};

void loop() {
  // Roll a string using ArduinoGraphics
  matrix.beginText(0, 0, 127, 0, 0); // X, Y, then R, G, B
  matrix.print("      arduino.cc/uno-q      ");
  matrix.endText(SCROLL_LEFT);
  delay(1000);
	// Draw shades
  matrix.setGrayscaleBits(3);
  matrix.draw(shades);
  delay(1000);
	matrix.clear();
	// Play an animation
  matrix.loadSequence(animation);
	for (int i = 0; i < 10; i++) {
  	matrix.playSequence();
	}
}