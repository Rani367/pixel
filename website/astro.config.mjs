import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import react from '@astrojs/react';
import sitemap from '@astrojs/sitemap';

// Get current date for lastmod
const lastmod = new Date().toISOString();

export default defineConfig({
  site: 'https://rani367.github.io',
  base: '/pixel',
  integrations: [
    react(),
    sitemap({
      filter: (page) => !page.includes('/playground'),
      serialize: (item) => {
        // Add lastmod and changefreq to all pages
        return {
          ...item,
          lastmod: lastmod,
          changefreq: item.url.includes('/docs/') ? 'weekly' : 'monthly',
          priority: item.url === 'https://rani367.github.io/pixel/' ? 1.0 :
                   item.url.includes('/docs/getting-started') ? 0.9 :
                   item.url.includes('/docs/') ? 0.8 : 0.7,
        };
      },
    }),
  ],
  vite: {
    plugins: [tailwindcss()]
  }
});
