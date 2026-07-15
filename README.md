# Landing Page for mitcdh

A small, framework-free landing page for `mitcdh.au`. It combines a rotating set of personal photographs, multilingual typed greetings, a CSS glitch sequence, and hand-styled social marks.

## Visual system

- `css/home.css` contains the readable source for the sliced background, mirrored text, flash, and cursor animations. The glitch keyframes use discrete timing so their digital jumps remain sharp without interpolating full-screen clip paths on every frame.
- `js/background.js` selects a random photograph and applies the focal position encoded in its filename. Add `?bg=elliot` (or another background name) to choose a specific image while developing.
- `js/greeting.js` types greetings in a shuffled order while prioritizing the viewer's browser languages.
- The six social logos are the original Font Awesome brand paths embedded directly in `index.html`. Their GitHub, Instagram, Flickr, LinkedIn, and YouTube color treatments remain in `css/home.css`; no icon runtime is sent to visitors.
- Visitors who request reduced motion receive the same composition without the glitch or typing animations.

## Local development

The uncompiled source works directly in a static server:

```sh
python3 -m http.server 4173
```

Then open `http://localhost:4173/`.

Background filenames use `name.position.webp`, with `-` separating position values and `p` standing for `%`. For example, `elliot.40p-70p.webp` becomes `background-position: 40% 70%`. After adding or renaming a source photograph, refresh the development JSON:

```sh
npm run refresh:backgrounds
```

## Production build

Install the locked build dependencies and compile the site:

```sh
npm ci
npm run build
```

The deployable site is written to `dist/`. The build:

- combines and minifies the readable CSS;
- bundles and minifies the JavaScript and embeds the small greeting/background manifests, avoiding two startup requests;
- adds content hashes to CSS and JavaScript filenames;
- caps deployment backgrounds at 2560 pixels wide, keeps a WebP fallback, and creates a smaller AVIF choice;
- copies Cloudflare Pages `_headers` and `_redirects` rules into the output; and
- excludes source/build files and the old, unused icon libraries from the deployment.

Source photographs are never modified by the production build.

## Cloudflare Pages

Use these project settings:

- Framework preset: `None`
- Build command: `npm run build`
- Build output directory: `dist`
- Root directory: the repository root
- Node.js: version 20 or newer

The generated hashed assets receive long-lived immutable browser caching. Backgrounds receive a shorter cache lifetime because their readable filenames are stable. The global header block also supplies a Content Security Policy and common browser hardening headers.

Cloudflare Pages build-output, `_headers`, and `_redirects` behavior is documented in the [build configuration](https://developers.cloudflare.com/pages/configuration/build-configuration/), [headers](https://developers.cloudflare.com/pages/configuration/headers/), and [redirects](https://developers.cloudflare.com/pages/configuration/redirects/) guides.

## Background

- [Original build write-up](https://blog.mitcdh.au/p/building-a-custom-landing-page)
- [Live site](https://mitcdh.au)
- Glitch animation based on Codrops' [CSS Glitch Effect](https://github.com/codrops/CSSGlitchEffect/)
