
/*-- Include other scrips ----*/
qs.script.include("scripts/lib.js");

console.log('Running main script');

var today = new Date();	

save('index_'+today.getTime()+'.html','Simple text file.');

save('index.html',parser.applyTemplate('templates/main.html',{
	lang:'en-US',
	myTitle:'My Title',
	content:'My text',
	myFooter:'My footer'
}));
