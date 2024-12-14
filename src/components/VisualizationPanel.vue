<script setup lang="ts">
import BasePanel from './BasePanel.vue'
</script>

<template>
  <BasePanel title="3D VISUALIZATION" class="large-panel">
    <div class="content-placeholder">
      <div class="controls">
        <button class="nav-btn">&lt;</button>
        <div class="vertical-controls">
          <button class="nav-btn">^</button>
          <button class="nav-btn">v</button>
        </div>
        <button class="nav-btn">&gt;</button>
      </div>
    </div>
  </BasePanel>
</template>

<style scoped>
.large-panel {
  position: relative;
  min-width: 400px;
}

.controls {
  position: absolute;
  bottom: 1rem;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  gap: 0.5rem;
  align-items: center;
}

.vertical-controls {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.nav-btn {
  width: 40px;
  height: 40px;
  border: none;
  background: white;
  border-radius: 4px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
}

.nav-btn:hover {
  background: #f0f0f0;
}
</style> 