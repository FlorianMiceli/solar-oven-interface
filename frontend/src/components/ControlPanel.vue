<script setup lang="ts">
import BasePanel from "./TemplatePanel.vue";
import { ChevronLeft, ChevronRight, ChevronUp, ChevronDown } from "lucide-vue-next";
import { Button } from "@/shadcn-components/ui/button";
import { computed, watch, ref, onMounted, onUnmounted } from "vue";
import { sendCommand } from "@/helpers/esp32Helpers";
import { Switch } from "@/components/ui/switch";
import { useEspStore } from "@/stores/esp";

const espStore = useEspStore();

const emit = defineEmits<{
    'update:targetTemperature': [value: number]
    'update:modelOrientation': [value: number]
}>();

const props = defineProps<{
    modelValue: number
    modelOrientation: number
}>();

const targetTemperature = computed(() => props.modelValue);
const currentOrientation = computed({
    get: () => props.modelOrientation,
    set: (value) => emit('update:modelOrientation', value)
});

const ROTATION_SPEED = 0.001;
let rotationInterval: ReturnType<typeof setInterval> | null = null;

const toggleMode = ref(true)

// Add polling mechanism for mode status
let pollInterval: ReturnType<typeof setInterval> | null = null;

const checkModeStatus = async () => {
    try {
        const response = await sendCommand('/getMode')
        console.log(response?.data)
        if (response) {
            toggleMode.value = response.data === 'manuel';
            espStore.setConnectionStatus(true);
        }
        if (response?.data === undefined) {
            espStore.setConnectionStatus(false);
        }
    } catch (error) {
        console.error('Failed to fetch mode status:', error);
        espStore.setConnectionStatus(false);
    }
};

onMounted(() => {
    // Initial check
    checkModeStatus();
    // Set up polling every second
    pollInterval = setInterval(checkModeStatus, 1000);
});

onUnmounted(() => {
    // Clean up intervals when component is destroyed
    if (pollInterval !== null) {
        clearInterval(pollInterval);
    }
    if (rotationInterval !== null) {
        clearInterval(rotationInterval);
    }
});

watch(toggleMode, (newValue) => {
    if (newValue) {
        turnOnManualMode();
    } else {
        turnOnAsservissementMode();
    }
});

const updateTemperature = (value: number) => {
    emit('update:targetTemperature', value);
};

const onLeftPress = () => {
    sendCommand('/rotation_gauche');
    rotationInterval = setInterval(() => {
        currentOrientation.value = (currentOrientation.value + ROTATION_SPEED) % 1;
    }, 16);
};

const onLeftRelease = () => {
    sendCommand('/stop');
    if (rotationInterval) {
        clearInterval(rotationInterval);
        rotationInterval = null;
    }
};

const onRightPress = () => {
    sendCommand('/rotation_droite');
    rotationInterval = setInterval(() => {
        currentOrientation.value = (currentOrientation.value - ROTATION_SPEED) % 1;
    }, 16);
};

const onRightRelease = () => {
    sendCommand('/stop');
    if (rotationInterval) {
        clearInterval(rotationInterval);
        rotationInterval = null;
    }
};

const onUpPress = () => {
    sendCommand('/translation_avant');
};

const onUpRelease = () => {
    sendCommand('/stop');
};

const onDownPress = () => {
    sendCommand('/translation_arriere');
};

const onDownRelease = () => {
    sendCommand('/stop');
};

const turnOnManualMode = () => {
    sendCommand('/mode_manuel')
}

const turnOnAsservissementMode = () => {
    sendCommand('/mode_asservissement')
}
</script>

<template>
    <div class="control-panels">
        <BasePanel title="MANUAL MODE" class="navigation-panel">
            <div class="controls">
                <div class="control-grid">
                    <Switch id="airplane-mode" v-model="toggleMode" :disabled="!espStore.isConnected" />
                </div>
            </div>
        </BasePanel>
        <BasePanel title="CONTROLS" class="navigation-panel">
            <div class="controls">
                <div class="controls-grid">
                    <div class="left-control">
                        <Button variant="outline" class="full-height-button" @mousedown="onLeftPress"
                            @mouseup="onLeftRelease" @mouseleave="onLeftRelease" :disabled="!toggleMode">
                            <ChevronLeft class="w-4 h-4" />
                        </Button>
                    </div>

                    <div class="center-controls">
                        <Button variant="outline" class="full-width-button" @mousedown="onUpPress"
                            @mouseup="onUpRelease" @mouseleave="onUpRelease" :disabled="!toggleMode">
                            <ChevronUp class="w-4 h-4" />
                        </Button>
                        <Button variant="outline" class="full-width-button" @mousedown="onDownPress"
                            @mouseup="onDownRelease" @mouseleave="onDownRelease" :disabled="!toggleMode">
                            <ChevronDown class="w-4 h-4" />
                        </Button>
                    </div>

                    <div class="right-control">
                        <Button variant="outline" class="full-height-button" @mousedown="onRightPress"
                            @mouseup="onRightRelease" @mouseleave="onRightRelease" :disabled="!toggleMode">
                            <ChevronRight class="w-4 h-4" />
                        </Button>
                    </div>
                </div>
            </div>
        </BasePanel>

        <BasePanel title="TARGET TEMPERATURE" class="temperature-panel">
            <div class="temperature-input">
                <NumberField id="temperature" :model-value="targetTemperature" @update:model-value="updateTemperature"
                    :min="0">
                    <NumberFieldContent>
                        <NumberFieldDecrement />
                        <NumberFieldInput />
                        <NumberFieldIncrement />
                    </NumberFieldContent>
                </NumberField>
            </div>
        </BasePanel>
    </div>
</template>

<style scoped>
.control-panels {
    display: flex;
    gap: 1rem;
    width: 100%;
}

.navigation-panel,
.temperature-panel {
    flex: 1;
}

.controls {
    height: 100%;
    padding: 0.5rem;
    display: flex;
    align-items: center;
    justify-content: center;
}

.control-grid {
    display: flex;
    align-items: center;
    justify-content: center;
    width: 100%;
}

.controls-grid {
    display: grid;
    grid-template-columns: 1fr 2fr 1fr;
    gap: 0.5rem;
    height: 100%;
    min-height: 120px;
}

.center-controls {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    height: 100%;
}

.full-height-button {
    height: 100%;
    width: 100%;
}

.full-width-button {
    width: 100%;
    height: 100%;
}

.temperature-input {
    display: flex;
    align-items: center;
    justify-content: center;
    height: 100%;
    padding: 0.5rem;
}

:deep(.number-field-content) {
    min-width: 200px;
}

@media (max-width: 768px) {
    .control-panels {
        flex-direction: column;
    }

    .navigation-panel,
    .temperature-panel {
        width: 100%;
    }

    .controls-grid {
        min-height: 100px;
    }
}
</style>
