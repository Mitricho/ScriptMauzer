# ScriptMauzer

Simple yet robust JavaScript interpreter based on Qt4. Turn JavaScript into a general purpose scripting language. 

# Usage
```
ScriptMauzer.exe Script.js
```
# License
GPL (https://en.wikipedia.org/wiki/GNU_General_Public_License)

# Example

```
/*-- Include some external scrips with qs.script.include()----*/
qs.script.include("scripts/lib.js");

/*-- Say something with classic console.log() ----*/
console.log('Sunning main script');

/*-- Create some regular javascript object ----*/
var today = new Date();	

/*-- Save plain text file with generated name and text "Simple text file" ----*/
save('index_'+today.getTime()+'.html','Simple text file.');

save('index.html',parser.applyTemplate('templates/main.html',{
	lang:'en-US',
	myTitle:'My Title',
	content:'My text',
	myFooter:'My footer'
}));
```
