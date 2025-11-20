#include "api-server.h"
#include <ArduinoJson.h>
#include "wbus/wbus.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-errors.h"

ApiServer apiServer;

#define FS LittleFS

ApiServer::ApiServer() : server(80) {}

void ApiServer::begin()
{
    // Инициализация файловой системы
    initializeFileSystem();

    // Настройка endpoint-ов
    setupEndpoints();

    server.begin();
    Serial.println();
    Serial.print("✅ HTTP server started on port 80");
    printAvailableEndpoints();
}

void ApiServer::initializeFileSystem()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("❌ Ошибка инициализации LittleFS");
        return;
    }

    Serial.println();
    Serial.print("✅ LittleFS инициализирован");
    listFilesystemContents();
}

void ApiServer::listFilesystemContents()
{
    Serial.println();
    Serial.print("📁 Содержимое LittleFS:");
    File root = FS.open("/");
    if (!root)
    {
        Serial.println("   ❌ Не удалось открыть корневую директорию");
        return;
    }

    if (!root.isDirectory())
    {
        Serial.println("   ❌ Корень не является директорией");
        root.close();
        return;
    }

    File file = root.openNextFile();
    int fileCount = 0;

    Serial.println();
    while (file)
    {
        fileCount++;
        Serial.println("   📄 " + String(file.name()) + " | Размер: " + String(file.size()) + " байт");
        file = root.openNextFile();
    }
    root.close();

    if (fileCount == 0)
    {
        Serial.println("   ℹ️  Файловая система пуста");
    }
}

void ApiServer::setupEndpoints()
{
    // Статические файлы
    server.on("/", HTTP_GET, [this]()
              { serveHTML(); });

    // API endpoint-ы для данных
    server.on("/api/system/state", HTTP_GET, [this]()
              { handleGetSystemState(); });
    server.on("/api/device/info", HTTP_GET, [this]()
              { handleGetDeviceInfo(); });
    server.on("/api/sensors/data", HTTP_GET, [this]()
              { handleGetSensorsData(); });
    server.on("/api/errors", HTTP_GET, [this]()
              { handleGetErrors(); });
    server.on("/api/all", HTTP_GET, [this]()
              { handleGetAllData(); });

    // API endpoint-ы для управления режимами
    server.on("/api/control/connect", HTTP_POST, [this]()
              { handleConnect(); });
    server.on("/api/control/disconnect", HTTP_POST, [this]()
              { handleDisconnect(); });
    server.on("/api/control/start_parking", HTTP_POST, [this]()
              { handleStartParkingHeat(); });
    server.on("/api/control/start_ventilation", HTTP_POST, [this]()
              { handleStartVentilation(); });
    server.on("/api/control/start_supplemental", HTTP_POST, [this]()
              { handleStartSupplementalHeat(); });
    server.on("/api/control/start_boost", HTTP_POST, [this]()
              { handleStartBoostMode(); });
    server.on("/api/control/circulation_pump", HTTP_POST, [this]()
              { handleControlCirculationPump(); });
    server.on("/api/control/stop", HTTP_POST, [this]()
              { handleStopHeater(); });
    server.on("/api/control/toggle_logging", HTTP_POST, [this]()
              { handleToggleLogging(); });

    // Endpoint-ы для тестирования компонентов
    setupTestEndpoints();

    server.onNotFound([this]()
                      { handleNotFound(); });
}

void ApiServer::setupTestEndpoints()
{
    // Тесты компонентов
    server.on("/api/test/combustion_fan", HTTP_POST, [this]()
              { handleTestCombustionFan(); });

    server.on("/api/test/fuel_pump", HTTP_POST, [this]()
              { handleTestFuelPump(); });

    server.on("/api/test/glow_plug", HTTP_POST, [this]()
              { handleTestGlowPlug(); });

    server.on("/api/test/circulation_pump", HTTP_POST, [this]()
              { handleTestCirculationPump(); });

    server.on("/api/test/vehicle_fan", HTTP_POST, [this]()
              { handleTestVehicleFan(); });

    server.on("/api/test/solenoid_valve", HTTP_POST, [this]()
              { handleTestSolenoidValve(); });

    server.on("/api/test/fuel_preheating", HTTP_POST, [this]()
              { handleTestFuelPreheating(); });
}

