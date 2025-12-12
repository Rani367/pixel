import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import react from '@astrojs/react';

export default defineConfig({
  site: 'https://rani367.github.io',
  base: '/pixel',
  integrations: [react()],
  vite: {
    plugins: [tailwindcss()]
  }
});
