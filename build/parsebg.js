const fs = require('fs');
const path = require('path');

function parseBackgrounds() {
  const backgrounds = [];
  let bgNames = '';

  fs.readdir('bg', (err, files) => {
    if (err) {
      console.error('parsebg.js :: Error reading bg directory:', err);
      return;
    }

    files.forEach(file => {
      const regex = /^(.+)\.(.+)\.(jpe?g|png|gif|webp)$/i;
      const match = regex.exec(file);
      if (match) {
        const name = match[1];
        const extension = match[3];
        const position = match[2].replace(/-/g, ' ');
        const src = `bg/${file}`;
        backgrounds.push({src, position});
        bgNames += ' ' + name;
      }
    });

    const data = {backgrounds};
    const json = JSON.stringify(data, null, 2);

    console.log(`parsebg.js :: Backgrounds parsed:${bgNames}`);

    fs.writeFile('data/backgrounds.json', json, (err) => {
      if (err) {
        console.error('parsebg.js :: Error writing to js/backgrounds.json:', err);
        return;
      }
        console.log('parsebg.js :: Successfully wrote to js/backgrounds.json');
    });
  });
}

module.exports = {
  parseBackgrounds
};
