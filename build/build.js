const { createHash } = require('node:crypto');
const fs = require('node:fs/promises');
const path = require('node:path');
const esbuild = require('esbuild');
const { minify } = require('html-minifier-terser');
const sharp = require('sharp');
const { getBackgrounds } = require('./parsebg');

const root = path.resolve(__dirname, '..');
const outputDirectory = path.join(root, 'dist');
const maximumBackgroundWidth = 2560;
const imageConcurrency = 4;

const publicFiles = [
  '_headers',
  '_redirects',
  'apple-touch-icon.png',
  'favicon-96x96.png',
  'favicon.ico',
  'favicon.svg',
  'logo.png',
  'logo.svg',
  'robots.txt',
  'site.webmanifest',
  'web-app-manifest-192x192.png',
  'web-app-manifest-512x512.png',
];

const contentHash = (content) => createHash('sha256').update(content).digest('hex').slice(0, 10);

async function copyPublicFiles() {
  await Promise.all(publicFiles.map((file) => fs.copyFile(
    path.join(root, file),
    path.join(outputDirectory, file),
  )));

  await Promise.all(['blog', 'pic'].map((directory) => fs.cp(
    path.join(root, directory),
    path.join(outputDirectory, directory),
    { recursive: true },
  )));

  await fs.cp(
    path.join(root, 'css', 'fonts'),
    path.join(outputDirectory, 'css', 'fonts'),
    { recursive: true },
  );
}

async function optimizeBackground(background) {
  const inputPath = path.join(root, background.src);
  const outputWebpPath = path.join(outputDirectory, background.src);
  const avifSource = background.src.replace(/\.webp$/i, '.avif');
  const outputAvifPath = path.join(outputDirectory, avifSource);
  const original = await fs.readFile(inputPath);

  const resized = sharp(original).rotate().resize({
    width: maximumBackgroundWidth,
    withoutEnlargement: true,
  });

  const [optimizedWebp, optimizedAvif] = await Promise.all([
    resized.clone().webp({ quality: 82, effort: 4, smartSubsample: true }).toBuffer(),
    resized.clone().avif({ quality: 42, effort: 3, chromaSubsampling: '4:2:0' }).toBuffer(),
  ]);
  const webp = optimizedWebp.length < original.length ? optimizedWebp : original;
  const useAvif = optimizedAvif.length < webp.length;

  await fs.mkdir(path.dirname(outputWebpPath), { recursive: true });
  await fs.writeFile(outputWebpPath, webp);
  if (useAvif) await fs.writeFile(outputAvifPath, optimizedAvif);

  return useAvif ? { ...background, avif: avifSource } : background;
}

async function optimizeBackgrounds(backgrounds) {
  const optimized = [];

  for (let index = 0; index < backgrounds.length; index += imageConcurrency) {
    const batch = backgrounds.slice(index, index + imageConcurrency);
    optimized.push(...await Promise.all(batch.map(optimizeBackground)));
  }

  return optimized;
}

async function buildCss() {
  const [webfonts, home] = await Promise.all([
    fs.readFile(path.join(root, 'css', 'webfonts.css'), 'utf8'),
    fs.readFile(path.join(root, 'css', 'home.css'), 'utf8'),
  ]);
  const result = await esbuild.transform(`${webfonts}\n${home}`, {
    legalComments: 'none',
    loader: 'css',
    minify: true,
    target: 'chrome95',
  });
  const filename = `site.${contentHash(result.code)}.css`;

  await fs.mkdir(path.join(outputDirectory, 'css'), { recursive: true });
  await fs.writeFile(path.join(outputDirectory, 'css', filename), result.code);
  return filename;
}

async function buildJavaScript(siteData) {
  const result = await esbuild.build({
    banner: { js: `globalThis.__LANDING_DATA__=${JSON.stringify(siteData)};` },
    bundle: true,
    entryPoints: [path.join(root, 'js', 'app.js')],
    format: 'iife',
    legalComments: 'none',
    minify: true,
    target: ['es2020'],
    write: false,
  });
  const code = result.outputFiles[0].contents;
  const filename = `app.${contentHash(code)}.js`;

  await fs.mkdir(path.join(outputDirectory, 'js'), { recursive: true });
  await fs.writeFile(path.join(outputDirectory, 'js', filename), code);
  return filename;
}

async function buildHtml(cssFilename, javascriptFilename) {
  const source = await fs.readFile(path.join(root, 'index.html'), 'utf8');
  const compiled = source
    .replace(
      /<!-- build:styles -->[\s\S]*?<!-- \/build:styles -->/,
      `<link rel="stylesheet" href="/css/${cssFilename}">`,
    )
    .replace(
      /<!-- build:scripts -->[\s\S]*?<!-- \/build:scripts -->/,
      `<script src="/js/${javascriptFilename}" defer></script>`,
    );
  const optimized = await minify(compiled, {
    collapseWhitespace: true,
    minifyCSS: true,
    removeComments: true,
    removeRedundantAttributes: true,
    sortAttributes: true,
    sortClassName: true,
    useShortDoctype: true,
  });

  await fs.writeFile(path.join(outputDirectory, 'index.html'), optimized);
}

async function build() {
  const [sourceBackgrounds, greetingData, signalbreakText] = await Promise.all([
    getBackgrounds(),
    fs.readFile(path.join(root, 'data', 'greetings.json'), 'utf8').then(JSON.parse),
    fs.readFile(path.join(root, 'data', 'signal-break.c'), 'utf8'),
  ]);

  await fs.rm(outputDirectory, { recursive: true, force: true });
  await fs.mkdir(outputDirectory, { recursive: true });

  const backgrounds = await optimizeBackgrounds(sourceBackgrounds);
  const siteData = { backgrounds, greetings: greetingData.greetings, signalbreak: signalbreakText };
  const [cssFilename, javascriptFilename] = await Promise.all([
    buildCss(),
    buildJavaScript(siteData),
    copyPublicFiles(),
  ]);

  await fs.mkdir(path.join(outputDirectory, 'data'), { recursive: true });
  await Promise.all([
    buildHtml(cssFilename, javascriptFilename),
    fs.writeFile(
      path.join(outputDirectory, 'data', 'backgrounds.json'),
      `${JSON.stringify({ backgrounds }, null, 2)}\n`,
    ),
    fs.copyFile(
      path.join(root, 'data', 'greetings.json'),
      path.join(outputDirectory, 'data', 'greetings.json'),
    ),
    fs.copyFile(
      path.join(root, 'data', 'signal-break.c'),
      path.join(outputDirectory, 'data', 'signal-break.c'),
    ),
  ]);

  const outputFiles = await fs.readdir(path.join(outputDirectory, 'bg'));
  console.log(`Built ${outputFiles.length} optimized background assets and hashed CSS/JS in dist/.`);
}

build().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
