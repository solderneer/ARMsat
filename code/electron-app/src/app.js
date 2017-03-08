// Here is the starting point for your application code.
// All stuff below is just to show you how it works. You can delete all of it.

// Use new ES6 modules syntax for everything.
import os from 'os'; // native node.js module
import { remote } from 'electron'; // native electron module
import jetpack from 'fs-jetpack'; // module loaded from npm
import env from './env';
var SerialPort = require("serialport");
import { date, time } from './hello_world/datetime'

console.log('Loaded environment variables:', env);

var app = remote.app;
var appDir = jetpack.cwd(app.getAppPath());

console.log('The author of this app is:', appDir.read('package.json', 'json').author);

//user code starts here
 SerialPort.list(function(err ,ports) {
    ports.forEach(function(port) {
        document.getElementById('COM-port').innerHTML = document.getElementById('COM-port').innerHTML + "<option>" + port.comName + "</option>";
        console.log(port.comName);
        console.log(port.pnpId);
        console.log(port.manufacturer);
    });
 });

document.addEventListener('DOMContentLoaded', function () {
    document.getElementById('platform-info').innerHTML = os.platform();
    console.log('I am actually here');
    document.getElementById('date').innerHTML = date();
    document.getElementById('time').innerHTML = time();
});
