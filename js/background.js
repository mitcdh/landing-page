// Listen for the DOMContentLoaded event to ensure the DOM is fully loaded before executing the script
document.addEventListener("DOMContentLoaded", function() {
  // Fetch the background JSON and set the background, or handle errors if the fetch fails
  fetchBackgroundJSON()
    .then(data => {
      const urlParams = new URLSearchParams(window.location.search);
      const bgName = urlParams.get('bg');
      setBackground(data, bgName);
    })
    .catch(handleJSONFetchError);
});

// Function to fetch the background JSON data
function fetchBackgroundJSON() {
  return fetch('data/backgrounds.json')
    .then(response => response.json());
}

// Function to set the background images
function setBackground(data, bgName) {
  const glitchImgs = document.querySelectorAll(".glitch__img");
  let background;

  if (bgName) {
    // Find the background with matching name
    background = data.backgrounds.find(bg => bg.name === bgName);
    // If no matching background found, fall back to random selection
    if (!background) {
      console.warn(`Background "${bgName}" not found, using random background`);
      background = getRandomBackground(data.backgrounds);
    }
  } else {
    // If no bgName provided, use random background
    background = getRandomBackground(data.backgrounds);
  }

  glitchImgs.forEach(glitchImg => {
    glitchImg.style.backgroundImage = `url(${background.src})`;
    glitchImg.style.backgroundPosition = background.position;
  });
}

// Function to get a random background
function getRandomBackground(backgrounds) {
  return backgrounds[Math.floor(backgrounds.length * Math.random())];
}

// Function to handle errors during the JSON fetch
function handleJSONFetchError(error) {
  console.error('Error fetching backgrounds:', error);
}