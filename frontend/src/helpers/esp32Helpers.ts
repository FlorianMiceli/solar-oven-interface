const ESP_URL = 'http://192.168.4.1'; // ESP32's default AP IP address

export const sendCommand = async (command: string) => {
    try {
        return await fetch(`${ESP_URL}${command}`, {
            method: 'GET',
            mode: 'no-cors'
        });
    } catch (error) {
        console.error('Failed to send command:', error);
    }
};