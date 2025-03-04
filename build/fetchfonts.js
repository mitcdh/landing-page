const https = require('https');
const fs = require('fs');
const querystring = require('querystring');

const fontWeight = '700';

function uniqueChars(str) {
  return String.prototype.concat.call(...new Set(str));
}

function fetchFonts() {
  fs.readFile('data/greetings.json', (err, data) => {
    if (err) throw err;
    const json = JSON.parse(data);
    const fontmap = {};
    json.greetings.forEach((greeting) => {
      const font = greeting.font.replace(/ /g, '+');
      const message = greeting.message.replace(/\s+/g, '');
      if (!fontmap[font]) {
        fontmap[font] = message;
      } else {
        fontmap[font] += uniqueChars(message);
      }
    });

    const fontUrls = [];
    for (const font in fontmap) {
      const url = `https://fonts.googleapis.com/css?family=${font}:${fontWeight}&text=${querystring.escape(uniqueChars(fontmap[font]))}&display=swap`;
      fontUrls.push(url);
    }

    let concatenatedFonts = '';
    let completedRequests = 0;
    fontUrls.forEach((url) => {
      console.log("fetchfonts.js :: Fetching " + url + "...");
      https.get(url, (res) => {
        let data = '';
        res.on('data', (chunk) => {
          data += chunk;
        });
        res.on('end', () => {
          concatenatedFonts += data + '\n';
          completedRequests++;
          if (completedRequests === fontUrls.length) {
            fs.writeFileSync('css/webfonts.css', concatenatedFonts, { flag: 'w' });
          }
        });
      }).on('error', (err) => {
        console.log('fetchfonts.js :: Error: ' + err.message);
      });
    });
  console.log('fetchfonts.js :: Successfully wrote to css/webfonts.css');  
  });
}

module.exports = { fetchFonts };
