import './assets/main.css'
import { createApp } from 'vue'
import { createPinia } from 'pinia'
import piniaPluginPersistedstate from 'pinia-plugin-persistedstate'
import App from './App.vue'
import { VueQueryPlugin } from '@tanstack/vue-query'
import Vue3Lottie from 'vue3-lottie'
import { createRouter, createWebHistory } from 'vue-router/auto'
import { routes } from 'vue-router/auto-routes'

const app = createApp(App)

const router = createRouter({
    history: createWebHistory(),
    routes,
})

const pinia = createPinia()
pinia.use(piniaPluginPersistedstate)
app.use(pinia)
app.use(router)
app.use(VueQueryPlugin)
app.use(Vue3Lottie)

app.mount('#app')
