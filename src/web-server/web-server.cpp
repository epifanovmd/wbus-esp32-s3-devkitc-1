#include "web-server.h"
#include "common/constants.h"

WebastoWebServer webServer;

WebastoWebServer::WebastoWebServer()
    : server(80),
      webSocket(81),
      lastBroadcast(0),
      broadcastInterval(2000)
{
}

void WebastoWebServer::begin()
{
    webPage = generateHTML();

    server.on("/", [this]()
              { this->handleRoot(); });
    server.on("/data", [this]()
              { this->handleData(); });
    server.on("/deviceinfo", [this]()
              { this->handleDeviceInfo(); });

    server.begin();

    webSocket.begin();
    webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      { this->handleWebSocket(num, type, payload, length); });

    Serial.println("✅ Веб-сервер запущен");
    Serial.println("   HTTP: http://" + WiFi.softAPIP().toString());
    Serial.println("   WebSocket: порт 81");
}

void WebastoWebServer::handleClient()
{
    server.handleClient();
    webSocket.loop();

    if (millis() - lastBroadcast >= broadcastInterval)
    {
        sendSensorData();
        lastBroadcast = millis();
    }
}

void WebastoWebServer::handleRoot()
{
    server.send(200, "text/html", webPage);
}

void WebastoWebServer::handleData()
{
    String jsonData = "{";
    jsonData += "\"temperature\":" + String(webastoSensors.getCurrentMeasurements().temperature) + ",";
    jsonData += "\"voltage\":" + String(webastoSensors.getCurrentMeasurements().voltage) + ",";
    jsonData += "\"heatingPower\":" + String(webastoSensors.getCurrentMeasurements().heatingPower) + ",";
    jsonData += "\"flameDetected\":" + String(webastoSensors.getCurrentMeasurements().flameDetected ? "true" : "false");
    jsonData += "}";

    server.send(200, "application/json", jsonData);
}

void WebastoWebServer::handleDeviceInfo()
{
    WebastoDeviceInfo deviceInfo = webastoInfo.getDeviceInfo();

    String jsonData = "{";
    jsonData += "\"wbusVersion\":\"" + deviceInfo.wbusVersion + "\",";
    jsonData += "\"deviceName\":\"" + deviceInfo.deviceName + "\",";
    jsonData += "\"deviceID\":\"" + deviceInfo.deviceID + "\",";
    jsonData += "\"serialNumber\":\"" + deviceInfo.serialNumber + "\",";
    jsonData += "\"controllerManufactureDate\":\"" + deviceInfo.controllerManufactureDate + "\",";
    jsonData += "\"heaterManufactureDate\":\"" + deviceInfo.heaterManufactureDate + "\",";
    jsonData += "\"customerID\":\"" + deviceInfo.customerID + "\",";
    jsonData += "\"wbusCode\":\"" + deviceInfo.wbusCode + "\"";
    jsonData += "}";

    server.send(200, "application/json", jsonData);
}

void WebastoWebServer::handleWebSocket(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Отключен\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Подключен из %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        sendSensorData();
    }
    break;
    case WStype_TEXT:
    {
        String message = String((char *)payload);
        Serial.printf("[%u] Получено: %s\n", num, message.c_str());

        if (message == "getData")
        {
            sendSensorData();
        }
        else if (message == "getStatus")
        {
            webastoSensors.getStatusFlags();
        }
        else if (message == "getOperatingState")
        {
            webastoSensors.getOperatingState();
        }
        else if (message == "getDeviceInfo")
        {
            sendDeviceInfo();
        }
        else if (message == "getAllInfo")
        {
            webastoInfo.getAllInfo();
            // Отправляем информацию об устройстве после запроса
            delay(1000);
            sendDeviceInfo();
        }
    }
    break;
    }
}

void WebastoWebServer::sendSensorData()
{
    if (webSocket.connectedClients() > 0)
    {
        OperationalMeasurements measurements = webastoSensors.getCurrentMeasurements();
        StatusFlags status = webastoSensors.getStatusFlagsData();
        OperatingState state = webastoSensors.getOperatingStateData();

        String jsonData = "{";
        jsonData += "\"type\":\"sensorData\",";
        jsonData += "\"temperature\":" + String(measurements.temperature, 1) + ",";
        jsonData += "\"voltage\":" + String(measurements.voltage, 1) + ",";
        jsonData += "\"heatingPower\":" + String(measurements.heatingPower) + ",";
        jsonData += "\"flameDetected\":" + String(measurements.flameDetected ? "true" : "false") + ",";
        jsonData += "\"flameResistance\":" + String(measurements.flameResistance) + ",";
        jsonData += "\"operationMode\":\"" + status.operationMode + "\",";
        jsonData += "\"stateName\":\"" + state.stateName + "\",";
        jsonData += "\"stateCode\":" + String(state.stateCode) + ",";
        jsonData += "\"timestamp\":" + String(millis());
        jsonData += "}";

        webSocket.broadcastTXT(jsonData);
    }
}

