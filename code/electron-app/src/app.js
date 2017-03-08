// Here is the starting point for your application code.
// All stuff below is just to show you how it works. You can delete all of it.

// Use new ES6 modules syntax for everything.
import os from 'os'; // native node.js module
import { remote } from 'electron'; // native electron module
import jetpack from 'fs-jetpack'; // module loaded from npm
import env from './env';
var SerialPort = require("serialport");
import { date, time, consoleLog, listMediaDevices } from './custom_modules/utility'

console.log('Loaded environment variables:', env);

var app = remote.app;
var appDir = jetpack.cwd(app.getAppPath());

console.log('The author of this app is:', appDir.read('package.json', 'json').author);


document.addEventListener('DOMContentLoaded', function () {
    document.getElementById('platform-info').innerHTML = os.platform();

    //setting initial time/date
    document.getElementById('date').innerHTML = date();
    document.getElementById('time').innerHTML = time();

    //listing available serial ports
    SerialPort.list(function(err ,ports) {
    ports.forEach(function(port) {
        document.getElementById('COM-port').innerHTML = document.getElementById('COM-port').innerHTML + "<option>" + port.comName + "</option>";
        console.log(port.comName);
        console.log(port.pnpId);
        console.log(port.manufacturer);
            });
        });

    //listing available camera devices
    listMediaDevices();

    
    document.getElementById("connect-btn").addEventListener('click', function (e) {
        consoleLog('Connecting to camera...');
        navigator.webkitGetUserMedia({video: true},
        function(stream) {
            document.getElementById('camera').src = URL.createObjectURL(stream);
            consoleLog('Connected to camera')
        },
        function() {
            consoleError('The selected camera could not be accessed');
        });

    });
});
