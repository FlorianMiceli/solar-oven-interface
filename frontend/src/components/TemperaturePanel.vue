<script setup lang="ts">
import BasePanel from "./TemplatePanel.vue";
import {
    Chart as ChartJS,
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Title,
    Tooltip,
    Legend
} from 'chart.js'
import { Line } from 'vue-chartjs'
import * as chartConfig from './chartConfig.js'
import { ref, onMounted, onUnmounted, computed } from 'vue';
import { sendCommand } from "@/helpers/esp32Helpers";
import { useEspStore } from "@/stores/esp";

// Register Chart.js components
ChartJS.register(
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Title,
    Tooltip,
    Legend
);

const espStore = useEspStore();

interface TemperatureDataPoint {
    timestamp: number;
    'Current Temperature': number;
    'Target Temperature': number;
}

const currentTemperature = ref(0);
const targetTemperature = ref(45); // Default matching ControlPanel
const temperatureData = ref<TemperatureDataPoint[]>([]);

// Chart.js configuration
const chartData = computed(() => ({
    labels: temperatureData.value.map((d: TemperatureDataPoint) =>
        new Date(d.timestamp).toLocaleTimeString('en-US', {
            hour12: false,
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        })
    ),
    datasets: [
        {
            label: 'Current Temperature',
            data: temperatureData.value.map((d: TemperatureDataPoint) => d['Current Temperature']),
            borderColor: 'rgb(75, 192, 192)',
            tension: 0.1,
            fill: false
        },
        {
            label: 'Target Temperature',
            data: temperatureData.value.map((d: TemperatureDataPoint) => d['Target Temperature']),
            borderColor: 'rgb(255, 99, 132)',
            tension: 0.1,
            fill: false,
            borderDash: [5, 5] // Makes the line dashed
        }
    ]
}));

const options = {
    responsive: true,
    maintainAspectRatio: false,
    animation: {
        duration: 0 // Disable animation for real-time updates
    },
    scales: {
        y: {
            beginAtZero: false,
            title: {
                display: true,
                text: 'Temperature (°C)'
            }
        },
        x: {
            title: {
                display: true,
                text: 'Time'
            }
        }
    },
    plugins: {
        legend: {
            display: true,
            position: 'top' as const
        },
        tooltip: {
            mode: 'index',
            intersect: false
        }
    }
};

// Add polling mechanism for target temperature
let targetTempPollInterval: ReturnType<typeof setInterval> | null = null;
let temperaturePollInterval: ReturnType<typeof setInterval> | null = null;

const checkTargetTemperature = async () => {
    try {
        const response = await sendCommand('/getTargetTemperature');
        if (response?.success && response.data) {
            targetTemperature.value = parseFloat(response.data);
        }
    } catch (error) {
        console.error('Failed to fetch target temperature:', error);
    }
};

const checkCurrentTemperature = async () => {
    try {
        const response = await sendCommand('/getTemperature');
        if (response?.success && response.data) {
            currentTemperature.value = parseFloat(response.data);
            // Add new data point
            temperatureData.value.push({
                timestamp: Date.now(),
                'Current Temperature': currentTemperature.value,
                'Target Temperature': targetTemperature.value
            });
            // Keep only last 15 data points
            if (temperatureData.value.length > 15) {
                temperatureData.value.shift();
            }
            espStore.setConnectionStatus(true);
        }
        if (response?.data === undefined) {
            espStore.setConnectionStatus(false);
        }
    } catch (error) {
        console.error('Failed to fetch temperature:', error);
        espStore.setConnectionStatus(false);
    }
};

onMounted(() => {
    // Initial checks
    checkCurrentTemperature();
    checkTargetTemperature();
    // Set up polling every second
    temperaturePollInterval = setInterval(checkCurrentTemperature, 1000);
    targetTempPollInterval = setInterval(checkTargetTemperature, 1000);
});

onUnmounted(() => {
    // Clean up intervals when component is destroyed
    if (temperaturePollInterval !== null) {
        clearInterval(temperaturePollInterval);
    }
    if (targetTempPollInterval !== null) {
        clearInterval(targetTempPollInterval);
    }
});

</script>

<template>
    <BasePanel title="TEMPERATURE">
        <div class="flex flex-col items-center justify-center m-2 p-4 bg-white rounded-lg shadow-sm">
            <div class="text-sm text-neutral-500 mb-1">Current Temperature</div>
            <div class="text-4xl font-bold text-neutral-800">
                {{ currentTemperature?.toFixed(1) || '--' }}°C
            </div>
        </div>
        <div class="h-[300px] m-2 p-4 bg-white rounded-lg shadow-sm">
            <Line :data="chartData" :options="options" />
        </div>
    </BasePanel>
</template>
