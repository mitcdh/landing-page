$(function() {
	var imagesArray = [
			["sea.webp", "center bottom"],
			["beach.webp", "center bottom"],
			["ocean.webp", "center bottom"],
			["river.webp", "center left"],
			["harbour.webp", "center right"],
			["canyon.webp", "center center"],
			["mountain.webp", "center right"],
			["balloon.webp", "center bottom"],
			["desert.webp", "center center"],
			["ruins.webp", "center center"],
			["valley.webp", "center right"],
			["cliff.webp", "center center"],
		],
		num = Math.floor(imagesArray.length * Math.random());
	$(".glitch__img").css({"background-image":"url(img/" + imagesArray[num][0] + ")", "background-position":imagesArray[num][1]});
});
