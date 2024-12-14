<script setup lang="ts">
import BasePanel from "./TemplatePanel.vue";
</script>

<template>
    <BasePanel title="3D VISUALIZATION" class="large-panel">
        <div class="content-placeholder">
            <!-- Contenu de la visualisation 3D -->
        </div>
    </BasePanel>
</template>

<style scoped>
.large-panel {
    position: relative;
    min-width: 400px;
}

.content-placeholder {
    width: 100%;
    height: 100%;
}
</style>
