#include <Arduino.h>
#include "Framework.h"
#include "MijiaBleListener.h"

void setup() {
  Framework::one(115200);

  module = new MijiaBleListener();

  Framework::setup();
}

void loop() {
  Framework::loop();
}