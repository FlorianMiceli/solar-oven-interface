<script setup lang="ts">
import { ref } from 'vue';

const modelOrientation = ref(0);
const targetTemperature = ref(0);
</script>

<template>
  <div class="dashboard">
    <div class="main-section">
      <VisualizationPanel v-model:modelOrientation="modelOrientation" />
      <ControlPanel v-model:modelOrientation="modelOrientation" v-model="targetTemperature" />
    </div>

    <div class="side-panels">
      <TemperaturePanel />
      <div class="panel-row">
        <NitsPanel />
        <BatteryPanel />
      </div>
    </div>
  </div>
</template>

<style scoped>
.dashboard {
  display: flex;
  gap: 1rem;
  padding: 1rem;
  background-color: #f5f5f5;
  height: calc(100vh - 4vh);
  box-sizing: border-box;
  overflow: hidden;
}

.main-section {
  display: flex;
  flex-direction: column;
  gap: 1rem;
  flex: 1;
}

.side-panels {
  display: flex;
  flex-direction: column;
  gap: 1rem;
  width: 600px;
  height: 100%;
}

.panel-row {
  display: flex;
  gap: 1rem;
  flex: 1;
}
</style>