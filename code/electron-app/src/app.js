// Here is the starting point for your application code.
// All stuff below is just to show you how it works. You can delete all of it.

// Use new ES6 modules syntax for everything.
import os from 'os'; // native node.js module
import { remote } from 'electron'; // native electron module
import jetpack from 'fs-jetpack'; // module loaded from npm
import env from './env';
//var SerialPort = require("serialport");

//import { greet } from './hello_world/hello_world'

console.log('Loaded environment variables:', env);

var app = remote.app;
var appDir = jetpack.cwd(app.getAppPath());

console.log('The author of this app is:', appDir.read('package.json', 'json').author);

//user code starts here
var today = new Date();
var dd = today.getDate();
var mm = today.getMonth();
var yyyy = today.getFullYear();
var hour = today.getHours();
var minutes = today.getMinutes();
var seconds = today.getSeconds();
var day;

if(dd < 10){
    dd = '0' + dd;
}

if(mm < 10){
    mm = '0' + mm;
}

if (hour < 12){
    day = 'AM';
}
else {
    day = 'PM'
}

if (hour > 12){
    hour = hour - 12;
}

if (hour < 10){
    hour = '0' + hour;
}

if (hour == '00'){
    hour = '12';
}

if(minutes < 10){
    minutes = '0' + minutes;
}

if(seconds < 10){
    seconds = '0' + seconds;
}

today = dd + '/' + mm + '/' + yyyy;
var time = hour + ':' + minutes + ':' + seconds + ' ' + day;
 
 /*SerialPort.list(function(err ,ports) {
    ports.forEach(function(port) {
        console.log(port.comName);
        console.log(port.pnpId);
        console.log(port.manufacturer);
    });
 });*/

document.addEventListener('DOMContentLoaded', function () {
    //document.getElementById('greet').innerHTML = greet();
    document.getElementById('platform-info').innerHTML = os.platform();
    console.log('I am actually here');
    console.log(mm);
    console.log(today);
    console.log(time);
    document.getElementById('date').innerHTML = today;
    document.getElementById('time').innerHTML = time;
});
