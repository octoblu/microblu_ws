Node.js client demo
===================

This client simply connects through skynet to a remote arduino that has been loaded with the skynet firmware and then toggles pin 13 off and on.



* Edit the index.js file to point to the uuid of the arduino you want to control:
```
var sendId = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
```


* From inside this folder, install the client's dependencies:
```
npm install
```


* launch!
```
node index.js
```
