// Here is the starting point for your application code.
// All stuff below is just to show you how it works. You can delete all of it.

// Use new ES6 modules syntax for everything.
import os from 'os'; // native node.js module
import { remote } from 'electron'; // native electron module
import jetpack from 'fs-jetpack'; // module loaded from npm
import env from './env';
var SerialPort = require("serialport");
import { date, time, consoleLog, listMediaDevices, consoleError } from './custom_modules/utility'

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
        document.getElementById('com-port').innerHTML = document.getElementById('com-port').innerHTML + "<option>" + port.comName + "</option>";
        console.log(port.comName);
        console.log(port.pnpId);
        console.log(port.manufacturer);
            });
        });

    //listing available camera devices
    listMediaDevices();

    //Connect routine
    document.getElementById("connect-btn").addEventListener('click', function (e) {
        var e = document.getElementById('camera-input');
        var f = document.getElementById('baudrate');
        var g = document.getElementById('com-port');

        var selCam = e.options[e.selectedIndex].text;
        var selBaudrate = f.options[f.selectedIndex].text;
        var selCOMport = g.options[g.selectedIndex].text;

        console.log(selCam + ":" + selBaudrate + ":" + selCOMport);

        document.getElementById('date').innerHTML = date();
        document.getElementById('time').innerHTML = time();

        //no camera selected error
        if(selCam == "Camera"){
            consoleError('No camera input selected');
            return;
        }
        //no baudrate selected error
        if(selBaudrate == "Baud Rate:"){
            consoleError('No baudrate selected');
            return;
        }
        /*no COM port selected error
        if(selCOMport == "COM port"){
            consoleError('No COM port selected');
            return;
        }*/

        //attempting to connect to camera
        consoleLog('Connecting to camera...');
        navigator.mediaDevices.enumerateDevices().then(function(devices) {
            devices.forEach(function(device) {
                
                if (device.label == selCam) {
                    var constraints = { deviceId: { exact: device.deviceId } };
                    navigator.webkitGetUserMedia({video: constraints},
                        function(stream) {
                            document.getElementById('camera').src = URL.createObjectURL(stream);
                            consoleLog('Connected to: ' + device.label)
                        },
                        function() {
                            consoleError('The selected camera could not be accessed');
                        }
                    );

                }
            });
        })

        document.getElementById('connect-btn').src = "assets/unlink.png";
        document.getElementById('connect-btn').id = "disconnect-btn";
        document.getElementsByClassName('right')[0].className = "lessright";
        document.getElementsByClassName('lessright')[0].innerHTML = "Disconnect";

    });
});
