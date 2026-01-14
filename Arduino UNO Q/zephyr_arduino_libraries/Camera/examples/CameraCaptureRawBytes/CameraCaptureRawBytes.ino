#include "camera.h"

Camera cam;

void fatal_error(const char *msg) {
  Serial.println(msg);
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
  }
}

void setup(void) {
  Serial.begin(115200);    
  if (!cam.begin(320, 240, CAMERA_RGB565)) {
    fatal_error("Camera begin failed");
  }
  cam.setVerticalFlip(false);
  cam.setHorizontalMirror(false);
}

void loop() {
  FrameBuffer fb;
  if (cam.grabFrame(fb)) {
    if (Serial.read() == 1) {
      Serial.write(fb.getBuffer(), fb.getBufferSize());
    }
    cam.releaseFrame(fb);
  }
}
