const ESP_URL = 'http://192.168.4.1'; // ESP32's default AP IP address

export const sendCommand = async (command: string, method: string = 'GET') => {
    try {
        const response = await fetch(`${ESP_URL}${command}`, {
            method: method,
            mode: 'cors',
            credentials: 'omit',
            headers: {
                'Accept': 'application/json, text/plain, */*',
                'Content-Type': 'application/json'
            }
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.text();
        return {
            success: true,
            data: data
        };
    } catch (error) {
        console.error('Failed to send command:', error);
        return {
            success: false,
            error: error instanceof Error ? error.message : 'Unknown error occurred'
        };
    }
};