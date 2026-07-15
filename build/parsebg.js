const fs = require('node:fs/promises');
const path = require('node:path');

const root = path.resolve(__dirname, '..');
const backgroundDirectory = path.join(root, 'bg');
const backgroundDataPath = path.join(root, 'data', 'backgrounds.json');

function parseBackgroundFilename(file) {
  const match = /^(.+)\.(.+)\.(jpe?g|png|gif|webp)$/i.exec(file);
  if (!match) return null;

  const [, name, encodedPosition] = match;
  const position = encodedPosition
    .replace(/-/g, ' ')
    .replace(/(\d+)p(?=\s|$)/g, '$1%');

  return { name, src: `bg/${file}`, position };
}

async function getBackgrounds() {
  const files = await fs.readdir(backgroundDirectory);
  return files
    .sort((a, b) => a.localeCompare(b))
    .map(parseBackgroundFilename)
    .filter(Boolean);
}

async function parseBackgrounds() {
  const backgrounds = await getBackgrounds();
  await fs.writeFile(
    backgroundDataPath,
    `${JSON.stringify({ backgrounds }, null, 2)}\n`,
  );
  console.log(`Wrote ${backgrounds.length} backgrounds to data/backgrounds.json.`);
  return backgrounds;
}

if (require.main === module) {
  parseBackgrounds().catch((error) => {
    console.error(error);
    process.exitCode = 1;
  });
}

module.exports = { getBackgrounds, parseBackgrounds };
