<script setup lang="ts">
import BasePanel from "./TemplatePanel.vue";
import { LineChart } from '@/components/ui/chart-line'

const props = defineProps<{
    targetTemperature: number
}>();

const data = [
    {
        'year': 1970,
        'Export Growth Rate': 2.04,
        'Import Growth Rate': 1.53,
        'Target Temperature': props.targetTemperature,
    },
    // ... existing code ...
    {
        'year': 2021,
        'Export Growth Rate': 0.82,
        'Import Growth Rate': 2.51,
        'Target Temperature': props.targetTemperature,
    },
]
</script>

<template>
    <BasePanel title="TEMPERATURE">
        <LineChart :data="data" index="year"
            :categories="['Export Growth Rate', 'Import Growth Rate', 'Target Temperature']" :y-formatter="(tick, i) => {
                return typeof tick === 'number'
                    ? `${tick}°F`
                    : ''
            }" />
    </BasePanel>
</template>
