// src/data/script.js
class WebastoController {
    constructor() {
        this.ws = null;
        this.isConnected = false;
        this.currentStatus = {
            connection: 'DISCONNECTED',
            heater: 'OFF'
        };
        
        this.components = {
            'combustionAirFan': { name: '🌀 Combustion Fan', active: false },
            'glowPlug': { name: '🔌 Glow Plug', active: false },
            'fuelPump': { name: '⛽ Fuel Pump', active: false },
            'circulationPump': { name: '💧 Circulation Pump', active: false },
            'vehicleFanRelay': { name: '🌬️ Vehicle Fan', active: false },
            'nozzleStockHeating': { name: '🔥 Nozzle Heating', active: false },
            'flameIndicator': { name: '📛 Flame Indicator', active: false }
        };

        this.init();
    }

    init() {
        this.bindEvents();
        this.connectWebSocket();
        this.loadInitialData();
    }

    bindEvents() {
        // Connection management
        document.getElementById('connectBtn').addEventListener('click', () => this.connect());
        document.getElementById('disconnectBtn').addEventListener('click', () => this.disconnect());
        document.getElementById('refreshBtn').addEventListener('click', () => this.refreshStatus());

        // Operating modes
        document.querySelectorAll('.mode-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const mode = e.target.dataset.mode;
                this.startMode(mode);
            });
        });

        // Circulation pump control
        document.getElementById('pumpOnBtn').addEventListener('click', () => this.controlPump(true));
        document.getElementById('pumpOffBtn').addEventListener('click', () => this.controlPump(false));
        document.getElementById('shutdownBtn').addEventListener('click', () => this.shutdown());

        // Device info
        document.getElementById('refreshDeviceInfo').addEventListener('click', () => this.refreshDeviceInfo());
        document.getElementById('refreshSensors').addEventListener('click', () => this.refreshSensors());

        // Error management
        document.getElementById('checkErrorsBtn').addEventListener('click', () => this.checkErrors());
        document.getElementById('clearErrorsBtn').addEventListener('click', () => this.clearErrors());

        // Component testing
        document.querySelectorAll('.btn-test').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const component = e.target.dataset.component;
                this.testComponent(component);
            });
        });

        // Messages
        document.getElementById('clearMessages').addEventListener('click', () => this.clearMessages());
    }

    connectWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}:81`;
        
        this.ws = new WebSocket(wsUrl);
        
        this.ws.onopen = () => {
            this.isConnected = true;
            this.updateWebSocketStatus('Connected', 'status-connected');
            this.addMessage('WebSocket connected', 'system');
        };
        
        this.ws.onclose = () => {
            this.isConnected = false;
            this.updateWebSocketStatus('Disconnected', 'status-disconnected');
            setTimeout(() => this.connectWebSocket(), 3000);
        };
        
        this.ws.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.updateWebSocketStatus('Error', 'status-disconnected');
        };
        
        this.ws.onmessage = (event) => {
            this.handleWebSocketMessage(JSON.parse(event.data));
        };
    }

    handleWebSocketMessage(message) {
        const { type, data } = message;
        
        switch (type) {
            case 'CONNECTION_STATE_CHANGED':
                this.updateConnectionStatus(data);
                break;
            case 'HEATER_STATE_CHANGED':
                this.updateHeaterStatus(data);
                break;
            case 'SENSOR_OPERATIONAL_INFO':
                this.updateSensorData(data);
                break;
            case 'SENSOR_ON_OFF_FLAGS':
                this.updateComponentStatus(data);
                break;
            case 'WBUS_ERRORS':
                this.updateErrors(data);
                break;
            case 'TX_RECEIVED':
                this.addMessage(`TX: ${data.rx}`, 'tx');
                break;
            case 'RX_RECEIVED':
                this.addMessage(`RX: ${data.rx}`, 'rx');
                break;
            case 'COMMAND_SENT':
                this.addMessage(`CMD: ${data.tx}`, 'tx');
                break;
            case 'COMMAND_RECEIVED':
                this.addMessage(`RESP: ${data.rx}`, 'rx');
                break;
            case 'COMMAND_SENT_ERRROR':
                this.addMessage(`ERROR: ${data.tx}`, 'error');
                break;
        }
    }

    updateConnectionStatus(data) {
        this.currentStatus.connection = data.newState;
        const statusElement = document.getElementById('connectionValue');
        statusElement.textContent = data.newState;
        statusElement.className = 'status-value ' + 
            (data.newState === 'CONNECTED' ? 'status-connected' : 
             data.newState === 'CONNECTING' ? 'status-waiting' : 'status-disconnected');
    }

    updateHeaterStatus(data) {
        this.currentStatus.heater = data.newState;
        document.getElementById('heaterValue').textContent = data.newState;
    }

    updateWebSocketStatus(status, className) {
        const element = document.getElementById('websocketValue');
        element.textContent = status;
        element.className = 'status-value ' + className;
    }

    updateSensorData(data) {
        document.getElementById('temperature').textContent = `${data.temperature} °C`;
        document.getElementById('voltage').textContent = `${data.voltage} V`;
        document.getElementById('heatingPower').textContent = `${data.heatingPower} W`;
        document.getElementById('flameStatus').textContent = data.flameDetected ? '🔥 Detected' : '❌ No flame';
    }

    updateComponentStatus(data) {
        this.components.combustionAirFan.active = data.combustion_air_fan;
        this.components.glowPlug.active = data.glow_plug;
        this.components.fuelPump.active = data.fuel_pump;
        this.components.circulationPump.active = data.circulation_pump;
        this.components.vehicleFanRelay.active = data.vehicle_fan_relay;
        this.components.nozzleStockHeating.active = data.nozzle_stock_heating;
        this.components.flameIndicator.active = data.flame_indicator;
        
        this.renderComponents();
    }

    renderComponents() {
        const grid = document.getElementById('componentsGrid');
        grid.innerHTML = '';
        
        Object.entries(this.components).forEach(([key, component]) => {
            const element = document.createElement('div');
            element.className = `component-item ${component.active ? 'component-active' : 'component-inactive'}`;
            element.innerHTML = `
                <span class="component-icon">${component.name.split(' ')[0]}</span>
                <span class="component-name">${component.name.split(' ').slice(1).join(' ')}</span>
                <div class="component-status ${component.active ? 'status-on' : 'status-off'}"></div>
            `;
            grid.appendChild(element);
        });
    }

    updateErrors(data) {
        const countElement = document.getElementById('errorCount');
        const listElement = document.getElementById('errorsList');
        
        if (data.count === 0) {
            countElement.textContent = '✅ No errors detected';
            countElement.style.background = '#d4edda';
            listElement.innerHTML = '';
        } else {
            countElement.textContent = `🚨 ${data.count} error(s) detected`;
            countElement.style.background = '#f8d7da';
            
            listElement.innerHTML = data.errors.map(error => `
                <div class="error-item">
                    <div class="error-code">${error.hex_code}</div>
                    <div class="error-desc">${error.description}</div>
                    ${error.counter > 0 ? `<small>Counter: ${error.counter}</small>` : ''}
                </div>
            `).join('');
        }
    }

    addMessage(message, type = 'system') {
        const log = document.getElementById('messageLog');
        const messageElement = document.createElement('div');
        messageElement.className = `message-item message-${type}`;
        messageElement.innerHTML = `
            <span class="message-time">${new Date().toLocaleTimeString()}</span>
            <span class="message-text">${message}</span>
        `;
        log.appendChild(messageElement);
        log.scrollTop = log.scrollHeight;
    }

    // API Calls
    async apiCall(endpoint, options = {}) {
        try {
            const response = await fetch(`/api${endpoint}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                ...options
            });
            return await response.json();
        } catch (error) {
            this.addMessage(`API Error: ${error.message}`, 'error');
            console.error('API call failed:', error);
        }
    }

    async connect() {
        await this.apiCall('/connect');
        this.addMessage('Connection request sent', 'system');
    }

    async disconnect() {
        await this.apiCall('/disconnect');
        this.addMessage('Disconnection request sent', 'system');
    }

    async refreshStatus() {
        try {
            const response = await fetch('/api/status');
            const status = await response.json();
            this.updateConnectionStatus({ newState: status.connection_state });
            this.updateHeaterStatus({ newState: status.heater_state });
        } catch (error) {
            console.error('Failed to refresh status:', error);
        }
    }

    async startMode(mode) {
        const minutesId = `${mode}Minutes`;
        const minutes = document.getElementById(minutesId).value;
        
        await this.apiCall(`/start/${mode}`, {
            body: JSON.stringify({ minutes: parseInt(minutes) })
        });
        
        this.addMessage(`Started ${mode} mode for ${minutes} minutes`, 'system');
    }

    async controlPump(enable) {
        await this.apiCall('/control/circulation-pump', {
            body: JSON.stringify({ enable })
        });
        
        this.addMessage(`Circulation pump ${enable ? 'enabled' : 'disabled'}`, 'system');
    }

    async shutdown() {
        if (confirm('Are you sure you want to shutdown the heater?')) {
            await this.apiCall('/shutdown');
            this.addMessage('Heater shutdown initiated', 'system');
        }
    }

    async refreshDeviceInfo() {
        try {
            const response = await fetch('/api/device/info');
            const info = await response.json();
            
            document.getElementById('wbusVersion').textContent = info.wbus_version || '-';
            document.getElementById('deviceName').textContent = info.device_name || '-';
            document.getElementById('deviceId').textContent = info.device_id || '-';
            document.getElementById('serialNumber').textContent = info.serial_number || '-';
        } catch (error) {
            console.error('Failed to refresh device info:', error);
        }
    }

    async refreshSensors() {
        try {
            const response = await fetch('/api/sensors/data');
            const data = await response.json();
            
            if (data.operational_measurements) {
                this.updateSensorData(data.operational_measurements);
            }
        } catch (error) {
            console.error('Failed to refresh sensors:', error);
        }
    }

    async checkErrors() {
        try {
            const response = await fetch('/api/errors');
            const data = await response.json();
            this.updateErrors(data);
        } catch (error) {
            console.error('Failed to check errors:', error);
        }
    }

    async clearErrors() {
        await this.apiCall('/errors/clear');
        this.addMessage('Errors cleared', 'system');
    }

    async testComponent(component) {
        const config = this.getTestConfig(component);
        if (!config) return;

        await this.apiCall(`/test/${component}`, {
            body: JSON.stringify(config)
        });
        
        this.addMessage(`Testing ${component}...`, 'system');
    }

    getTestConfig(component) {
        const configs = {
            'combustion-fan': {
                seconds: document.getElementById('fanSeconds').value,
                power: document.getElementById('fanPower').value
            },
            'fuel-pump': {
                seconds: document.getElementById('pumpSeconds').value,
                frequency: document.getElementById('pumpFreq').value
            },
            'glow-plug': {
                seconds: document.getElementById('glowSeconds').value,
                power: document.getElementById('glowPower').value
            },
            'circulation-pump': {
                seconds: document.getElementById('circSeconds').value,
                power: document.getElementById('circPower').value
            },
            'vehicle-fan': {
                seconds: document.getElementById('vehicleSeconds').value
            },
            'solenoid': {
                seconds: document.getElementById('solenoidSeconds').value
            }
        };
        
        return configs[component];
    }

    async clearMessages() {
        await this.apiCall('/messages/clear');
        document.getElementById('messageLog').innerHTML = '';
    }

    loadInitialData() {
        this.refreshStatus();
        this.refreshDeviceInfo();
        this.refreshSensors();
        this.checkErrors();
        this.renderComponents();
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.webastoController = new WebastoController();
});