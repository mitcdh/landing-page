# Landing Page for mitcdh
This is just a basic landing page for someone connecting to the domain I use in my personal email address. It is written entirely in HTML/CSS/JS and as minimally as possible as I wanted it to be static to host on, for example, Cloudflare Pages. There are a few things that I did find kind of fun.

## User-facing Code
* `background.js`: Randomly selects a picture and assigns `background-position` based on what I thought would look best.
* `greeting.js`: Types out a greeting sorted by the viewer's language preferences (then random) obtained from `navigator.languages`.
* `home.css`: Uses Codrops' CSSGlitchEffect to glitch the screen after a time; also includes social icons from fa with particular effort in using a gradient colour for Instagram.

## Build Scripts
* `fetchfonts.js`: Fetches minimal versions of the Google fonts in `greeting.json` with only the required characters.
* `parsebg.js`: Generates the list of backgrounds in `backgrounds.json` from filenames in `/bg/`.

## Links
* Blog about this: https://blog.mitcdh.au/p/building-a-custom-landing-page
* Page itself: https://mitcdh.au
