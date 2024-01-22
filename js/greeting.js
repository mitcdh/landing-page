// getBrowserLocales adapted from: https://phrase.com/blog/posts/detecting-a-users-locale/#Client-side_The_navigatorlanguages_Object
// Typing adapted from: https://usefulangle.com/post/75/typing-effect-animation-javascript-css

// Wait for the DOM to be fully loaded before running the script
document.addEventListener("DOMContentLoaded", function() {
  // Select DOM elements for the text and cursor
  const textElement = document.querySelector(".greeting__text");
  const cursorElement = document.querySelector(".greeting__cursor");
  textElement.innerHTML = "";

  // Fetch greetings from a JSON file
  async function getGreetings() {
    try {
      const response = await fetch("data/greetings.json");
      return (await response.json()).greetings;
    } catch (error) {
      // Log the error and return a default greeting in case of fetch failure
      console.error('Error fetching greetings:', error);
      return [{ message: "Hello World!", code: "en", font: "sans-serif" }];
    }
  }

  // Get the browser's preferred locales
  function getBrowserLocales(options = { languageCodeOnly: true }) {
    return (navigator.languages || [navigator.language])
      .map(locale => options.languageCodeOnly ? locale.split(/-|_/)[0] : locale.trim());
  }

  // Shuffle the elements of an array
  function shuffleArray(array) {
    for (let i = array.length - 1; i > 0; i--) {
      let j = Math.floor(Math.random() * (i + 1));
      [array[i], array[j]] = [array[j], array[i]];
    }
  }

  // Prioritize greetings based on the user's browser languages
  function prioritizeGreetings(greetings, languages) {
    return greetings.sort((a, b) => languages.indexOf(b.code) - languages.indexOf(a.code));
  }

  // Main function to run the typing animation
  async function asyncRunner() {
    let greetings = await getGreetings();
    shuffleArray(greetings);
    const userLanguages = Array.from(new Set(getBrowserLocales()));
    greetings = prioritizeGreetings(greetings, userLanguages);

    // Initialize and start the typing animation
    new TypingAnimation(textElement, cursorElement, greetings).start();
  }

  asyncRunner();
});

// TypingAnimation class to handle the typing effect
class TypingAnimation {
  constructor(textElement, cursorElement, greetings, typingSpeed = 100, deletingSpeed = 50) {
    this.textElement = textElement;
    this.cursorElement = cursorElement;
    this.greetings = greetings;
    this.currentGreeting = 0;
    this.greetingIndex = 0;
    this.typingSpeed = typingSpeed; // Speed of typing in ms
    this.deletingSpeed = deletingSpeed; // Speed of deleting in ms
  }

  // Method to handle the typing effect
  type() {
    const greeting = this.greetings[this.currentGreeting];
    let text = greeting.message.substring(0, this.greetingIndex + 1);
    this.textElement.style.fontFamily = `'${greeting.font}', sans-serif`;
    this.textElement.innerHTML = text;
    this.greetingIndex++;

    if (text === greeting.message) {
      this.cursorElement.style.display = "none";
      setTimeout(() => this.delete(), 3000); // Pause before deleting
    } else {
      setTimeout(() => requestAnimationFrame(() => this.type()), this.typingSpeed);
    }
  }

  // Method to handle the deleting effect
  delete() {
    const greeting = this.greetings[this.currentGreeting];
    let text = greeting.message.substring(0, this.greetingIndex - 1);
    this.textElement.innerHTML = text;
    this.greetingIndex--;

    if (text === "") {
      this.currentGreeting = (this.currentGreeting + 1) % this.greetings.length;
      this.greetingIndex = 0;
      this.cursorElement.style.display = "inline-block";
      setTimeout(() => this.type(), 200); // Pause before typing next greeting
    } else {
      setTimeout(() => requestAnimationFrame(() => this.delete()), this.deletingSpeed);
    }
  }

  // Start the typing animation
  start() {
    requestAnimationFrame(() => this.type());
  }
}
