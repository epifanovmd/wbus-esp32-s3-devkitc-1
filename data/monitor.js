// src/data/monitor.js
class MessageMonitor {
    constructor() {
        this.messages = [];
        this.filteredMessages = [];
        this.filters = {
            direction: 'all',
            search: ''
        };
        this.stats = { total: 0, tx: 0, rx: 0, error: 0 };
        this.autoScroll = true;
        
        this.init();
    }

    init() {
        this.bindEvents();
        this.connectWebSocket();
        this.loadMessages();
    }

    bindEvents() {
        document.getElementById('directionFilter').addEventListener('change', (e) => {
            this.filters.direction = e.target.value;
            this.applyFilters();
        });

        document.getElementById('searchFilter').addEventListener('input', (e) => {
            this.filters.search = e.target.value.toLowerCase();
            this.applyFilters();
        });

        document.getElementById('autoScroll').addEventListener('change', (e) => {
            this.autoScroll = e.target.checked;
        });

        document.getElementById('applyFilters').addEventListener('click', () => this.applyFilters());
        document.getElementById('clearAllBtn').addEventListener('click', () => this.clearAllMessages());
        document.getElementById('exportBtn').addEventListener('click', () => this.exportToCSV());
    }

    connectWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}:81`;
        
        const ws = new WebSocket(wsUrl);
        
        ws.onopen = () => console.log('WebSocket connected for monitoring');
        ws.onmessage = (event) => {
            const message = JSON.parse(event.data);
            this.handleWebSocketMessage(message);
        };
    }

    handleWebSocketMessage(message) {
        if (message.type === 'TX_RECEIVED' || message.type === 'COMMAND_SENT') {
            this.addMessage({
                timestamp: new Date().toISOString(),
                direction: 'TX',
                data: message.data.rx || message.data.tx,
                description: 'Command sent to Webasto'
            });
        } else if (message.type === 'RX_RECEIVED' || message.type === 'COMMAND_RECEIVED') {
            this.addMessage({
                timestamp: new Date().toISOString(),
                direction: 'RX', 
                data: message.data.rx,
                description: 'Response from Webasto'
            });
        } else if (message.type === 'COMMAND_SENT_ERRROR') {
            this.addMessage({
                timestamp: new Date().toISOString(),
                direction: 'ERROR',
                data: message.data.tx,
                description: 'Command failed'
            });
        }
    }

    async loadMessages() {
        try {
            const response = await fetch('/api/messages');
            this.messages = await response.json();
            this.applyFilters();
        } catch (error) {
            console.error('Failed to load messages:', error);
        }
    }

    addMessage(message) {
        this.messages.unshift(message); // Add to beginning for reverse chronological order
        if (this.messages.length > 1000) {
            this.messages.pop(); // Keep only last 1000 messages
        }
        this.applyFilters();
    }

    applyFilters() {
        this.filteredMessages = this.messages.filter(msg => {
            // Direction filter
            if (this.filters.direction !== 'all' && msg.direction.toLowerCase() !== this.filters.direction) {
                return false;
            }
            
            // Search filter
            if (this.filters.search && !msg.data.toLowerCase().includes(this.filters.search)) {
                return false;
            }
            
            return true;
        });
        
        this.updateStats();
        this.renderMessages();
    }

    updateStats() {
        this.stats = {
            total: this.messages.length,
            tx: this.messages.filter(m => m.direction === 'TX').length,
            rx: this.messages.filter(m => m.direction === 'RX').length,
            error: this.messages.filter(m => m.direction === 'ERROR').length
        };
        
        document.getElementById('totalCount').textContent = this.stats.total;
        document.getElementById('txCount').textContent = this.stats.tx;
        document.getElementById('rxCount').textContent = this.stats.rx;
        document.getElementById('errorCount').textContent = this.stats.error;
    }

    renderMessages() {
        const container = document.getElementById('messagesContainer');
        container.innerHTML = '';
        
        this.filteredMessages.forEach(msg => {
            const row = document.createElement('div');
            row.className = 'table-row';
            row.innerHTML = `
                <div>${new Date(msg.timestamp).toLocaleTimeString()}</div>
                <div class="direction-${msg.direction.toLowerCase()}">${msg.direction}</div>
                <div class="message-hex">${msg.data}</div>
                <div>${msg.description}</div>
            `;
            container.appendChild(row);
        });
        
        if (this.autoScroll) {
            container.scrollTop = 0;
        }
    }

    async clearAllMessages() {
        if (confirm('Are you sure you want to clear all messages?')) {
            await fetch('/api/messages/clear', { method: 'POST' });
            this.messages = [];
            this.applyFilters();
        }
    }

    exportToCSV() {
        const headers = ['Timestamp', 'Direction', 'Message', 'Description'];
        const csvContent = [
            headers.join(','),
            ...this.messages.map(msg => [
                msg.timestamp,
                msg.direction,
                `"${msg.data}"`,
                `"${msg.description}"`
            ].join(','))
        ].join('\n');
        
        const blob = new Blob([csvContent], { type: 'text/csv' });
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `webasto-messages-${new Date().toISOString().split('T')[0]}.csv`;
        a.click();
        window.URL.revokeObjectURL(url);
    }
}

// Initialize monitor
document.addEventListener('DOMContentLoaded', () => {
    window.messageMonitor = new MessageMonitor();
});