<script setup lang="ts">
</script>

<template>
  <div class="dashboard">
    <VisualizationPanel />

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