<script setup lang="ts">
import BasePanel from "./TemplatePanel.vue";
import { ChevronLeft, ChevronRight, ChevronUp, ChevronDown } from "lucide-vue-next";
import { Button } from "@/shadcn-components/ui/button";
import { computed } from "vue";

const emit = defineEmits<{
    'update:targetTemperature': [value: number]
}>();

const props = defineProps<{
    modelValue: number
}>();

const targetTemperature = computed(() => props.modelValue);

const updateTemperature = (value: number) => {
    emit('update:targetTemperature', value);
};

const onLeftPress = () => {
    emit('leftPress');
};

const onLeftRelease = () => {
    emit('leftRelease');
};

const onRightPress = () => {
    emit('rightPress');
};

const onRightRelease = () => {
    emit('rightRelease');
};

const onUpPress = () => {
    emit('upPress');
};

const onUpRelease = () => {
    emit('upRelease');
};

const onDownPress = () => {
    emit('downPress');
};

const onDownRelease = () => {
    emit('downRelease');
};
</script>

<template>
    <div class="control-panels">
        <BasePanel title="CONTROLS" class="navigation-panel">
            <div class="controls">
                <div class="controls-grid">
                    <div class="left-control">
                        <Button variant="outline" class="full-height-button" @mousedown="onLeftPress"
                            @mouseup="onLeftRelease" @mouseleave="onLeftRelease">
                            <ChevronLeft class="w-4 h-4" />
                        </Button>
                    </div>

                    <div class="center-controls">
                        <Button variant="outline" class="full-width-button" @mousedown="onUpPress"
                            @mouseup="onUpRelease" @mouseleave="onUpRelease">
                            <ChevronUp class="w-4 h-4" />
                        </Button>
                        <Button variant="outline" class="full-width-button" @mousedown="onDownPress"
                            @mouseup="onDownRelease" @mouseleave="onDownRelease">
                            <ChevronDown class="w-4 h-4" />
                        </Button>
                    </div>

                    <div class="right-control">
                        <Button variant="outline" class="full-height-button" @mousedown="onRightPress"
                            @mouseup="onRightRelease" @mouseleave="onRightRelease">
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
</style>
