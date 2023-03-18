$(function () {
  // getBrowserLocales adapted from: https://phrase.com/blog/posts/detecting-a-users-locale/#Client-side_The_navigatorlanguages_Object
  // Typing adapted from: https://usefulangle.com/post/75/typing-effect-animation-javascript-css

  function getBrowserLocales(options = {}) {
    const defaultOptions = {
      languageCodeOnly: false,
    };

    const opt = {
      ...defaultOptions,
      ...options,
    };

    const browserLocales =
      navigator.languages === undefined
        ? [navigator.language]
        : navigator.languages;

    if (!browserLocales) {
      return undefined;
    }

    return browserLocales.map((locale) => {
      const trimmedLocale = locale.trim();

      return opt.languageCodeOnly
        ? trimmedLocale.split(/-|_/)[0]
        : trimmedLocale;
    });
  }

  function shuffleArray(a) {
    let j, t;
    for (let i = a.length - 1; i > 0; i--) {
      j = Math.floor(Math.random() * (i + 1));
      t = a[i];
      a[i] = a[j];
      a[j] = t;
    }
  }

  function Type() {
    var text = greetingContents[currentGreeting].message.substring(
      0,
      greetingIndex + 1
    );
    textElement.style.fontFamily =
      "'" + greetingContents[currentGreeting].font + "', sans-serif";
    textElement.innerHTML = text;
    greetingIndex++;
    if (text === greetingContents[currentGreeting].message) {
      cursorElement.style.display = "none";
      if (greetingContents.length > 1) {
        clearInterval(currentInterval);
        setTimeout(function () {
          currentInterval = setInterval(Delete, 50);
        }, 3000);
      }
    }
  }

  function Delete() {
    var text = greetingContents[currentGreeting].message.substring(
      0,
      greetingIndex - 1
    );
    textElement.innerHTML = text;
    greetingIndex--;
    if (text === "") {
      clearInterval(currentInterval);
      if (currentGreeting == greetingContents.length - 1) {
        currentGreeting = 0;
      } else {
        currentGreeting++;
      }
      greetingIndex = 0;
      setTimeout(function () {
        cursorElement.style.display = "inline-block";
        currentInterval = setInterval(Type, 100);
      }, 200);
    }
  }

  async function getGreetings() {
    try {
      const response = await fetch("js/greetings.json");
      const data = await response.json();
      return data.greetings.map((greeting) => ({
        message: greeting.message,
        code: greeting.code,
        font: greeting.font,
      }));
    } catch (error) {
      console.error(error);
      return [
        {
          message: "Hello World!",
          code: "en",
          font: "sans-serif",
        },
      ];
    }
  }

  async function asyncRunner() {
    // Greetings and language codes
    greetingContents = await getGreetings();

    // Before we start shuffle all greetings to show no preferences
    shuffleArray(greetingContents);

    // Move greetings that match the user's languages to the front of the array
    uniqueLanguages.reverse().forEach((language) =>
      greetingContents.forEach(function (greeting, i) {
        if (greeting.code === language) {
          greetingContents.splice(i, 1);
          greetingContents.unshift(greeting);
        }
      })
    );

    // Start the type/delete cycle
    currentInterval = setInterval(Type, 100);
  }

  var currentGreeting = 0;
  var currentInterval;
  var greetingIndex = 0;

  var textElement = document.querySelector(".greeting__text");
  var cursorElement = document.querySelector(".greeting__cursor");
  var browserLanguages = getBrowserLocales({ languageCodeOnly: true });
  var uniqueLanguages =
    typeof browserLanguages !== "undefined"
      ? Array.from(new Set(browserLanguages))
      : ["en"];

  var greetingContents = [];

  textElement.innerHTML = "";

  asyncRunner();
});