String WebastoWebServer::escapeJSON(String input) {
    input.replace("\\", "\\\\");
    input.replace("\"", "\\\"");
    input.replace("\n", "\\n");
    input.replace("\r", "\\r");
    input.replace("\t", "\\t");
    input.replace("/", "\\/");
    return input;
}

void WebastoWebServer::sendDeviceInfo()
{
    if (webSocket.connectedClients() > 0)
    {
        WebastoDeviceInfo deviceInfo = webastoInfo.getDeviceInfo();

        String jsonData = "{";
        jsonData += "\"type\":\"deviceInfo\",";
        jsonData += "\"wbusVersion\":\"" + deviceInfo.wbusVersion + "\",";
        jsonData += "\"deviceName\":\"" + deviceInfo.deviceName + "\",";
        jsonData += "\"deviceID\":\"" + deviceInfo.deviceID + "\",";
        jsonData += "\"serialNumber\":\"" + deviceInfo.serialNumber + "\",";
        jsonData += "\"testStandCode\":\"" + deviceInfo.testStandCode + "\",";
        jsonData += "\"controllerManufactureDate\":\"" + deviceInfo.controllerManufactureDate + "\",";
        jsonData += "\"heaterManufactureDate\":\"" + deviceInfo.heaterManufactureDate + "\",";
        jsonData += "\"customerID\":\"" + deviceInfo.customerID + "\",";
        jsonData += "\"wbusCode\":\"" + deviceInfo.wbusCode + "\",";
        jsonData += "\"supportedFunctions\":\"" + escapeJSON(deviceInfo.supportedFunctions) + "\",";
        jsonData += "\"hasData\":" + String(deviceInfo.hasData() ? "true" : "false");
        jsonData += "}";

        webSocket.broadcastTXT(jsonData);
    }
}

// Вспомогательная функция для экранирования JSON
String escapeJSON(String input)
{
    input.replace("\\", "\\\\");
    input.replace("\"", "\\\"");
    input.replace("\n", "\\n");
    input.replace("\r", "\\r");
    input.replace("\t", "\\t");
    return input;
}

void WebastoWebServer::broadcastData(String data)
{
    webSocket.broadcastTXT(data);
}

