$(function() {
	fetch('data/backgrounds.json')
	  .then(response => response.json())
	  .then(data => {
		const backgrounds = data.backgrounds;
		const num = Math.floor(backgrounds.length * Math.random());
		$(".glitch__img").css({"background-image":"url(" + backgrounds[num].src + ")", "background-position":backgrounds[num].position});
	  })
	  .catch(error => {
		console.error('Error fetching backgrounds:', error);
	  });
  });
  