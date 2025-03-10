<script setup lang="ts">
import * as THREE from 'three'
import { ParametricGeometry } from 'three/examples/jsm/geometries/ParametricGeometry';

const props = defineProps<{
  modelOrientation?: number
}>();

const emit = defineEmits<{
  'update:modelOrientation': [value: number]
}>();

const canvas = ref<HTMLCanvasElement>();
const webGLError = ref<string | null>(null);
const hasWebGL = ref(true);

const orientation = computed({
  get: () => props.modelOrientation ?? 0,
  set: (value) => emit('update:modelOrientation', value)
});

let scene: THREE.Scene, camera: THREE.PerspectiveCamera, renderer: THREE.WebGLRenderer, parabola: THREE.Mesh;

const checkWebGLSupport = (): boolean => {
  try {
    const canvas = document.createElement('canvas');
    return !!(window.WebGLRenderingContext &&
      (canvas.getContext('webgl') || canvas.getContext('experimental-webgl')));
  } catch (e) {
    return false;
  }
};

const initScene = () => {
  if (!checkWebGLSupport()) {
    hasWebGL.value = false;
    webGLError.value = "WebGL is not supported in your browser";
    return;
  }

  try {
    scene = new THREE.Scene();
    camera = new THREE.PerspectiveCamera(75, 1, 0.1, 1000);

    if (!canvas.value) return;

    renderer = new THREE.WebGLRenderer({
      canvas: canvas.value,
      antialias: true,
      alpha: true
    });

    if (!renderer) {
      throw new Error("Failed to create WebGL renderer");
    }

    const geometry = new ParametricGeometry((u: number, v: number, dest: THREE.Vector3) => {
      const x = (u * 2 - 1) / 2;
      const y = (v * 2 - 1) / 2;
      const z = x * x + y * y;
      dest.set(x, y, z);
    }, 50, 50);

    const material = new THREE.MeshBasicMaterial({ color: 0x00ff00, wireframe: true });
    parabola = new THREE.Mesh(geometry, material);
    scene.add(parabola);

    camera.position.z = 2;
  } catch (error: any) {
    hasWebGL.value = false;
    webGLError.value = `Failed to initialize 3D visualization: ${error.message}`;
    console.error('Three.js initialization error:', error);
  }
};

const animate = () => {
  if (!hasWebGL.value || !renderer) return;

  requestAnimationFrame(animate);
  renderer.render(scene, camera);
};

onMounted(() => {
  initScene();
  if (hasWebGL.value) {
    animate();
  }
});

watch(orientation, (newValue) => {
  if (parabola) {
    parabola.rotation.y = newValue * Math.PI * 2;
  }
});
</script>

<template>
  <TemplatePanel title="3D VISUALIZATION" class="large-panel">
    <div class="content-placeholder">
      <div v-if="hasWebGL" class="viewport-container">
        <input type="range" v-model="orientation" min="0" max="1" step="0.01">
        <canvas ref="canvas" width="300" height="300"></canvas>
      </div>
      <div v-else class="error-message">
        {{ webGLError }}
        <p>Please try using a modern browser with WebGL support.</p>
      </div>
    </div>
  </TemplatePanel>
</template>

<style scoped>
.large-panel {
  position: relative;
  min-width: 300px;
}

.content-placeholder {
  width: 100%;
  height: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
}

.viewport-container {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 10px;
}

.error-message {
  text-align: center;
  color: #ff4444;
  padding: 20px;
}
</style>
