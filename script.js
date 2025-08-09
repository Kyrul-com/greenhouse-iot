let ws;

function connectWebSocket() {
  const serverUrl = 'ws://YOUR_ESP32_SERVER_IP:PORT/ws'; // <-- Change this to your ESP32 server IP and port
  ws = new WebSocket(serverUrl);

  ws.onopen = () => {
    document.getElementById('status').textContent = 'Connected to server.';
    console.log('WebSocket connected');
  };

  ws.onmessage = (event) => {
    try {
      const msg = JSON.parse(event.data);
      if (msg.type === 'telemetry') {
        document.getElementById('temp').textContent = msg.data.temp_c ?? '--';
        document.getElementById('humidity').textContent = msg.data.humidity ?? '--';
        document.getElementById('battery').textContent = msg.data.battery_pct ?? '--';
        document.getElementById('solar').textContent = msg.data.solar_power ?? '--';
      }
    } catch (e) {
      console.error('Invalid message:', event.data);
    }
  };

  ws.onclose = () => {
    document.getElementById('status').textContent = 'Disconnected from server. Retrying...';
    console.log('WebSocket closed, retrying in 3s');
    setTimeout(connectWebSocket, 3000);
  };

  ws.onerror = (err) => {
    console.error('WebSocket error:', err);
  };
}

function sendControl(device, state) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    const msg = {
      type: 'control',
      node_id: 'server', // or specify node if multi-node support later
      cmd: 'set',
      args: {}
    };
    msg.args[device] = state;
    ws.send(JSON.stringify(msg));
    console.log('Sent control:', msg);
  } else {
    alert('WebSocket not connected.');
  }
}

// Start WebSocket connection when page loads
window.onload = connectWebSocket;
