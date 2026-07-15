// Locale detection adapted from Phrase's navigator.languages article.
// Typing effect adapted from usefulangle.com/post/75/typing-effect-animation-javascript-css.

const textElement = document.querySelector('.greeting__text');
const cursorElement = document.querySelector('.greeting__cursor');

async function getGreetings() {
  const embeddedGreetings = globalThis.__LANDING_DATA__?.greetings;
  if (embeddedGreetings) return embeddedGreetings;

  try {
    const response = await fetch('data/greetings.json');
    if (!response.ok) throw new Error(`Greeting data request failed (${response.status})`);
    return (await response.json()).greetings;
  } catch (error) {
    console.error('Unable to load greetings:', error);
    return [{ message: 'Hello World!', code: 'en', font: 'sans-serif' }];
  }
}

function getBrowserLanguages() {
  return (navigator.languages || [navigator.language])
    .map((locale) => locale.split(/-|_/)[0].toLowerCase());
}

function shuffle(array) {
  for (let index = array.length - 1; index > 0; index -= 1) {
    const randomIndex = Math.floor(Math.random() * (index + 1));
    [array[index], array[randomIndex]] = [array[randomIndex], array[index]];
  }

  return array;
}

function prioritizeGreetings(greetings, languages) {
  const rank = ({ code }) => {
    const index = languages.indexOf(code.toLowerCase());
    return index === -1 ? Number.POSITIVE_INFINITY : index;
  };

  return greetings.sort((a, b) => rank(a) - rank(b));
}

class TypingAnimation {
  constructor(greetings, typingSpeed = 100, deletingSpeed = 50) {
    this.greetings = greetings;
    this.typingSpeed = typingSpeed;
    this.deletingSpeed = deletingSpeed;
    this.currentGreeting = 0;
    this.characterIndex = 0;
  }

  type() {
    const greeting = this.greetings[this.currentGreeting];

    if (this.characterIndex === 0) {
      textElement.style.fontFamily = `'${greeting.font}', sans-serif`;
    }

    this.characterIndex += 1;
    textElement.textContent = greeting.message.substring(0, this.characterIndex);

    if (this.characterIndex === greeting.message.length) {
      cursorElement.hidden = true;
      window.setTimeout(() => this.delete(), 3000);
      return;
    }

    window.setTimeout(() => this.type(), this.typingSpeed);
  }

  delete() {
    const greeting = this.greetings[this.currentGreeting];
    this.characterIndex -= 1;
    textElement.textContent = greeting.message.substring(0, this.characterIndex);

    if (this.characterIndex === 0) {
      this.currentGreeting = (this.currentGreeting + 1) % this.greetings.length;
      cursorElement.hidden = false;
      window.setTimeout(() => this.type(), 200);
      return;
    }

    window.setTimeout(() => this.delete(), this.deletingSpeed);
  }

  start() {
    this.type();
  }
}

async function initializeGreeting() {
  if (!textElement || !cursorElement) return;

  const greetings = prioritizeGreetings(
    shuffle([...(await getGreetings())]),
    [...new Set(getBrowserLanguages())],
  );

  if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
    const greeting = greetings[0];
    textElement.style.fontFamily = `'${greeting.font}', sans-serif`;
    textElement.textContent = greeting.message;
    cursorElement.hidden = true;
    return;
  }

  textElement.textContent = '';
  new TypingAnimation(greetings).start();
}

initializeGreeting();