String WebastoWebServer::generateHTML()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Webasto Monitor</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            min-height: 100vh;
            padding: 15px;
            overflow-x: hidden;
        }
        
        .container {
            max-width: 100%;
            margin: 0 auto;
        }
        
        .header {
            text-align: center;
            margin-bottom: 20px;
            color: white;
            padding: 10px;
        }
        
        .header h1 {
            font-size: 1.8em;
            margin-bottom: 8px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
            font-weight: 700;
        }
        
        .header .subtitle {
            font-size: 1em;
            opacity: 0.9;
            font-weight: 400;
        }
        
        .tabs {
            display: flex;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 12px;
            padding: 5px;
            margin-bottom: 15px;
            backdrop-filter: blur(10px);
        }
        
        .tab {
            flex: 1;
            padding: 12px 8px;
            text-align: center;
            background: transparent;
            border: none;
            color: white;
            font-weight: 600;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-size: 0.9em;
        }
        
        .tab.active {
            background: rgba(255, 255, 255, 0.2);
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        
        .tab-content {
            display: none;
        }
        
        .tab-content.active {
            display: block;
            animation: fadeIn 0.5s ease;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: 1fr;
            gap: 15px;
            margin-bottom: 15px;
        }
        
        @media (min-width: 768px) {
            .dashboard {
                grid-template-columns: repeat(2, 1fr);
            }
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
            transition: all 0.3s ease;
        }
        
        .card:active {
            transform: scale(0.98);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
            border-bottom: 1px solid #f0f0f0;
            padding-bottom: 12px;
        }
        
        .card-icon {
            font-size: 1.5em;
            margin-right: 12px;
            min-width: 30px;
        }
        
        .card-title {
            font-size: 1.1em;
            font-weight: 600;
            color: #2c3e50;
        }
        
        .value {
            font-size: 1.8em;
            font-weight: bold;
            color: #2c3e50;
            margin-bottom: 4px;
            line-height: 1.2;
        }
        
        .info-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 8px 0;
            border-bottom: 1px solid #f8f9fa;
        }
        
        .info-row:last-child {
            border-bottom: none;
        }
        
        .info-label {
            font-weight: 600;
            color: #2c3e50;
            font-size: 0.9em;
        }
        
        .info-value {
            color: #7f8c8d;
            font-size: 0.9em;
            text-align: right;
            max-width: 60%;
            word-break: break-word;
        }
        
        .status-badge {
            display: inline-block;
            padding: 6px 12px;
            border-radius: 15px;
            font-size: 0.75em;
            font-weight: bold;
            margin-top: 8px;
            text-align: center;
        }
        
        .status-on {
            background: #27ae60;
            color: white;
        }
        
        .status-off {
            background: #e74c3c;
            color: white;
        }
        
        .status-warning {
            background: #f39c12;
            color: white;
        }
        
        .status-info {
            background: #3498db;
            color: white;
        }
        
        .controls {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 10px;
            margin-top: 15px;
        }
        
        .btn {
            padding: 14px 8px;
            border: none;
            border-radius: 8px;
            font-size: 0.85em;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-align: center;
            min-height: 44px;
        }
        
        .btn:active {
            transform: scale(0.95);
        }
        
        .btn-primary {
            background: #3498db;
            color: white;
        }
        
        .btn-success {
            background: #27ae60;
            color: white;
        }
        
        .btn-warning {
            background: #f39c12;
            color: white;
        }
        
        .btn-danger {
            background: #e74c3c;
            color: white;
        }
        
        .btn-info {
            background: #9b59b6;
            color: white;
        }
        
        .connection-status {
            position: fixed;
            top: 10px;
            right: 10px;
            padding: 8px 12px;
            border-radius: 15px;
            font-weight: bold;
            z-index: 1000;
            font-size: 0.8em;
            backdrop-filter: blur(10px);
        }
        
        .connected {
            background: rgba(39, 174, 96, 0.9);
            color: white;
        }
        
        .disconnected {
            background: rgba(231, 76, 60, 0.9);
            color: white;
        }
        
        .connecting {
            background: rgba(241, 196, 15, 0.9);
            color: white;
        }
        
        .connection-info {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 12px;
            padding: 15px;
            margin-bottom: 15px;
            text-align: center;
            backdrop-filter: blur(10px);
        }
        
        .last-update {
            text-align: center;
            color: rgba(255, 255, 255, 0.8);
            font-size: 0.8em;
            margin-top: 15px;
            padding: 10px;
        }
        
        .no-data {
            text-align: center;
            color: #7f8c8d;
            padding: 20px;
            font-style: italic;
        }
        
        .function-list {
            font-size: 0.85em;
            line-height: 1.4;
            color: #2c3e50;
        }
        
        .function-item {
            margin-bottom: 8px;
            padding-left: 15px;
            position: relative;
        }
        
        .function-item:before {
            content: "•";
            position: absolute;
            left: 0;
            color: #3498db;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚗 Webasto Monitor</h1>
            <div class="subtitle">Прямое подключение к нагревателю</div>
        </div>
        
        <div class="connection-status" id="connectionStatus">
            Подключение...
        </div>
        
        <div class="connection-info">
            <h3>📱 Подключено к ESP32</h3>
            <p>Сеть: WebastoMonitor | IP: 192.168.4.1</p>
        </div>
        
        <!-- Табы для переключения между разделами -->
        <div class="tabs">
            <button class="tab active" onclick="switchTab('monitor')">Мониторинг</button>
            <button class="tab" onclick="switchTab('device')">Инфо устройство</button>
            <button class="tab" onclick="switchTab('system')">Система</button>
        </div>
        
        <!-- Вкладка мониторинга -->
        <div id="monitor" class="tab-content active">
            <div class="dashboard">
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">🌡️</div>
                        <div class="card-title">Температура</div>
                    </div>
                    <div class="value" id="temperature">--</div>
                    <div class="label">Градусы Цельсия</div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">🔋</div>
                        <div class="card-title">Напряжение</div>
                    </div>
                    <div class="value" id="voltage">--</div>
                    <div class="label">Вольты</div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">⚡</div>
                        <div class="card-title">Мощность</div>
                    </div>
                    <div class="value" id="heatingPower">--</div>
                    <div class="label">Ватты</div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">🔥</div>
                        <div class="card-title">Пламя</div>
                    </div>
                    <div class="value" id="flameStatus">--</div>
                    <div class="label">Статус горения</div>
                    <div class="status-badge" id="flameBadge">--</div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">🎛️</div>
                        <div class="card-title">Режим работы</div>
                    </div>
                    <div class="value" id="operationMode">--</div>
                    <div class="label">Текущий режим</div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">🔄</div>
                        <div class="card-title">Состояние системы</div>
                    </div>
                    <div class="value" id="systemState">--</div>
                    <div class="label">Код состояния</div>
                    <div id="stateDescription" style="margin-top: 8px; font-size: 0.8em; color: #7f8c8d;"></div>
                </div>
            </div>
            
            <div class="controls">
                <button class="btn btn-primary" onclick="sendCommand('getData')">Обновить</button>
                <button class="btn btn-success" onclick="sendCommand('getStatus')">Статус</button>
                <button class="btn btn-warning" onclick="sendCommand('getOperatingState')">Состояние</button>
                <button class="btn btn-info" onclick="sendCommand('getDeviceInfo')">Инфо устройство</button>
            </div>
        </div>
        
        <!-- Вкладка информации об устройстве -->
        <div id="device" class="tab-content">
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">📋</div>
                    <div class="card-title">Информация об устройстве</div>
                </div>
                <div id="deviceInfoContent">
                    <div class="no-data">Нажмите "Загрузить информацию" для получения данных</div>
                </div>
                <div class="controls" style="margin-top: 20px;">
                    <button class="btn btn-info" onclick="sendCommand('getAllInfo')">Загрузить информацию</button>
                    <button class="btn btn-primary" onclick="sendCommand('getDeviceInfo')">Обновить</button>
                </div>
            </div>
        </div>
        
        <!-- Вкладка системной информации -->
        <div id="system" class="tab-content">
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">⚙️</div>
                    <div class="card-title">Системное управление</div>
                </div>
                <div class="controls">
                    <button class="btn btn-primary" onclick="sendCommand('getData')">Данные датчиков</button>
                    <button class="btn btn-success" onclick="sendCommand('getStatus')">Статус флаги</button>
                    <button class="btn btn-warning" onclick="sendCommand('getOperatingState')">Состояние работы</button>
                    <button class="btn btn-danger" onclick="sendCommand('getErrors')">Проверить ошибки</button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">🔧</div>
                    <div class="card-title">Дополнительные команды</div>
                </div>
                <div class="controls">
                    <button class="btn btn-info" onclick="sendCommand('getFuelSettings')">Настройки топлива</button>
                    <button class="btn btn-info" onclick="sendCommand('getOnOffFlags')">Флаги подсистем</button>
                </div>
            </div>
        </div>
        
        <div class="last-update" id="lastUpdate">
            Ожидание данных...
        </div>
    </div>

    <script>
        let ws;
        let isConnected = false;
        let reconnectAttempts = 0;
        const maxReconnectAttempts = 10;
        
        function switchTab(tabName) {
            // Скрыть все вкладки
            document.querySelectorAll('.tab-content').forEach(tab => {
                tab.classList.remove('active');
            });
            
            // Убрать активный класс со всех кнопок
            document.querySelectorAll('.tab').forEach(btn => {
                btn.classList.remove('active');
            });
            
            // Показать выбранную вкладку
            document.getElementById(tabName).classList.add('active');
            
            // Активировать соответствующую кнопку
            event.target.classList.add('active');
            
            // Если переключились на вкладку устройства, загружаем информацию
            if (tabName === 'device') {
                sendCommand('getDeviceInfo');
            }
        }
        
        function connectWebSocket() {
            const wsUrl = 'ws://' + window.location.hostname + ':81';
            
            updateConnectionStatus('connecting', 'Подключение...');
            
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                console.log('WebSocket подключен');
                isConnected = true;
                reconnectAttempts = 0;
                updateConnectionStatus('connected', '✅ Онлайн');
                sendCommand('getData');
            };
            
            ws.onclose = function() {
                console.log('WebSocket отключен');
                isConnected = false;
                
                if (reconnectAttempts < maxReconnectAttempts) {
                    reconnectAttempts++;
                    const delay = Math.min(1000 * reconnectAttempts, 10000);
                    updateConnectionStatus('disconnected', `❌ Переподключение (${reconnectAttempts}/${maxReconnectAttempts})...`);
                    setTimeout(connectWebSocket, delay);
                } else {
                    updateConnectionStatus('disconnected', '❌ Офлайн');
                }
            };
            
            ws.onerror = function(error) {
                console.error('WebSocket ошибка:', error);
                isConnected = false;
                updateConnectionStatus('disconnected', '❌ Ошибка подключения');
            };
            
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    if (data.type === 'sensorData') {
                        updateSensorData(data);
                    } else if (data.type === 'deviceInfo') {
                        updateDeviceInfo(data);
                    }
                } catch (error) {
                    console.error('Ошибка парсинга JSON:', error);
                }
            };
        }
        
        function updateConnectionStatus(status, message) {
            const statusElement = document.getElementById('connectionStatus');
            statusElement.textContent = message;
            statusElement.className = 'connection-status ' + status;
        }
        
        function updateSensorData(data) {
            // Температура
            if (data.temperature !== undefined) {
                document.getElementById('temperature').textContent = data.temperature.toFixed(1);
            }
            
            // Напряжение
            if (data.voltage !== undefined) {
                document.getElementById('voltage').textContent = data.voltage.toFixed(1);
            }
            
            // Мощность
            if (data.heatingPower !== undefined) {
                document.getElementById('heatingPower').textContent = data.heatingPower;
            }
            
            // Пламя
            if (data.flameDetected !== undefined) {
                const flameStatus = document.getElementById('flameStatus');
                const flameBadge = document.getElementById('flameBadge');
                
                if (data.flameDetected) {
                    flameStatus.textContent = 'Активно';
                    flameBadge.textContent = 'ГОРЕНИЕ';
                    flameBadge.className = 'status-badge status-on';
                } else {
                    flameStatus.textContent = 'Неактивно';
                    flameBadge.textContent = 'НЕТ ПЛАМЕНИ';
                    flameBadge.className = 'status-badge status-off';
                }
            }
            
            // Режим работы
            if (data.operationMode) {
                document.getElementById('operationMode').textContent = data.operationMode;
            }
            
            // Состояние системы
            if (data.stateName) {
                document.getElementById('systemState').textContent = data.stateName;
                document.getElementById('stateDescription').textContent = 
                    'Код: 0x' + data.stateCode.toString(16).toUpperCase();
            }
            
            // Время обновления
            document.getElementById('lastUpdate').textContent = 
                'Обновлено: ' + new Date().toLocaleTimeString();
        }
        
        function updateDeviceInfo(data) {
            const content = document.getElementById('deviceInfoContent');
            
            if (!data.hasData) {
                content.innerHTML = '<div class="no-data">Данные об устройстве не загружены. Нажмите "Загрузить информацию"</div>';
                return;
            }
            
            let html = '<div class="info-container">';
            
            // Основная информация
            html += '<div class="info-row"><span class="info-label">Версия W-Bus:</span><span class="info-value">' + (data.wbusVersion || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">Имя устройства:</span><span class="info-value">' + (data.deviceName || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">ID устройства:</span><span class="info-value">' + (data.deviceID || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">Серийный номер:</span><span class="info-value">' + (data.serialNumber || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">Код стенда:</span><span class="info-value">' + (data.testStandCode || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">Дата пр-ва контроллера:</span><span class="info-value">' + (data.controllerManufactureDate || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">Дата пр-ва нагревателя:</span><span class="info-value">' + (data.heaterManufactureDate || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">ID клиента:</span><span class="info-value">' + (data.customerID || '--') + '</span></div>';
            html += '<div class="info-row"><span class="info-label">W-Bus код:</span><span class="info-value">' + (data.wbusCode || '--') + '</span></div>';
            
            // Поддерживаемые функции
            if (data.supportedFunctions) {
                html += '<div class="info-row" style="align-items: flex-start; margin-top: 15px;">';
                html += '<span class="info-label">Поддерживаемые функции:</span>';
                html += '<div class="info-value function-list">' + data.supportedFunctions.replace(/\\n/g, '<br>') + '</div>';
                html += '</div>';
            }
            
            html += '</div>';
            content.innerHTML = html;
        }
        
        function sendCommand(command) {
            if (isConnected && ws) {
                ws.send(command);
                
                // Визуальная обратная связь
                if (event && event.target) {
                    event.target.classList.add('active');
                    setTimeout(() => {
                        event.target.classList.remove('active');
                    }, 300);
                }
            } else {
                alert('Нет подключения к Webasto!');
            }
        }
        
        // Автоматическое обновление данных
        setInterval(() => {
            if (isConnected) {
                sendCommand('getData');
            }
        }, 3000);
        
        // Инициализация при загрузке страницы
        window.onload = function() {
            connectWebSocket();
            
            // Добавляем обработчики для кнопок
            document.querySelectorAll('.btn').forEach(btn => {
                btn.addEventListener('touchstart', function() {
                    this.classList.add('active');
                });
                
                btn.addEventListener('touchend', function() {
                    this.classList.remove('active');
                });
            });
        };
    </script>
</body>
</html>
)rawliteral";

    return html;
}