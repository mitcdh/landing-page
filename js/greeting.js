$(function() {
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

  return browserLocales.map(locale => {
    const trimmedLocale = locale.trim();

    return opt.languageCodeOnly
      ? trimmedLocale.split(/-|_/)[0]
      : trimmedLocale;
  });
}
	
function shuffleArray(a) {
	let j, t;
	for(let i = a.length - 1; i > 0; i--) {
  		j = Math.floor(Math.random() * (i + 1));
  		t = a[i];
  		a[i] = a[j];
  		a[j] = t;
	}
}

function Type() { 
	var text =  greetingContents[currentGreeting][0].substring(0, greetingIndex + 1);
	textElement.style.fontFamily = greetingContents[currentGreeting][2];
	textElement.innerHTML = text;
	greetingIndex++;
	if(text === greetingContents[currentGreeting][0]) {
		cursorElement.style.display = 'none';
		if(greetingContents.length > 1)
		{
			clearInterval(currentInterval);
			setTimeout(function() {
				currentInterval = setInterval(Delete, 50);
			}, 3000);
		}
	}
}

function Delete() {
	var text =  greetingContents[currentGreeting][0].substring(0, greetingIndex - 1);
	textElement.innerHTML = text;
	greetingIndex--;
	if(text === '') {
		clearInterval(currentInterval);
		if(currentGreeting == (greetingContents.length - 1))
			currentGreeting = 0;
		else
			currentGreeting++;
		
		greetingIndex = 0;
		setTimeout(function() {
			cursorElement.style.display = 'inline-block';
			currentInterval = setInterval(Type, 100);
		}, 200);
	}
}
	
var currentGreeting = 0;
var currentInterval;
var greetingIndex = 0;

var textElement = document.querySelector(".greeting__text");
var cursorElement = document.querySelector(".greeting__cursor");
var browserLanguages = getBrowserLocales({languageCodeOnly: true});
var uniqueLanguages = (typeof browserLanguages !== "undefined") ? Array.from(new Set(browserLanguages)) : ["en"];
	
textElement.innerHTML = "";

// Greetings and language codes
var greetingContents = [ 
	["G'day Mates", "en", "'Noto Sans', sans-serif"],
	["Ciao Amici", "it", "'Noto Sans', sans-serif"],
	["Salut les Amis", "fr", "'Noto Sans', sans-serif"],
	["¡Hola Amigos!", "es", "'Noto Sans', sans-serif"],
	["Привет друзья", "ru", "'Noto Sans', sans-serif"],
	["大家好", "zh", "'Noto Sans TC', sans-serif"],
	["مرحبا اصدقء", "ar", "'Noto Sans Arabic', sans-serif"],
	["Servus, Liebe Freunde", "de", "'Noto Sans', sans-serif"],
	["Alô Galera", "pt", "'Noto Sans', sans-serif"],
	["안녕하세요 친구", "ko", "'Noto Sans KR', sans-serif"],
	["皆さん、こんにちは", "ja", "'Noto Sans JP', sans-serif"],
	["Hej Mina Vänner", "sv", "'Noto Sans', sans-serif"],
	["Selam Dostlar", "tr", "'Noto Sans', sans-serif"],
	["Pozdravljeni Prijatelji", "sl", "'Noto Sans', sans-serif"],
];

// Before we start shuffle all greetings to show no preferences 
shuffleArray(greetingContents);

// Now go through the users languages in reverse order and move matches to the front
uniqueLanguages.reverse().forEach(language =>
	greetingContents.forEach(function(greeting,i)
	{
		if(greeting[1] === language)
		{
			greetingContents.splice(i, 1);
			greetingContents.unshift(greeting);
		}
	})
);

// Start the type/delete cycle
currentInterval = setInterval(Type, 100);
});
