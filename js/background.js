document.addEventListener("DOMContentLoaded", function() {
  fetch('data/backgrounds.json')
    .then(response => response.json())
    .then(data => {
      const backgrounds = data.backgrounds;
      const glitchImgs = document.querySelectorAll(".glitch__img");
	  const num = Math.floor(backgrounds.length * Math.random());
      glitchImgs.forEach((glitchImg) => {
        glitchImg.style.backgroundImage = "url(" + backgrounds[num].src + ")";
        glitchImg.style.backgroundPosition = backgrounds[num].position;
      });
    })
    .catch(error => {
      console.error('Error fetching backgrounds:', error);
    });
});