void ApiServer::serveHTML()
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
        .status-success { color: #28a745; font-weight: bold; }
        .status-error { color: #dc3545; font-weight: bold; }
        .status-warning { color: #ffc107; font-weight: bold; }
        .params { background: #f8f9fa; padding: 10px; border-radius: 5px; margin: 10px 0; }
        .message { padding: 10px; border-radius: 5px; margin: 10px 0; }
        .message-success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .message-error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .message-info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }
        
        /* Стили для сводной информации */
        .summary-grid { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); 
            gap: 15px; 
            margin-bottom: 20px;
        }
        .summary-card { 
            background: white; 
            padding: 15px; 
            border-radius: 8px; 
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        .summary-value { 
            font-size: 24px; 
            font-weight: bold; 
            margin: 10px 0;
        }
        .summary-label { 
            font-size: 14px; 
            color: #666; 
            text-transform: uppercase;
        }
        .status-connected { color: #28a745; }
        .status-disconnected { color: #dc3545; }
        .status-connecting { color: #ffc107; }
        
        /* Стили для компонентов */
        .components-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 10px;
            margin-top: 15px;
        }
        .component {
            padding: 8px;
            border-radius: 5px;
            text-align: center;
            font-size: 12px;
        }
        .component-active { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .component-inactive { background: #f8f9fa; color: #6c757d; border: 1px solid #dee2e6; }
        
        /* Стили для WebSocket статуса */
        .websocket-status {
            position: fixed;
            top: 10px;
            right: 10px;
            padding: 8px 12px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
        }
        .websocket-connected { background: #d4edda; color: #155724; }
        .websocket-disconnected { background: #f8d7da; color: #721c24; }
        .websocket-connecting { background: #fff3cd; color: #856404; }
    </style>
</head>
<body>
    <!-- WebSocket статус -->
    <div id="websocketStatus" class="websocket-status websocket-disconnected">
        🔴 WebSocket: Отключен
    </div>

    <h1>🚗 Webasto W-Bus Диагностика</h1>
    
    <!-- Сводная информация -->
    <div class="card section">
        <h3>📊 Сводная информация (Real-time)</h3>
        <div class="summary-grid">
            <div class="summary-card">
                <div class="summary-label">Статус подключения</div>
                <div id="connection-status" class="summary-value status-disconnected">❌ Отключен</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Режим работы</div>
                <div id="operation-mode" class="summary-value">💤 Ожидание</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Температура</div>
                <div id="temperature" class="summary-value">-- °C</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Напряжение</div>
                <div id="voltage" class="summary-value">-- V</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Мощность</div>
                <div id="heating-power" class="summary-value">-- W</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Пламя</div>
                <div id="flame-status" class="summary-value">--</div>
            </div>
        </div>

        <!-- Активные компоненты -->
        <div style="margin-top: 20px;">
            <div class="summary-label" style="text-align: left; margin-bottom: 10px;">🔧 Активные компоненты:</div>
            <div class="components-grid" id="active-components">
                <!-- Компоненты будут заполняться через WebSocket -->
            </div>
        </div>
    </div>

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
            <button class="btn btn-success" onclick="sendCommand('/api/control/start_parking?minutes=60')">Паркинг-нагрев (60min)</button>
            <button class="btn btn-success" onclick="sendCommand('/api/control/start_ventilation?minutes=60')">Вентиляция (60min)</button>
            <button class="btn btn-success" onclick="sendCommand('/api/control/start_supplemental?minutes=60')">Доп. нагрев (60min)</button>
            <button class="btn btn-warning" onclick="sendCommand('/api/control/start_boost?minutes=60')">Boost режим (60min)</button>
            <button class="btn" onclick="sendCommand('/api/control/circulation_pump?enable=true')">Вкл цирк. насос</button>
            <button class="btn" onclick="sendCommand('/api/control/circulation_pump?enable=false')">Выкл цирк. насос</button>
            <button class="btn btn-danger" onclick="sendCommand('/api/control/stop')">Остановить всё</button>
            <button class="btn" onclick="sendCommand('/api/control/toggle_logging')">Переключить логирование</button>
        </div>
    </div>

    <div class="card section">
        <h3>🔧 Тестирование компонентов</h3>
        <div class="grid">
            <button class="btn" onclick="testComponent('combustion_fan', 10, 50)">Вентилятор горения</button>
            <button class="btn" onclick="testComponent('fuel_pump', 5, 10)">Топливный насос</button>
            <button class="btn" onclick="testComponent('glow_plug', 15, 75)">Свеча накала</button>
            <button class="btn" onclick="testComponent('circulation_pump', 20, 100)">Циркуляционный насос</button>
            <button class="btn" onclick="testComponent('vehicle_fan', 8)">Вентилятор авто</button>
            <button class="btn" onclick="testComponent('solenoid_valve', 12)">Соленоидный клапан</button>
            <button class="btn" onclick="testComponent('fuel_preheating', 25, 50)">Подогрев топлива</button>
        </div>
    </div>

    <div id="data" class="card">
        <h3>📋 Результат</h3>
        <div id="result-container">
            <div id="status-indicator"></div>
            <div id="params-container"></div>
            <div id="message-container"></div>
            <pre id="output">Нажмите кнопку для загрузки данных...</pre>
        </div>
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
        <div class="endpoint"><strong>POST</strong> /api/control/toggle_logging - Переключение логирования</div>
        
        <div class="endpoint"><strong>POST</strong> /api/test/combustion_fan - Тест вентилятора горения</div>
        <div class="endpoint"><strong>POST</strong> /api/test/fuel_pump - Тест топливного насоса</div>
        <div class="endpoint"><strong>POST</strong> /api/test/glow_plug - Тест свечи накала</div>
        <div class="endpoint"><strong>POST</strong> /api/test/circulation_pump - Тест циркуляционного насоса</div>
        <div class="endpoint"><strong>POST</strong> /api/test/vehicle_fan - Тест вентилятора автомобиля</div>
        <div class="endpoint"><strong>POST</strong> /api/test/solenoid_valve - Тест соленоидного клапана</div>
        <div class="endpoint"><strong>POST</strong> /api/test/fuel_preheating - Тест подогрева топлива</div>
    </div>
    
    <script>
        // WebSocket подключение
        let socket = null;
        let reconnectAttempts = 0;
        const maxReconnectAttempts = 5;

        function connectWebSocket() {
            try {
                const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
                const wsUrl = `${protocol}//${window.location.hostname}:81`;
                
                socket = new WebSocket(wsUrl);
                
                socket.onopen = function(event) {
                    console.log('✅ WebSocket подключен');
                    updateWebSocketStatus('connected', '🟢 WebSocket: Подключен');
                    reconnectAttempts = 0;
                };
                
                socket.onmessage = function(event) {
                    try {
                        const data = JSON.parse(event.data);
                        handleWebSocketMessage(data);
                    } catch (error) {
                        console.error('Ошибка парсинга WebSocket сообщения:', error);
                    }
                };
                
                socket.onclose = function(event) {
                    console.log('❌ WebSocket отключен:', event.code, event.reason);
                    updateWebSocketStatus('disconnected', '🔴 WebSocket: Отключен');
                    
                    // Попытка переподключения
                    if (reconnectAttempts < maxReconnectAttempts) {
                        reconnectAttempts++;
                        const delay = Math.min(1000 * reconnectAttempts, 10000);
                        console.log(`🔄 Попытка переподключения ${reconnectAttempts}/${maxReconnectAttempts} через ${delay}ms`);
                        updateWebSocketStatus('connecting', `🟡 WebSocket: Переподключение... (${reconnectAttempts}/${maxReconnectAttempts})`);
                        setTimeout(connectWebSocket, delay);
                    }
                };
                
                socket.onerror = function(error) {
                    console.error('❌ WebSocket ошибка:', error);
                    updateWebSocketStatus('error', '🔴 WebSocket: Ошибка подключения');
                };
                
            } catch (error) {
                console.error('❌ Ошибка создания WebSocket:', error);
            }
        }

        function handleWebSocketMessage(data) {
            switch(data.type) {
                case 'operational_measurements':
                    updateOperationalData(JSON.parse(data.data));
                    break;
                case 'status_flags':
                    updateStatusFlags(JSON.parse(data.data));
                    break;
                case 'on_off_flags':
                    updateOnOffFlags(JSON.parse(data.data));
                    break;
                case 'operating_state':
                    updateOperatingState(JSON.parse(data.data));
                    break;
                case 'subsystems_status':
                    updateSubsystemsStatus(JSON.parse(data.data));
                    break;
                case 'errors':
                    updateErrors(JSON.parse(data.data));
                    break;
                case 'system_status':
                    updateSystemStatus(data.data);
                    break;
                case 'tx':
                case 'rx':
                    // Логирование коммуникации (опционально)
                    break;
                case 'connection_state':
                    updateConnectionState(data.data);
                    break;
                case 'state':
                    updateHeaterState(data.data);
                    break;
            }
        }

        function updateOperationalData(data) {
            if (data.temperature !== undefined) {
                document.getElementById('temperature').textContent = `${data.temperature.toFixed(1)} °C`;
            }
            if (data.voltage !== undefined) {
                document.getElementById('voltage').textContent = `${data.voltage.toFixed(1)} V`;
            }
            if (data.heating_power !== undefined) {
                document.getElementById('heating-power').textContent = `${data.heating_power} W`;
            }
            if (data.flame_detected !== undefined) {
                const flameStatus = data.flame_detected ? '🔥 Обнаружено' : '❌ Нет';
                document.getElementById('flame-status').textContent = flameStatus;
            }
        }

        function updateStatusFlags(data) {
            if (data.operation_mode) {
                document.getElementById('operation-mode').textContent = data.operation_mode;
            }
        }

        function updateOnOffFlags(data) {
            const componentsContainer = document.getElementById('active-components');
            componentsContainer.innerHTML = '';
            
            const components = [
                { id: 'combustion_fan', name: 'Вентилятор', active: data.combustion_air_fan },
                { id: 'glow_plug', name: 'Свеча', active: data.glow_plug },
                { id: 'fuel_pump', name: 'Топл. насос', active: data.fuel_pump },
                { id: 'circulation_pump', name: 'Цирк. насос', active: data.circulation_pump },
                { id: 'vehicle_fan', name: 'Вент. авто', active: data.vehicle_fan_relay },
                { id: 'nozzle_heating', name: 'Подогрев', active: data.nozzle_stock_heating },
                { id: 'flame_indicator', name: 'Пламя', active: data.flame_indicator }
            ];
            
            components.forEach(component => {
                if (component.active) {
                    const element = document.createElement('div');
                    element.className = 'component component-active';
                    element.textContent = component.name;
                    componentsContainer.appendChild(element);
                }
            });
            
            // Если нет активных компонентов
            if (componentsContainer.children.length === 0) {
                const element = document.createElement('div');
                element.className = 'component component-inactive';
                element.textContent = 'Нет активных';
                componentsContainer.appendChild(element);
            }
        }

        function updateConnectionState(data) {
            const statusElement = document.getElementById('connection-status');
            const statusMap = {
                'DISCONNECTED': { text: '❌ Отключен', class: 'status-disconnected' },
                'CONNECTING': { text: '🟡 Подключение...', class: 'status-connecting' },
                'CONNECTED': { text: '✅ Подключен', class: 'status-connected' },
                'CONNECTION_FAILED': { text: '🔴 Ошибка', class: 'status-error' }
            };
            
            if (statusMap[data]) {
                statusElement.textContent = statusMap[data].text;
                statusElement.className = `summary-value ${statusMap[data].class}`;
            }
        }

        function updateHeaterState(data) {
            // Обновление состояния нагревателя если нужно
        }

        function updateWebSocketStatus(status, text) {
            const element = document.getElementById('websocketStatus');
            element.textContent = text;
            element.className = `websocket-status websocket-${status}`;
        }

        // API функции
        async function loadData(endpoint) {
            try {
                showLoading();
                const response = await fetch(endpoint);
                const data = await response.json();
                displayResult(data, 'data');
            } catch (error) {
                displayError(error);
            }
        }

        async function sendCommand(endpoint) {
            try {
                showLoading();
                const response = await fetch(endpoint, { method: 'POST' });
                const data = await response.json();
                displayResult(data, 'command');
            } catch (error) {
                displayError(error);
            }
        }

        async function testComponent(component, seconds, value) {
            try {
                showLoading();
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
                displayResult(data, 'test');
            } catch (error) {
                displayError(error);
            }
        }

        function displayResult(data, type) {
            const output = document.getElementById('output');
            const statusIndicator = document.getElementById('status-indicator');
            const paramsContainer = document.getElementById('params-container');
            const messageContainer = document.getElementById('message-container');
            
            // Очищаем предыдущие результаты
            statusIndicator.innerHTML = '';
            paramsContainer.innerHTML = '';
            messageContainer.innerHTML = '';
            
            // Отображаем статус
            if (data.success !== undefined) {
                const statusText = data.success ? '✅ УСПЕШНО' : '❌ ОШИБКА';
                const statusClass = data.success ? 'status-success' : 'status-error';
                statusIndicator.innerHTML = `<div class="${statusClass}">${statusText}</div>`;
            }
            
            // Отображаем параметры
            if (data.params && Object.keys(data.params).length > 0) {
                paramsContainer.innerHTML = `
                    <div class="params">
                        <strong>📋 Параметры:</strong>
                        <pre>${JSON.stringify(data.params, null, 2)}</pre>
                    </div>
                `;
            }
            
            // Отображаем сообщение
            if (data.message) {
                const messageClass = data.success ? 'message-success' : 
                                   data.error ? 'message-error' : 'message-info';
                messageContainer.innerHTML = `
                    <div class="message ${messageClass}">
                        <strong>💬 Сообщение:</strong> ${data.message}
                    </div>
                `;
            }
            
            // Отображаем ошибку
            if (data.error) {
                messageContainer.innerHTML += `
                    <div class="message message-error">
                        <strong>🚨 Ошибка:</strong> ${data.error}
                    </div>
                `;
            }
            
            // Отображаем полные данные
            output.textContent = JSON.stringify(data, null, 2);
        }

        function displayError(error) {
            const output = document.getElementById('output');
            const statusIndicator = document.getElementById('status-indicator');
            const messageContainer = document.getElementById('message-container');
            
            statusIndicator.innerHTML = '<div class="status-error">❌ ОШИБКА СЕТИ</div>';
            messageContainer.innerHTML = `
                <div class="message message-error">
                    <strong>🚨 Ошибка сети:</strong> ${error.message}
                </div>
            `;
            output.textContent = `Ошибка: ${error}`;
        }

        function showLoading() {
            const output = document.getElementById('output');
            const statusIndicator = document.getElementById('status-indicator');
            const paramsContainer = document.getElementById('params-container');
            const messageContainer = document.getElementById('message-container');
            
            statusIndicator.innerHTML = '<div class="status-warning">⏳ ЗАГРУЗКА...</div>';
            paramsContainer.innerHTML = '';
            messageContainer.innerHTML = '';
            output.textContent = 'Обработка запроса...';
        }

        // Инициализация при загрузке страницы
        document.addEventListener('DOMContentLoaded', function() {
            // Подключаемся к WebSocket
            connectWebSocket();
            
            // Загружаем начальное состояние
            loadData('/api/system/state');
            
            // Периодическое обновление данных (fallback)
            setInterval(() => {
                if (socket && socket.readyState === WebSocket.OPEN) {
                    // WebSocket активен, данные приходят в реальном времени
                } else {
                    // Fallback: периодический опрос API
                    loadData('/api/system/state');
                }
            }, 30000); // Каждые 30 секунд
        });
    </script>
</body>
</html>
    )rawliteral";

    server.send(200, "text/html", html);

    // serveStaticFile("/index.html", "text/html");
}

void ApiServer::serveStaticFile(String path, String contentType)
{
    if (LittleFS.exists(path))
    {
        File file = LittleFS.open(path, "r");
        if (file)
        {
            server.streamFile(file, contentType);
            file.close();
            Serial.println();
            Serial.print("✅ Обслужен файл: " + path);
        }
        else
        {
            server.send(500, "application/json", "{\"error\":\"file_open_error\"}");
            Serial.println();
            Serial.print("❌ Ошибка открытия файла: " + path);
        }
    }
    else
    {
        // serveFallbackHTML();
        Serial.println("⚠️  Файл не найден: " + path);
    }
}

void ApiServer::loop()
{
    server.handleClient();
}

// =============================================================================
// HANDLERS ДЛЯ УПРАВЛЕНИЯ РЕЖИМАМИ
// =============================================================================

void ApiServer::handleConnect()
{
    wBus.connect([this](String tx, String rx)
                 {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized("{}");
    
    if (!rx.isEmpty()) {
      doc["message"] = "Успешное подключение к Webasto";
    } else {
      doc["error"] = "Ошибка подключения к Webasto";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleDisconnect()
{
    wBus.disconnect();

    DynamicJsonDocument doc(512);

    doc["success"] = true; // disconnect всегда успешен
    doc["params"] = serialized("{}");
    doc["message"] = "Отключение от Webasto выполнено";

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void ApiServer::handleStartParkingHeat()
{
    int minutes = server.arg("minutes").toInt();
    if (minutes <= 0)
        minutes = 60;

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["minutes"] = minutes;
    paramsDoc["mode"] = "parking_heat";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.startParkingHeat(minutes, [this, paramsStr](String tx, String rx)
                          {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Паркинг-нагрев успешно запущен";
    } else {
      doc["error"] = "Ошибка запуска паркинг-нагрева";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleStartVentilation()
{
    int minutes = server.arg("minutes").toInt();
    if (minutes <= 0)
        minutes = 60;

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["minutes"] = minutes;
    paramsDoc["mode"] = "ventilation";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.startVentilation(minutes, [this, paramsStr](String tx, String rx)
                          {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Вентиляция успешно запущена";
    } else {
      doc["error"] = "Ошибка запуска вентиляции";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleStartSupplementalHeat()
{
    int minutes = server.arg("minutes").toInt();
    if (minutes <= 0)
        minutes = 60;

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["minutes"] = minutes;
    paramsDoc["mode"] = "supplemental_heat";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.startSupplementalHeat(minutes, [this, paramsStr](String tx, String rx)
                               {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Дополнительный нагрев успешно запущен";
    } else {
      doc["error"] = "Ошибка запуска дополнительного нагрева";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleStartBoostMode()
{
    int minutes = server.arg("minutes").toInt();
    if (minutes <= 0)
        minutes = 60;

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["minutes"] = minutes;
    paramsDoc["mode"] = "boost";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.startBoostMode(minutes, [this, paramsStr](String tx, String rx)
                        {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Boost режим успешно запущен";
    } else {
      doc["error"] = "Ошибка запуска Boost режима";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleControlCirculationPump()
{
    String enableStr = server.arg("enable");
    bool enable = (enableStr == "true" || enableStr == "1");

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["enable"] = enable;
    paramsDoc["mode"] = "circulation_pump_control";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.controlCirculationPump(enable, [this, paramsStr](String tx, String rx)
                                {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      String status = doc["params"]["enable"] ? "включен" : "выключен";
      doc["message"] = "Циркуляционный насос " + status;
    } else {
      doc["error"] = "Ошибка управления циркуляционным насосом";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleStopHeater()
{
    wBus.shutdown([this](String tx, String rx)
                  {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized("{\"mode\":\"shutdown\"}");
    
    if (!rx.isEmpty()) {
      doc["message"] = "Нагреватель успешно остановлен";
    } else {
      doc["error"] = "Ошибка остановки нагревателя";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleToggleLogging()
{
    if (wBus.isLogging())
    {
        wBus.stopLogging();
        server.send(200, "application/json", "{\"status\":\"logging_disabled\",\"message\":\"Логирование отключено\"}");
    }
    else
    {
        wBus.startLogging();
        server.send(200, "application/json", "{\"status\":\"logging_enabled\",\"message\":\"Логирование включено\"}");
    }
}

// =============================================================================
// HANDLERS ДЛЯ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
// =============================================================================

void ApiServer::handleTestCombustionFan()
{
    int seconds = server.arg("seconds").toInt();
    int power = server.arg("power").toInt();

    if (seconds <= 0 || power < 0 || power > 100)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds must be positive, power 0-100";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["power"] = power;
    paramsDoc["component"] = "combustion_fan";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testCombustionFan(seconds, power, [this, paramsStr](String tx, String rx)
                           {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест вентилятора горения запущен";
    } else {
      doc["error"] = "Ошибка теста вентилятора горения";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleTestFuelPump()
{
    int seconds = server.arg("seconds").toInt();
    int frequency = server.arg("frequency").toInt();

    if (seconds <= 0 || frequency <= 0)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds and frequency must be positive";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["frequency"] = frequency;
    paramsDoc["component"] = "fuel_pump";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testFuelPump(seconds, frequency, [this, paramsStr](String tx, String rx)
                      {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест топливного насоса запущен";
    } else {
      doc["error"] = "Ошибка теста топливного насоса";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleTestGlowPlug()
{
    int seconds = server.arg("seconds").toInt();
    int power = server.arg("power").toInt();

    if (seconds <= 0 || power < 0 || power > 100)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds must be positive, power 0-100";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["power"] = power;
    paramsDoc["component"] = "glow_plug";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testGlowPlug(seconds, power, [this, paramsStr](String tx, String rx)
                      {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест свечи накала запущен";
    } else {
      doc["error"] = "Ошибка теста свечи накала";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleTestCirculationPump()
{
    int seconds = server.arg("seconds").toInt();
    int power = server.arg("power").toInt();

    if (seconds <= 0 || power < 0 || power > 100)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds must be positive, power 0-100";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["power"] = power;
    paramsDoc["component"] = "circulation_pump";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testCirculationPump(seconds, power, [this, paramsStr](String tx, String rx)
                             {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест циркуляционного насоса запущен";
    } else {
      doc["error"] = "Ошибка теста циркуляционного насоса";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleTestVehicleFan()
{
    int seconds = server.arg("seconds").toInt();

    if (seconds <= 0)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds must be positive";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["component"] = "vehicle_fan";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testVehicleFan(seconds, [this, paramsStr](String tx, String rx)
                        {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест вентилятора автомобиля запущен";
    } else {
      doc["error"] = "Ошибка теста вентилятора автомобиля";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleTestSolenoidValve()
{
    int seconds = server.arg("seconds").toInt();

    if (seconds <= 0)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds must be positive";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["component"] = "solenoid_valve";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testSolenoidValve(seconds, [this, paramsStr](String tx, String rx)
                           {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест соленоидного клапана запущен";
    } else {
      doc["error"] = "Ошибка теста соленоидного клапана";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

void ApiServer::handleTestFuelPreheating()
{
    int seconds = server.arg("seconds").toInt();
    int power = server.arg("power").toInt();

    if (seconds <= 0 || power < 0 || power > 100)
    {
        DynamicJsonDocument doc(512);
        doc["success"] = false;
        doc["params"] = serialized("{}");
        doc["error"] = "invalid_parameters";
        doc["message"] = "seconds must be positive, power 0-100";

        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
        return;
    }

    DynamicJsonDocument paramsDoc(128);
    paramsDoc["seconds"] = seconds;
    paramsDoc["power"] = power;
    paramsDoc["component"] = "fuel_preheating";

    String paramsStr;
    serializeJson(paramsDoc, paramsStr);

    wBus.testFuelPreheating(seconds, power, [this, paramsStr](String tx, String rx)
                            {
    DynamicJsonDocument doc(512);
    
    doc["success"] = !rx.isEmpty();
    doc["params"] = serialized(paramsStr);
    
    if (!rx.isEmpty()) {
      doc["message"] = "Тест подогрева топлива запущен";
    } else {
      doc["error"] = "Ошибка теста подогрева топлива";
      doc["message"] = "Не получен ответ от устройства";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); });
}

// =============================================================================
// HANDLERS ДЛЯ ДАННЫХ (остаются без изменений)
// =============================================================================

void ApiServer::handleGetSystemState()
{
    DynamicJsonDocument doc(1024);

    doc["connection_state"] = ConnectionStateNames[wBus.getConnectionState()];
    doc["heater_state"] = WebastoStateNames[wBus.getState()];
    doc["is_connected"] = wBus.isConnected();
    doc["is_logging"] = wBus.isLogging();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void ApiServer::handleGetDeviceInfo()
{
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

void ApiServer::handleGetSensorsData()
{
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

void ApiServer::handleGetErrors()
{
    // Используем готовую JSON функцию из WebastoErrors
    String jsonResponse = webastoErrors.createJsonErrors();
    server.send(200, "application/json", jsonResponse);
}

void ApiServer::handleGetAllData()
{
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

void ApiServer::handleNotFound()
{
    DynamicJsonDocument doc(512);
    doc["error"] = "not_found";
    doc["uri"] = server.uri();
    doc["method"] = (server.method() == HTTP_GET) ? "GET" : "POST";
    doc["available_endpoints"] = "/api/system/state, /api/device/info, /api/sensors/data, /api/errors, /api/all";

    String response;
    serializeJson(doc, response);
    server.send(404, "application/json", response);
}

void ApiServer::printAvailableEndpoints()
{
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