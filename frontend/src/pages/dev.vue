<script setup lang="ts">
const toggle = async () => {
    //fetch http://192.168.4.1/turnonled
    const response = await fetch('http://192.168.4.1/turnonled',{
      method: 'GET',
      mode: 'no-cors'
    })
    console.log(response)
}
</script>

<template>
  <div class="flex items-center justify-center min-h-screen">
    <button @click="toggle" class="px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600">
      Toggle
    </button>
  </div>
</template>