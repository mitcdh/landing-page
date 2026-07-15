const backgroundContainer = document.querySelector('.glitch__container');

async function getBackgrounds() {
  const embeddedBackgrounds = globalThis.__LANDING_DATA__?.backgrounds;
  if (embeddedBackgrounds) return embeddedBackgrounds;

  const response = await fetch('data/backgrounds.json');
  if (!response.ok) {
    throw new Error(`Background data request failed (${response.status})`);
  }

  return (await response.json()).backgrounds;
}

function getRandomBackground(backgrounds) {
  return backgrounds[Math.floor(backgrounds.length * Math.random())];
}

function chooseBackground(backgrounds) {
  const requestedName = new URLSearchParams(window.location.search).get('bg');
  if (!requestedName) return getRandomBackground(backgrounds);

  const requestedBackground = backgrounds.find(({ name }) => name === requestedName);
  if (requestedBackground) return requestedBackground;

  console.warn(`Background "${requestedName}" not found; using a random background.`);
  return getRandomBackground(backgrounds);
}

function createBackgroundImage({ src, avif }) {
  const webpUrl = new URL(src, document.baseURI).href;
  if (!avif) return `url("${webpUrl}")`;

  const avifUrl = new URL(avif, document.baseURI).href;
  return `image-set(url("${avifUrl}") type("image/avif"), url("${webpUrl}") type("image/webp"))`;
}

async function initializeBackground() {
  if (!backgroundContainer) return;

  try {
    const backgrounds = await getBackgrounds();
    if (!backgrounds.length) throw new Error('No backgrounds are configured.');

    const background = chooseBackground(backgrounds);
    backgroundContainer.style.setProperty('--glitch-image', createBackgroundImage(background));
    backgroundContainer.style.setProperty('--glitch-position', background.position);
  } catch (error) {
    console.error('Unable to initialize the background:', error);
  }
}

initializeBackground();
