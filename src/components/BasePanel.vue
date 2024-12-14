<script setup lang="ts">
defineProps<{
  title: string
}>()
</script>

<template>
  <div class="panel">
    <h2>{{ title }}</h2>
    <div class="content-placeholder">
      <slot></slot>
    </div>
  </div>
</template>

<style scoped>
.panel {
  background: white;
  border-radius: 8px;
  padding: 1rem;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  display: flex;
  flex-direction: column;
  flex: 1;
}

h2 {
  margin: 0 0 1rem 0;
  font-size: 0.875rem;
  font-weight: bold;
  color: #333;
}

.content-placeholder {
  background: #f0f0f0;
  width: 100%;
  height: 100%;
  border-radius: 4px;
  flex: 1;
}
</style> 