# SEO Setup Guide for Pixel Website

This document contains instructions for completing the SEO setup for the Pixel programming language website.

## Google Search Console Setup

### Step 1: Add Property

1. Go to [Google Search Console](https://search.google.com/search-console)
2. Click "Add property"
3. Choose "URL prefix" and enter: `https://rani367.github.io/pixel`
4. Click "Continue"

### Step 2: Verify Ownership

Choose one of these verification methods:

#### Option A: HTML File (Recommended for GitHub Pages)

1. Download the verification HTML file from Google
2. Add it to `/website/public/` directory
3. Deploy the site
4. Click "Verify" in Google Search Console

#### Option B: HTML Meta Tag

1. Copy the meta tag from Google (looks like `<meta name="google-site-verification" content="...">`)
2. Add it to `/website/src/components/SEO.astro` before the closing of the component
3. Deploy the site
4. Click "Verify" in Google Search Console

### Step 3: Submit Sitemap

1. In Search Console, go to "Sitemaps" in the left sidebar
2. Enter: `sitemap-index.xml`
3. Click "Submit"

The sitemap is located at: `https://rani367.github.io/pixel/sitemap-index.xml`

### Step 4: Request Indexing

1. Go to "URL Inspection" in Search Console
2. Enter your homepage URL: `https://rani367.github.io/pixel/`
3. Click "Request Indexing"
4. Repeat for key pages like `/docs/getting-started/`

## Bing Webmaster Tools (Optional)

1. Go to [Bing Webmaster Tools](https://www.bing.com/webmasters)
2. Sign in and add your site
3. Import from Google Search Console (easiest method)
4. Or add the sitemap manually: `https://rani367.github.io/pixel/sitemap-index.xml`

## Current SEO Implementation

### Meta Tags
- Title, description, keywords on all pages
- Open Graph tags for Facebook/LinkedIn sharing
- Twitter Card tags for Twitter/X sharing
- Canonical URLs to prevent duplicate content
- Theme color for browser UI

### Structured Data
- JSON-LD SoftwareApplication schema on all pages
- Includes: name, description, features, license, author, pricing

### Sitemap
- Auto-generated on each build
- Includes: lastmod, changefreq, priority
- Located at: `/sitemap-index.xml`

### robots.txt
- Allows all crawlers
- Points to sitemap location
- Located at: `/robots.txt`

### OG Image
- PNG format: `/og-image.png` (1200x630)
- SVG source: `/og-image.svg`

## Monitoring SEO Performance

### Key Metrics to Track

1. **Impressions**: How often your pages appear in search results
2. **Clicks**: How often users click through to your site
3. **Average Position**: Where your pages rank in search results
4. **Click-Through Rate (CTR)**: Clicks / Impressions

### Recommended Actions

- Check Search Console weekly for the first month
- Look for crawl errors and fix them
- Monitor which queries bring traffic
- Update meta descriptions for low-CTR pages

## Updating SEO Content

### To update page descriptions:

1. For doc pages: Edit frontmatter in `/docs/*.md` files:
   ```yaml
   ---
   title: "Page Title"
   description: "Your 150-160 character description here"
   keywords: ["keyword1", "keyword2", "keyword3"]
   ---
   ```

2. For other pages: Edit the `description` and `keywords` props in the page's Astro file

### To update the OG image:

1. Edit `/website/public/og-image.svg`
2. Regenerate PNG: `cd website/public && qlmanage -t -s 1200 -o . og-image.svg && mv og-image.svg.png og-image.png`

## SEO Checklist

- [x] Meta title and description on all pages
- [x] Open Graph tags
- [x] Twitter Card tags
- [x] Canonical URLs
- [x] Sitemap with lastmod
- [x] robots.txt
- [x] Structured data (JSON-LD)
- [x] OG image (1200x630 PNG)
- [ ] Google Search Console verification
- [ ] Submit sitemap to Google
- [ ] Bing Webmaster Tools (optional)
