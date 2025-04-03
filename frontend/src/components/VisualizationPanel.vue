<script setup lang="ts">
import * as THREE from 'three'
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader'
import { useEspStore } from "@/stores/esp";
import { computed,ref, onMounted, watch } from 'vue';

const espStore = useEspStore();

const props = defineProps<{
  modelOrientation: number
}>();

const emit = defineEmits<{
  'update:modelOrientation': [value: number]
}>();

const modelOrientation = computed({
  get: () => props.modelOrientation,
  set: (value) => emit('update:modelOrientation', value)
});

const canvas = ref<HTMLCanvasElement>();
const webGLError = ref<string | null>(null);
const hasWebGL = ref(true);

let scene: THREE.Scene, camera: THREE.PerspectiveCamera, renderer: THREE.WebGLRenderer, model: THREE.Group;

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

    // Activer le clipping
    renderer.localClippingEnabled = true;

    const clippingPlane = new THREE.Plane(new THREE.Vector3(0, 0, 1), 0);

    const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
    scene.add(ambientLight);

    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(5, 5, 5);
    scene.add(directionalLight);

    const loader = new GLTFLoader();
    loader.load(
      '/src/assets/3dmodel.gltf',
      (gltf) => {
        model = gltf.scene;
        model.scale.set(1, 1, 1);
        model.position.set(0, 0, 0);

        model.rotation.x = 0;
        model.rotation.y = 0;
        model.rotation.z = Math.PI / 2;

        // Appliquer le clipping plane à tous les matériaux du modèle
        model.traverse((node: THREE.Object3D) => {
          if ((node as THREE.Mesh).isMesh) {
            const mesh = node as THREE.Mesh;
            if (mesh.material) {
              const material = mesh.material as THREE.Material;
              material.clippingPlanes = [clippingPlane];
              material.clipShadows = true;
              material.needsUpdate = true;
            }
          }
        });

        scene.add(model);
      },
      (progress) => {
        console.log('Loading progress:', (progress.loaded / progress.total * 100) + '%');
      },
      (error) => {
        console.error('Error loading model:', error);
        webGLError.value = `Failed to load 3D model: ${error.message}`;
      }
    );

    camera.position.z = 10;
    camera.position.y = -25;
    camera.position.x = 0;
    camera.lookAt(0, 0, 10);
    camera.up.set(0, 0, 0);
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

watch(modelOrientation, (newValue: any) => {
  if (model) {
    // Rotation du modèle autour de l'axe Z
    model.rotation.x = 0;
    model.rotation.y = 0;
    model.rotation.z = Math.PI / 2 + (newValue * Math.PI * 2); // Rotation autour de l'axe Z en partant de PI/2
  }
});
</script>

<template>
  <TemplatePanel title="3D VISUALIZATION" class="large-panel">
    <div class="content-placeholder h-[200px] md:h-[300px]">
      <div v-show="espStore.isConnected">
        <div v-if="hasWebGL" class="viewport-container">
          <canvas ref="canvas" width="300" height="300"></canvas>
        </div>
        <div v-else class="error-message">
          {{ webGLError }}
          <p>Please try using a modern browser with WebGL support.</p>
        </div>
      </div>
      <div v-show="!espStore.isConnected" class="text-destructive font-medium">
        ESP DISCONNECTED
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

.text-destructive {
  text-align: center;
}
</style>
