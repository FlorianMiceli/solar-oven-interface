import type { User } from "@/types/global_types";

export const useUserStore = defineStore("user", {
    state: () => ({
        user: null as User | null,
    }),
    persist: true, // pinia-plugin-persistedstate to use localStorage
    actions: {
        emptyStore() {
            this.user = null;
        },
    },
    getters: {},
});
