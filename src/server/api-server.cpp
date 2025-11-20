#include "api-server.h"
#include <ArduinoJson.h>
#include "wbus/wbus.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-errors.h"

ApiServer apiServer;

#define FS LittleFS

ApiServer::ApiServer(): server(80) {}

void ApiServer::begin() {
  // Инициализация файловой системы
  initializeFileSystem();

  // Настройка endpoint-ов
  setupEndpoints();

  server.begin();
  Serial.println("✅ HTTP server started on port 80");
  printAvailableEndpoints();
}

void ApiServer::initializeFileSystem() {
  if (!LittleFS.begin(true)) {
    Serial.println("❌ Ошибка инициализации LittleFS");
    return;
  }

  Serial.println("✅ LittleFS инициализирован");
  listFilesystemContents();
}

void ApiServer::listFilesystemContents() {
  Serial.println("📁 Содержимое LittleFS:");
  File root = FS.open("/");
  if (!root) {
    Serial.println("   ❌ Не удалось открыть корневую директорию");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("   ❌ Корень не является директорией");
    root.close();
    return;
  }

  File file = root.openNextFile();
  int fileCount = 0;

  while (file) {
    fileCount++;
    Serial.println("   📄 " + String(file.name()) + " | Размер: " + String(file.size()) + " байт");
    file = root.openNextFile();
  }
  root.close();

  if (fileCount == 0) {
    Serial.println("   ℹ️  Файловая система пуста");
  }
}

void ApiServer::setupEndpoints() {
  // Статические файлы
  server.on("/", HTTP_GET, [this]() {
    serveHTML();
  });
  server.on("/fallback", HTTP_GET, [this]() {
    serveFallbackHTML();
  });

  // API endpoint-ы для данных
  server.on("/api/system/state", HTTP_GET, [this]() {
    handleGetSystemState();
  });
  server.on("/api/device/info", HTTP_GET, [this]() {
    handleGetDeviceInfo();
  });
  server.on("/api/sensors/data", HTTP_GET, [this]() {
    handleGetSensorsData();
  });
  server.on("/api/errors", HTTP_GET, [this]() {
    handleGetErrors();
  });
  server.on("/api/all", HTTP_GET, [this]() {
    handleGetAllData();
  });

  // API endpoint-ы для управления режимами
  server.on("/api/control/connect", HTTP_POST, [this]() {
    handleConnect();
  });
  server.on("/api/control/disconnect", HTTP_POST, [this]() {
    handleDisconnect();
  });
  server.on("/api/control/start_parking", HTTP_POST, [this]() {
    handleStartParkingHeat();
  });
  server.on("/api/control/start_ventilation", HTTP_POST, [this]() {
    handleStartVentilation();
  });
  server.on("/api/control/start_supplemental", HTTP_POST, [this]() {
    handleStartSupplementalHeat();
  });
  server.on("/api/control/start_boost", HTTP_POST, [this]() {
    handleStartBoostMode();
  });
  server.on("/api/control/circulation_pump", HTTP_POST, [this]() {
    handleControlCirculationPump();
  });
  server.on("/api/control/stop", HTTP_POST, [this]() {
    handleStopHeater();
  });
  server.on("/api/control/toggle_logging", HTTP_POST, [this]() {
    handleToggleLogging();
  });

  // Endpoint-ы для тестирования компонентов
  setupTestEndpoints();

  server.onNotFound([this]() {
    handleNotFound();
  });
}

void ApiServer::setupTestEndpoints() {
  // Тесты компонентов
  server.on("/api/test/combustion_fan", HTTP_POST, [this]() {
    handleTestCombustionFan();
  });

  server.on("/api/test/fuel_pump", HTTP_POST, [this]() {
    handleTestFuelPump();
  });

  server.on("/api/test/glow_plug", HTTP_POST, [this]() {
    handleTestGlowPlug();
  });

  server.on("/api/test/circulation_pump", HTTP_POST, [this]() {
    handleTestCirculationPump();
  });

  server.on("/api/test/vehicle_fan", HTTP_POST, [this]() {
    handleTestVehicleFan();
  });

  server.on("/api/test/solenoid_valve", HTTP_POST, [this]() {
    handleTestSolenoidValve();
  });

  server.on("/api/test/fuel_preheating", HTTP_POST, [this]() {
    handleTestFuelPreheating();
  });
}

