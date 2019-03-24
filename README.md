# ScriptMauzer

Simple yet robust JavaScript interpreter based on Qt4. Turn JavaScript into a general purpose scripting language. 

# Usage
```
ScriptMauzer.exe Script.js
```
# License
GPL (https://en.wikipedia.org/wiki/GNU_General_Public_License)

# Example

```javascript
/*-- Include some external scrips with qs.script.include()----*/
qs.script.include("scripts/lib.js");

/*-- Say something with classic console.log() ----*/
console.log('Sunning main script');

/*-- Create some regular javascript object ----*/
var today = new Date();	

/*-- Save plain text file with generated name and text "Simple text file" ----*/
save('index_'+today.getTime()+'.html','Simple text file.');

/*-- ScriptMauzer will loop through object properties, look for eponymous tags in template, replace them with corresponding property content ----*/
var object = {
	lang:'en-US',/* 	looking for <lang> tag to replace it with 'en-US'*/
	myTitle:'My Title',/* 	looking for <myTitle> tag to replace it with 'My Title' text*/
	content:'My text',/* 	looking for <content> tag to replace it with 'My text'*/
	myFooter:'My footer'/* 	looking for <myFooter> tag to replace it with 'My footer' text*/
};
/*-- Save HTML file using main.html as a template. ----*/
save('index.html',parser.applyTemplate('templates/main.html',object));
```
