<template>
    <header class="header">
        <div class="app-name">SOLAR OVEN PROJECT</div>
    </header>
    <router-view />
    <Toaster />
</template>

<style scoped>
.header {
    height: 4vh;
    background-color: white;
    padding: 10px;
    border-bottom: 1px solid #e0e0e0;
    display: flex;
    align-items: center;
    justify-content: center;
}

.app-name {
    font-weight: bold;
}
</style>