void ApiServer::serveHTML() {
  serveStaticFile("/index.html", "text/html");
}

void ApiServer::serveStaticFile(String path, String contentType) {
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    if (file) {
      server.streamFile(file, contentType);
      file.close();
      Serial.println("✅ Обслужен файл: " + path);
    } else {
      server.send(500, "application/json", "{\"error\":\"file_open_error\"}");
      Serial.println("❌ Ошибка открытия файла: " + path);
    }
  } else {
    serveFallbackHTML();
    Serial.println("⚠️  Файл не найден: " + path);
  }
}

void ApiServer::serveFallbackHTML()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Webasto W-Bus</title>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .card { background: white; padding: 20px; margin: 10px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        .btn { padding: 10px 15px; margin: 5px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; }
        .btn:hover { background: #0056b3; }
        .btn-success { background: #28a745; }
        .btn-success:hover { background: #218838; }
        .btn-warning { background: #ffc107; color: black; }
        .btn-warning:hover { background: #e0a800; }
        .btn-danger { background: #dc3545; }
        .btn-danger:hover { background: #c82333; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 15px; }
        .endpoint { background: #f8f9fa; padding: 10px; border-radius: 5px; margin: 5px 0; }
        .section { margin-bottom: 30px; }
    </style>
</head>
<body>
    <h1>🚗 Webasto W-Bus Диагностика</h1>
    
    <div class="card section">
        <h3>📊 Быстрый доступ к данным</h3>
        <div class="grid">
            <button class="btn" onclick="loadData('/api/system/state')">Состояние системы</button>
            <button class="btn" onclick="loadData('/api/device/info')">Информация устройства</button>
            <button class="btn" onclick="loadData('/api/sensors/data')">Данные сенсоров</button>
            <button class="btn" onclick="loadData('/api/errors')">Ошибки</button>
            <button class="btn" onclick="loadData('/api/all')">Все данные</button>
        </div>
    </div>

    <div class="card section">
        <h3>🎮 Управление режимами</h3>
        <div class="grid">
            <button class="btn" onclick="sendCommand('/api/control/connect')">Подключиться</button>
            <button class="btn" onclick="sendCommand('/api/control/disconnect')">Отключиться</button>
            <button class="btn btn-success" onclick="sendCommand('/api/control/start_parking')">Паркинг-нагрев</button>
            <button class="btn btn-success" onclick="sendCommand('/api/control/start_ventilation')">Вентиляция</button>
            <button class="btn btn-success" onclick="sendCommand('/api/control/start_supplemental')">Доп. нагрев</button>
            <button class="btn btn-warning" onclick="sendCommand('/api/control/start_boost')">Boost режим</button>
            <button class="btn" onclick="sendCommand('/api/control/circulation_pump?enable=true')">Вкл цирк. насос</button>
            <button class="btn" onclick="sendCommand('/api/control/circulation_pump?enable=false')">Выкл цирк. насос</button>
            <button class="btn btn-danger" onclick="sendCommand('/api/control/stop')">Остановить всё</button>
        </div>
    </div>

    <div class="card section">
        <h3>🔧 Тестирование компонентов</h3>
        <div class="grid">
            <button class="btn" onclick="testComponent('combustion_fan', 10, 50)">Вентилятор горения</button>
            <button class="btn" onclick="testComponent('fuel_pump', 5, 10)">Топливный насос</button>
            <button class="btn" onclick="testComponent('glow_plug', 15, 75)">Свеча накала</button>
            <button class="btn" onclick="testComponent('circulation_pump', 20, 100)">Циркуляционный насос</button>
            <button class="btn" onclick="testComponent('vehicle_fan', 8, 0)">Вентилятор авто</button>
            <button class="btn" onclick="testComponent('solenoid_valve', 12, 0)">Соленоидный клапан</button>
            <button class="btn" onclick="testComponent('fuel_preheating', 25, 50)">Подогрев топлива</button>
        </div>
    </div>

    <div id="data" class="card">
        <h3>📋 Результат</h3>
        <pre id="output">Нажмите кнопку для загрузки данных...</pre>
    </div>

    <div class="card">
        <h3>🔗 Доступные endpoint-ы</h3>
        <div class="endpoint"><strong>GET</strong> /api/system/state - Состояние системы</div>
        <div class="endpoint"><strong>GET</strong> /api/device/info - Информация устройства</div>
        <div class="endpoint"><strong>GET</strong> /api/sensors/data - Данные сенсоров</div>
        <div class="endpoint"><strong>GET</strong> /api/errors - Ошибки Webasto</div>
        <div class="endpoint"><strong>GET</strong> /api/all - Все данные</div>
        
        <div class="endpoint"><strong>POST</strong> /api/control/connect - Подключение к Webasto</div>
        <div class="endpoint"><strong>POST</strong> /api/control/disconnect - Отключение от Webasto</div>
        <div class="endpoint"><strong>POST</strong> /api/control/start_parking - Запуск паркинг-нагрева</div>
        <div class="endpoint"><strong>POST</strong> /api/control/start_ventilation - Запуск вентиляции</div>
        <div class="endpoint"><strong>POST</strong> /api/control/start_supplemental - Запуск доп. нагрева</div>
        <div class="endpoint"><strong>POST</strong> /api/control/start_boost - Запуск Boost режима</div>
        <div class="endpoint"><strong>POST</strong> /api/control/circulation_pump - Управление цирк. насосом</div>
        <div class="endpoint"><strong>POST</strong> /api/control/stop - Остановка нагревателя</div>
        
        <div class="endpoint"><strong>POST</strong> /api/test/combustion_fan - Тест вентилятора горения</div>
        <div class="endpoint"><strong>POST</strong> /api/test/fuel_pump - Тест топливного насоса</div>
        <div class="endpoint"><strong>POST</strong> /api/test/glow_plug - Тест свечи накала</div>
        <div class="endpoint"><strong>POST</strong> /api/test/circulation_pump - Тест циркуляционного насоса</div>
        <div class="endpoint"><strong>POST</strong> /api/test/vehicle_fan - Тест вентилятора автомобиля</div>
        <div class="endpoint"><strong>POST</strong> /api/test/solenoid_valve - Тест соленоидного клапана</div>
        <div class="endpoint"><strong>POST</strong> /api/test/fuel_preheating - Тест подогрева топлива</div>
    </div>
    
    <script>
        async function loadData(endpoint) {
            try {
                document.getElementById('output').textContent = 'Загрузка...';
                const response = await fetch(endpoint);
                const data = await response.json();
                document.getElementById('output').textContent = JSON.stringify(data, null, 2);
            } catch (error) {
                document.getElementById('output').textContent = 'Ошибка: ' + error;
            }
        }

        async function sendCommand(endpoint) {
            try {
                document.getElementById('output').textContent = 'Отправка команды...';
                const response = await fetch(endpoint, { method: 'POST' });
                const data = await response.json();
                document.getElementById('output').textContent = JSON.stringify(data, null, 2);
            } catch (error) {
                document.getElementById('output').textContent = 'Ошибка: ' + error;
            }
        }

        async function testComponent(component, seconds, value) {
            try {
                document.getElementById('output').textContent = 'Запуск теста...';
                let url = `/api/test/${component}?seconds=${seconds}`;
                
                if (component === 'fuel_pump') {
                    url += `&frequency=${value}`;
                } else if (component === 'vehicle_fan' || component === 'solenoid_valve') {
                    // Эти компоненты не требуют power параметра
                } else {
                    url += `&power=${value}`;
                }
                
                const response = await fetch(url, { method: 'POST' });
                const data = await response.json();
                document.getElementById('output').textContent = JSON.stringify(data, null, 2);
            } catch (error) {
                document.getElementById('output').textContent = 'Ошибка: ' + error;
            }
        }

        // Автоматическая загрузка состояния при открытии
        loadData('/api/system/state');
    </script>
</body>
</html>
    )rawliteral";

    server.send(200, "text/html", html);
}

void ApiServer::loop() {
  server.handleClient();
}

// =============================================================================
// HANDLERS ДЛЯ УПРАВЛЕНИЯ РЕЖИМАМИ
// =============================================================================

void ApiServer::handleConnect() {
  wBus.connect();
  server.send(200, "application/json", "{\"status\":\"connecting\",\"message\":\"Инициировано подключение к Webasto\"}");
}

void ApiServer::handleDisconnect() {
  wBus.disconnect();
  server.send(200, "application/json", "{\"status\":\"disconnecting\",\"message\":\"Инициировано отключение от Webasto\"}");
}

void ApiServer::handleStartParkingHeat() {
  int minutes = server.arg("minutes").toInt();
  if (minutes <= 0) minutes = 60; // Значение по умолчанию
  
  wBus.startParkingHeat(minutes);
  server.send(200, "application/json", 
    "{\"status\":\"started\",\"mode\":\"parking_heat\",\"message\":\"Паркинг-нагрев запущен\",\"duration_minutes\":" + String(minutes) + "}");
}

void ApiServer::handleStartVentilation() {
  int minutes = server.arg("minutes").toInt();
  if (minutes <= 0) minutes = 60; // Значение по умолчанию
  
  wBus.startVentilation(minutes);
  server.send(200, "application/json", 
    "{\"status\":\"started\",\"mode\":\"ventilation\",\"message\":\"Вентиляция запущена\",\"duration_minutes\":" + String(minutes) + "}");
}

void ApiServer::handleStartSupplementalHeat() {
  int minutes = server.arg("minutes").toInt();
  if (minutes <= 0) minutes = 60; // Значение по умолчанию
  
  wBus.startSupplementalHeat(minutes);
  server.send(200, "application/json", 
    "{\"status\":\"started\",\"mode\":\"supplemental_heat\",\"message\":\"Дополнительный нагрев запущен\",\"duration_minutes\":" + String(minutes) + "}");
}

void ApiServer::handleStartBoostMode() {
  int minutes = server.arg("minutes").toInt();
  if (minutes <= 0) minutes = 60; // Значение по умолчанию
  
  wBus.startBoostMode(minutes);
  server.send(200, "application/json", 
    "{\"status\":\"started\",\"mode\":\"boost\",\"message\":\"Boost режим запущен\",\"duration_minutes\":" + String(minutes) + "}");
}

void ApiServer::handleControlCirculationPump() {
  String enableStr = server.arg("enable");
  bool enable = (enableStr == "true" || enableStr == "1");
  
  wBus.controlCirculationPump(enable);
  
  String statusMessage = enable ? "включен" : "выключен";
  String enabledStr = enable ? "true" : "false";
  
  String response = "{\"status\":\"success\",\"message\":\"Циркуляционный насос " + statusMessage + "\",\"enabled\":" + enabledStr + "}";
  
  server.send(200, "application/json", response);
}

void ApiServer::handleStopHeater() {
  wBus.shutdown();
  server.send(200, "application/json", "{\"status\":\"stopped\",\"message\":\"Нагреватель остановлен\"}");
}

void ApiServer::handleToggleLogging() {
  if (wBus.isLogging()) {
    wBus.stopLogging();
    server.send(200, "application/json", "{\"status\":\"logging_disabled\",\"message\":\"Логирование отключено\"}");
  } else {
    wBus.startLogging();
    server.send(200, "application/json", "{\"status\":\"logging_enabled\",\"message\":\"Логирование включено\"}");
  }
}

// =============================================================================
// HANDLERS ДЛЯ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
// =============================================================================

void ApiServer::handleTestCombustionFan() {
  int seconds = server.arg("seconds").toInt();
  int power = server.arg("power").toInt();

  if (seconds > 0 && power >= 0 && power <= 100) {
    wBus.testCombustionFan(seconds, power);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"combustion_fan\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds must be positive, power 0-100\"}");
  }
}

void ApiServer::handleTestFuelPump() {
  int seconds = server.arg("seconds").toInt();
  int frequency = server.arg("frequency").toInt();

  if (seconds > 0 && frequency > 0) {
    wBus.testFuelPump(seconds, frequency);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"fuel_pump\",\"seconds\":" + String(seconds) + ",\"frequency\":" + String(frequency) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds and frequency must be positive\"}");
  }
}

void ApiServer::handleTestGlowPlug() {
  int seconds = server.arg("seconds").toInt();
  int power = server.arg("power").toInt();

  if (seconds > 0 && power >= 0 && power <= 100) {
    wBus.testGlowPlug(seconds, power);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"glow_plug\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds must be positive, power 0-100\"}");
  }
}

void ApiServer::handleTestCirculationPump() {
  int seconds = server.arg("seconds").toInt();
  int power = server.arg("power").toInt();

  if (seconds > 0 && power >= 0 && power <= 100) {
    wBus.testCirculationPump(seconds, power);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"circulation_pump\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds must be positive, power 0-100\"}");
  }
}

void ApiServer::handleTestVehicleFan() {
  int seconds = server.arg("seconds").toInt();

  if (seconds > 0) {
    wBus.testVehicleFan(seconds);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"vehicle_fan\",\"seconds\":" + String(seconds) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds must be positive\"}");
  }
}

void ApiServer::handleTestSolenoidValve() {
  int seconds = server.arg("seconds").toInt();

  if (seconds > 0) {
    wBus.testSolenoidValve(seconds);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"solenoid_valve\",\"seconds\":" + String(seconds) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds must be positive\"}");
  }
}

void ApiServer::handleTestFuelPreheating() {
  int seconds = server.arg("seconds").toInt();
  int power = server.arg("power").toInt();

  if (seconds > 0 && power >= 0 && power <= 100) {
    wBus.testFuelPreheating(seconds, power);
    server.send(200, "application/json", 
      "{\"status\":\"test_started\",\"component\":\"fuel_preheating\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
  } else {
    server.send(400, "application/json", "{\"error\":\"invalid_parameters\",\"message\":\"seconds must be positive, power 0-100\"}");
  }
}

// =============================================================================
// HANDLERS ДЛЯ ДАННЫХ (остаются без изменений)
// =============================================================================

void ApiServer::handleGetSystemState() {
  DynamicJsonDocument doc(1024);

  doc["connection_state"] = ConnectionStateNames[wBus.getConnectionState()];
  doc["heater_state"] = WebastoStateNames[wBus.getState()];
  doc["is_connected"] = wBus.isConnected();
  doc["is_logging"] = wBus.isLogging();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ApiServer::handleGetDeviceInfo() {
  // Создаем JSON с информацией об устройстве
  DynamicJsonDocument doc(4096);

  // Заполняем данными из WebastoInfo
  doc["wbus_version"] = webastoInfo.getWBusVersionData();
  doc["device_name"] = webastoInfo.getDeviceNameData();
  doc["device_id"] = webastoInfo.getDeviceIDData();
  doc["serial_number"] = webastoInfo.getSerialNumberData();
  doc["controller_manufacture_date"] = webastoInfo.getControllerManufactureDateData();
  doc["heater_manufacture_date"] = webastoInfo.getHeaterManufactureDateData();
  doc["customer_id"] = webastoInfo.getCustomerIDData();
  doc["wbus_code"] = webastoInfo.getWBusCodeData();
  doc["supported_functions"] = webastoInfo.getSupportedFunctionsData();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ApiServer::handleGetSensorsData() {
  DynamicJsonDocument doc(4096);

  doc["operational_measurements"] = serialized(webastoSensors.createJsonOperationalInfo());
  doc["fuel_settings"] = serialized(webastoSensors.createJsonFuelSettings());
  doc["on_off_flags"] = serialized(webastoSensors.createJsonOnOffFlags());
  doc["status_flags"] = serialized(webastoSensors.createJsonStatusFlags());
  doc["operating_state"] = serialized(webastoSensors.createJsonOperatingState());
  doc["subsystems_status"] = serialized(webastoSensors.createJsonSubsystemsStatus());

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ApiServer::handleGetErrors() {
  // Используем готовую JSON функцию из WebastoErrors
  String jsonResponse = webastoErrors.createJsonErrors();
  server.send(200, "application/json", jsonResponse);
}

void ApiServer::handleGetAllData() {
  DynamicJsonDocument doc(8192);

  // Системное состояние
  JsonObject systemState = doc.createNestedObject("system_state");
  systemState["connection_state"] = ConnectionStateNames[wBus.getConnectionState()];
  systemState["heater_state"] = wBus.getCurrentStateName();
  systemState["is_connected"] = wBus.isConnected();
  systemState["is_logging"] = wBus.isLogging();

  // Информация об устройстве
  JsonObject deviceInfo = doc.createNestedObject("device_info");
  deviceInfo["wbus_version"] = webastoInfo.getWBusVersionData();
  deviceInfo["device_name"] = webastoInfo.getDeviceNameData();
  deviceInfo["device_id"] = webastoInfo.getDeviceIDData();
  deviceInfo["serial_number"] = webastoInfo.getSerialNumberData();
  deviceInfo["controller_manufacture_date"] = webastoInfo.getControllerManufactureDateData();
  deviceInfo["heater_manufacture_date"] = webastoInfo.getHeaterManufactureDateData();
  deviceInfo["customer_id"] = webastoInfo.getCustomerIDData();
  deviceInfo["wbus_code"] = webastoInfo.getWBusCodeData();
  deviceInfo["supported_functions"] = webastoInfo.getSupportedFunctionsData();

  // Данные сенсоров
  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["operational_measurements"] = serialized(webastoSensors.createJsonOperationalInfo());
  sensors["fuel_settings"] = serialized(webastoSensors.createJsonFuelSettings());
  sensors["on_off_flags"] = serialized(webastoSensors.createJsonOnOffFlags());
  sensors["status_flags"] = serialized(webastoSensors.createJsonStatusFlags());
  sensors["operating_state"] = serialized(webastoSensors.createJsonOperatingState());
  sensors["subsystems_status"] = serialized(webastoSensors.createJsonSubsystemsStatus());

  // Ошибки
  doc["errors"] = serialized(webastoErrors.createJsonErrors());

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void ApiServer::handleNotFound() {
  DynamicJsonDocument doc(512);
  doc["error"] = "not_found";
  doc["uri"] = server.uri();
  doc["method"] = (server.method() == HTTP_GET) ? "GET" : "POST";
  doc["available_endpoints"] = "/api/system/state, /api/device/info, /api/sensors/data, /api/errors, /api/all";

  String response;
  serializeJson(doc, response);
  server.send(404, "application/json", response);
}

void ApiServer::printAvailableEndpoints() {
  Serial.println("📋 Available API endpoints:");
  Serial.println("  GET  /api/system/state        - System connection and heater state");
  Serial.println("  GET  /api/device/info         - Webasto device information");
  Serial.println("  GET  /api/sensors/data        - Complete sensors data");
  Serial.println("  GET  /api/errors              - Webasto error codes");
  Serial.println("  GET  /api/all                 - Combined all data");
  
  Serial.println("  POST /api/control/connect     - Connect to Webasto");
  Serial.println("  POST /api/control/disconnect  - Disconnect from Webasto");
  Serial.println("  POST /api/control/start_parking - Start parking heat (default 60min)");
  Serial.println("  POST /api/control/start_ventilation - Start ventilation (default 60min)");
  Serial.println("  POST /api/control/start_supplemental - Start supplemental heat (default 60min)");
  Serial.println("  POST /api/control/start_boost - Start boost mode (default 60min)");
  Serial.println("  POST /api/control/circulation_pump - Control circulation pump (enable=true/false)");
  Serial.println("  POST /api/control/stop        - Stop heater");
  Serial.println("  POST /api/control/toggle_logging - Toggle WebSocket logging");
  
  Serial.println("  POST /api/test/combustion_fan - Test combustion fan (seconds, power)");
  Serial.println("  POST /api/test/fuel_pump      - Test fuel pump (seconds, frequency)");
  Serial.println("  POST /api/test/glow_plug      - Test glow plug (seconds, power)");
  Serial.println("  POST /api/test/circulation_pump - Test circulation pump (seconds, power)");
  Serial.println("  POST /api/test/vehicle_fan    - Test vehicle fan (seconds)");
  Serial.println("  POST /api/test/solenoid_valve - Test solenoid valve (seconds)");
  Serial.println("  POST /api/test/fuel_preheating - Test fuel preheating (seconds, power)");
  
  Serial.println("");
  Serial.println("🌐 Web interface available at: http://" + WiFi.softAPIP().toString());
}