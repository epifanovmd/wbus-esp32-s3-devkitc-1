#include "api-server.h"

ApiServer apiServer;

ApiServer::ApiServer() : server(80) {}

void ApiServer::begin()
{
    server.on("/", [this]()
              { handleRoot(); });

    server.onNotFound([this]()
                      { handleNotFound(); });

    server.begin();
    Serial.println("HTTP server started on port 80");
}

void ApiServer::loop()
{
    server.handleClient();
}

void ApiServer::handleRoot()
{
    server.send(200, "text/html", getHTMLPage());
}

void ApiServer::handleNotFound()
{
    server.send(404, "text/plain", "File Not Found");
}

String ApiServer::getHTMLPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Webasto W-Bus Monitor</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.1);
            overflow: hidden;
        }
        
        .header {
            background: #2c3e50;
            color: white;
            padding: 20px 30px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .header h1 {
            font-size: 24px;
            font-weight: 300;
        }
        
        .controls {
            background: #34495e;
            padding: 15px 30px;
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
        }
        
        .control-group {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        
        button {
            background: #3498db;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            transition: background 0.3s;
        }
        
        button:hover {
            background: #2980b9;
        }
        
        button.clear {
            background: #e74c3c;
        }
        
        button.clear:hover {
            background: #c0392b;
        }
        
        input, select {
            padding: 8px 12px;
            border: 1px solid #bdc3c7;
            border-radius: 5px;
            font-size: 14px;
        }
        
        .stats {
            background: #ecf0f1;
            padding: 10px 30px;
            display: flex;
            gap: 30px;
            font-size: 14px;
            color: #2c3e50;
        }
        
        .stat-item {
            display: flex;
            align-items: center;
            gap: 5px;
        }
        
        .log-container {
            display: flex;
            height: 600px;
        }
        
        .log-list {
            flex: 1;
            overflow-y: auto;
            border-right: 1px solid #ecf0f1;
        }
        
        .log-details {
            flex: 1;
            padding: 20px;
            background: #f8f9fa;
            overflow-y: auto;
        }
        
        .log-entry {
            padding: 12px 15px;
            border-bottom: 1px solid #ecf0f1;
            cursor: pointer;
            transition: background 0.2s;
        }
        
        .log-entry:hover {
            background: #f8f9fa;
        }
        
        .log-entry.rx {
            border-left: 4px solid #27ae60;
        }
        
        .log-entry.tx {
            border-left: 4px solid #e67e22;
        }
        
        .log-entry.info {
            border-left: 4px solid #3498db;
        }
        
        .log-entry.error {
            border-left: 4px solid #e74c3c;
        }
        
        .log-time {
            font-size: 12px;
            color: #7f8c8d;
            margin-bottom: 5px;
        }
        
        .log-data {
            font-family: 'Courier New', monospace;
            font-size: 13px;
            word-break: break-all;
        }
        
        .log-type {
            display: inline-block;
            padding: 2px 8px;
            border-radius: 3px;
            font-size: 11px;
            font-weight: bold;
            margin-right: 8px;
        }
        
        .type-rx { background: #d5f4e6; color: #27ae60; }
        .type-tx { background: #fdebd0; color: #e67e22; }
        .type-info { background: #d6eaf8; color: #3498db; }
        .type-error { background: #fadbd8; color: #e74c3c; }
        
        .selected {
            background: #e3f2fd !important;
        }
        
        .details-content {
            font-family: 'Courier New', monospace;
            font-size: 14px;
            white-space: pre-wrap;
            background: white;
            padding: 15px;
            border-radius: 5px;
            border: 1px solid #bdc3c7;
        }
        
        .search-highlight {
            background: yellow;
            padding: 2px;
        }
        
        .connection-status {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: #e74c3c;
        }
        
        .status-dot.connected {
            background: #27ae60;
        }
        
        @media (max-width: 768px) {
            .log-container {
                flex-direction: column;
                height: 800px;
            }
            
            .log-list {
                border-right: none;
                border-bottom: 1px solid #ecf0f1;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚗 Webasto W-Bus Protocol Monitor</h1>
            <div class="connection-status">
                <div class="status-dot" id="statusDot"></div>
                <span id="statusText">Disconnected</span>
            </div>
        </div>
        
        <div class="controls">
            <div class="control-group">
                <button onclick="clearLogs()">Clear Logs</button>
                <button onclick="exportLogs()">Export Logs</button>
            </div>
            
            <div class="control-group">
                <input type="text" id="searchInput" placeholder="Search in logs..." onkeyup="filterLogs()">
                <select id="typeFilter" onchange="filterLogs()">
                    <option value="all">All Types</option>
                    <option value="rx">RX Only</option>
                    <option value="tx">TX Only</option>
                    <option value="info">Info Only</option>
                    <option value="error">Errors Only</option>
                </select>
            </div>
            
            <div class="control-group">
                <label>
                    <input type="checkbox" id="autoScroll" checked> Auto-scroll
                </label>
                <label>
                    <input type="checkbox" id="showHex" checked> Show HEX
                </label>
            </div>
        </div>
        
        <div class="stats">
            <div class="stat-item">
                <span>RX:</span>
                <span id="rxCount">0</span>
            </div>
            <div class="stat-item">
                <span>TX:</span>
                <span id="txCount">0</span>
            </div>
            <div class="stat-item">
                <span>Total:</span>
                <span id="totalCount">0</span>
            </div>
            <div class="stat-item">
                <span>Connected:</span>
                <span id="connectedTime">0s</span>
            </div>
        </div>
        
        <div class="log-container">
            <div class="log-list" id="logList"></div>
            <div class="log-details">
                <h3>Details</h3>
                <div id="logDetails" class="details-content">
                    Select a log entry to view details...
                </div>
            </div>
        </div>
    </div>

    <script>
        class WebastoMonitor {
            constructor() {
                this.ws = null;
                this.logs = [];
                this.stats = { rx: 0, tx: 0, total: 0 };
                this.connectedAt = null;
                this.selectedLog = null;
                this.searchTerm = '';
                this.typeFilter = 'all';
                
                this.initWebSocket();
                this.startStatsTimer();
            }
            
            initWebSocket() {
                const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
                const wsUrl = `${protocol}//${window.location.hostname}:81`;
                
                this.ws = new WebSocket(wsUrl);
                
                this.ws.onopen = () => {
                    this.updateConnectionStatus(true);
                    console.log('WebSocket connected');
                };
                
                this.ws.onclose = () => {
                    this.updateConnectionStatus(false);
                    console.log('WebSocket disconnected');
                    // Попытка переподключения через 3 секунды
                    setTimeout(() => this.initWebSocket(), 3000);
                };
                
                this.ws.onmessage = (event) => {
                    this.handleMessage(JSON.parse(event.data));
                };
                
                this.ws.onerror = (error) => {
                    console.error('WebSocket error:', error);
                };
            }
            
            handleMessage(data) {
                if (data.type === 'history') {
                    this.logs = data.data.reverse();
                    this.updateDisplay();
                } else if (data.type === 'clear') {
                    this.logs = [];
                    this.stats = { rx: 0, tx: 0, total: 0 };
                    this.updateDisplay();
                } else {
                    this.addLog(data);
                }
            }
            
            addLog(log) {
                this.logs.unshift(log);
                
                // Обновляем статистику
                if (log.type === 'rx') this.stats.rx++;
                if (log.type === 'tx') this.stats.tx++;
                this.stats.total++;
                
                this.updateDisplay();
                
                // Авто-скролл
                if (document.getElementById('autoScroll').checked) {
                    const logList = document.getElementById('logList');
                    logList.scrollTop = 0;
                }
            }
            
            updateDisplay() {
                this.updateStats();
                this.renderLogs();
            }
            
            updateStats() {
                document.getElementById('rxCount').textContent = this.stats.rx;
                document.getElementById('txCount').textContent = this.stats.tx;
                document.getElementById('totalCount').textContent = this.stats.total;
                
                if (this.connectedAt) {
                    const seconds = Math.floor((Date.now() - this.connectedAt) / 1000);
                    document.getElementById('connectedTime').textContent = `${seconds}s`;
                }
            }
            
            renderLogs() {
                const logList = document.getElementById('logList');
                const filteredLogs = this.getFilteredLogs();
                
                logList.innerHTML = filteredLogs.map((log, index) => {
                    const isSelected = this.selectedLog === index ? 'selected' : '';
                    const displayData = this.formatData(log.data);
                    
                    return `
                        <div class="log-entry ${log.type} ${isSelected}" onclick="monitor.selectLog(${index})">
                            <div class="log-time">
                                <span class="log-type type-${log.type}">${log.type.toUpperCase()}</span>
                                ${log.timestamp}
                            </div>
                            <div class="log-data">${displayData}</div>
                        </div>
                    `;
                }).join('');
            }
            
            getFilteredLogs() {
                return this.logs.filter(log => {
                    const typeMatch = this.typeFilter === 'all' || log.type === this.typeFilter;
                    const searchMatch = !this.searchTerm || 
                                      log.data.toLowerCase().includes(this.searchTerm.toLowerCase()) ||
                                      log.timestamp.includes(this.searchTerm);
                    return typeMatch && searchMatch;
                });
            }
            
            formatData(data) {
                if (!document.getElementById('showHex').checked) {
                    return data;
                }
                
                // Форматирование HEX данных для лучшего отображения
                return data.split(' ').map(byte => {
                    return `<span class="hex-byte">${byte}</span>`;
                }).join(' ');
            }
            
            selectLog(index) {
                this.selectedLog = index;
                const log = this.getFilteredLogs()[index];
                
                if (log) {
                    document.getElementById('logDetails').textContent = 
                        `Timestamp: ${log.timestamp}\n` +
                        `Type: ${log.type}\n` +
                        `Direction: ${log.direction || 'N/A'}\n` +
                        `Data: ${log.data}\n\n` +
                        `Raw: ${JSON.stringify(log, null, 2)}`;
                }
                
                this.renderLogs();
            }
            
            filterLogs() {
                this.searchTerm = document.getElementById('searchInput').value;
                this.typeFilter = document.getElementById('typeFilter').value;
                this.selectedLog = null;
                this.renderLogs();
            }
            
            clearLogs() {
                this.ws.send(JSON.stringify({ command: 'clear' }));
            }
            
            exportLogs() {
                const data = this.logs.map(log => 
                    `${log.timestamp} [${log.type}] ${log.data}`
                ).join('\n');
                
                const blob = new Blob([data], { type: 'text/plain' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `webasto-logs-${new Date().toISOString().replace(/[:.]/g, '-')}.txt`;
                a.click();
                URL.revokeObjectURL(url);
            }
            
            updateConnectionStatus(connected) {
                const dot = document.getElementById('statusDot');
                const text = document.getElementById('statusText');
                
                if (connected) {
                    dot.className = 'status-dot connected';
                    text.textContent = 'Connected';
                    this.connectedAt = Date.now();
                } else {
                    dot.className = 'status-dot';
                    text.textContent = 'Disconnected';
                }
            }
            
            startStatsTimer() {
                setInterval(() => this.updateStats(), 1000);
            }
        }
        
        // Инициализация монитора
        const monitor = new WebastoMonitor();
        
        // Глобальные функции для кнопок
        function clearLogs() {
            monitor.clearLogs();
        }
        
        function exportLogs() {
            monitor.exportLogs();
        }
        
        function filterLogs() {
            monitor.filterLogs();
        }
    </script>
</body>
</html>
)rawliteral";

    return html;
}