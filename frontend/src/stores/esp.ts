import { defineStore } from 'pinia';

export const useEspStore = defineStore("esp", {
    state: () => ({
        isConnected: false,
    }),
    actions: {
        setConnectionStatus(status: boolean) {
            this.isConnected = status;
        },
    },
}); 