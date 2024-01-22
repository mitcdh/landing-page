// Listen for the DOMContentLoaded event to ensure the DOM is fully loaded before executing the script
document.addEventListener("DOMContentLoaded", function() {
  // Fetch the background JSON and set the background, or handle errors if the fetch fails
  fetchBackgroundJSON()
    .then(setBackground)
    .catch(handleJSONFetchError);
});

// Function to fetch the background JSON data
function fetchBackgroundJSON() {
  return fetch('data/backgrounds.json')
    .then(response => response.json());
}

// Function to set the background images
function setBackground(data) {
  const glitchImgs = document.querySelectorAll(".glitch__img"); // Select all elements with the 'glitch__img' class
  const background = data.backgrounds[Math.floor(data.backgrounds.length * Math.random())]; // Randomly select a background
  glitchImgs.forEach(glitchImg => {
    glitchImg.style.backgroundImage = `url(${background.src})`; // Set background-image
    glitchImg.style.backgroundPosition = background.position; // Set background-position
  });
}

// Function to handle errors during the JSON fetch
function handleJSONFetchError(error) {
  console.error('Error fetching backgrounds:', error);
}
