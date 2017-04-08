// Here is the starting point for your application code.
// All stuff below is just to show you how it works. You can delete all of it.

// Use new ES6 modules syntax for everything.
import os from 'os'; // native node.js module
import { remote } from 'electron'; // native electron module
import jetpack from 'fs-jetpack'; // module loaded from npm
import env from './env';
var SerialPort = require("serialport");
window.jQuery = window.$ = require('jquery');
var Chart = require('chart.js')
import {date, 
        time, 
        consoleLog, 
        listMediaDevices, 
        consoleError, 
        listSerialDevices, 
        connectRoutine, 
        consoleSuccess} from './custom_modules/utility';
import {graphInit, graphAddDatapoint} from './custom_modules/graph'
import {SerialInit, getDataPoint} from './custom_modules/serialpull';
var JsonDB = require('node-json-db');
const CsvDb = require('csv-db');

console.log('Loaded environment variables:', env);

var app = remote.app;
var appDir = jetpack.cwd(app.getAppPath());

console.log('The author of this app is:', appDir.read('package.json', 'json').author);


document.addEventListener('DOMContentLoaded', function () {
    document.getElementById('platform-info').innerHTML = os.platform();

    //setting initial time/date
    document.getElementById('date').innerHTML = date();
    document.getElementById('time').innerHTML = time();

    var attitude = $.flightIndicator('#attitude', 'attitude', {roll:0, pitch:0, size:325, showBox : true});
    var heading = $.flightIndicator('#heading', 'heading', {heading:0, showBox:true, size:325});
    var altimeter = $.flightIndicator('#altimeter', 'altimeter', {size:325, showBox : true});

    //listing available serial ports
    listSerialDevices();

    //listing available camera devices
    listMediaDevices();

    var increment = 0;
    graphInit();

    setInterval(function(){
        graphAddDatapoint(30*Math.sin(increment/10));
        increment++;
    }, 50);

    document.getElementById("connect-btn").addEventListener('click', function (e) {
        
            if(document.getElementsByClassName('lessright')[0] == null){
                //connect to the selected devices
                var port = connectRoutine(SerialInit, attitude, heading, altimeter);
                document.onkeydown = checkKey;

                function checkKey(e) {

                    e = e || window.event;

                    if (e.keyCode == '38') {
                        console.log('up');
                    }
                    else if (e.keyCode == '40') {
                        console.log('down');
                    }
                    else if (e.keyCode == '37') {
                        console.log('left');
                    }
                    else if (e.keyCode == '39') {
                        console.log('right');
                    }

                }
                
                var currentDataType;
                //datalogger things
                 $('.dropdown-data li > a').click(function (e){
                    $('.dropdown-status').html(this.innerHTML+' <span class="caret"></span>');
                    currentDataType = this.innerHTML;
                    console.log(currentDataType);
                });

                document.getElementById("logging").addEventListener('click', function (e) {
                    var i = 0;
                    if(document.getElementById("logging").innerText == "Start Logging"){
                        var input = document.getElementById("file-location").value;
                        consoleLog("Datalogger starting");

                        if(currentDataType == "JSON"){
                            var db = new JsonDB (input, true, true);
                            document.getElementById("logging").innerText = "Stop Logging";
                            document.getElementById("logging").className = "btn btn-danger button-style";
                            window.logger = setInterval(function () {
                                        console.log(i);
                                        db.push(("/datapoint" + i), getDataPoint(), false);
                                        i++;
                                    }, 500);
                            consoleSuccess("Datalogger successfully started (Format: JSON)");
                        }

                        else if(currentDataType == "CSV"){
                            /*
                            const csvDb = new CsvDb(input, ['altitude', 
                                                            'current', 
                                                            'dustconc', 
                                                            'endbyte', 
                                                            'heading', 
                                                            'pressure', 
                                                            'startbyte1', 
                                                            'startbyte2', 
                                                            'temperature', 
                                                            'voltage_cell1', 
                                                            'voltage_cell2', 
                                                            'voltage_cell3', 
                                                            'windspeed']);

                            window.logger = setInterval(function () {
                                console.log(i);
                                csvDb.insert(getDataPoint()).then((data) => {
                                    //console.log(data);
                                    }, (err) => {
                                    console.log(err);
                                });
                                i++;
                            }, 500);*/
                            consoleError("Feature not enabled yet");
                        }

                        else if(currentDataType == "Cloud database"){
                            consoleError("Feature not enabled yet");
                        }
                        else {
                            consoleError("No format selected");
                        }
                    }
                    else if(document.getElementById("logging").innerText == "Stop Logging"){
                        consoleLog("Stopping datalogger");
                        document.getElementById("logging").innerText = "Start Logging";
                        document.getElementById("logging").className = "btn btn-success button-style";
                        clearInterval(window.logger);
                        consoleSuccess("Stopped daatalogger")
                    }
                });
            }
            else {
                //gonna be lazy and reload the page for a disconnect
                location.reload();
            }

        }); 
    });