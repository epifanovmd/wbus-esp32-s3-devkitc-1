// src/main.cpp
#include "WebastoApplication.h"

// Глобальный экземпляр приложения
WebastoApplication app;

void setup() {
      app.initialize();
}

void loop() {
    app.process();
}