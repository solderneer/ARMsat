var SerialPort = require("serialport");
var jquery = require('jquery');
var Parser = require('binary-parser').Parser;

var payload = new Parser()
    .endianess('big')
    .uint16('humidity')
    .uint16('temperature')
    .uint32('pressure')
    .uint16('altitude')
    .uint16('dustconc')
    .uint16('heading')
    .uint16('voltage_cell1')
    .uint16('voltage_cell2')
    .uint16('voltage_cell3')
    .uint16('current');

/*var ipHeader = new Parser()
    .endianess('little')
    .uint16('humidity')
    .uint16('temperature')
    .uint32('pressure')
    .uint16('altitude')
    .uint16('heading')
    .uint16('checksu');*/


export var date = function () {
    var today = new Date();
    var dd = today.getDate();
    var mm = today.getMonth() + 1;
    var yyyy = today.getFullYear();

    dd = dd < 10 ? "0" + dd: dd;
    mm = mm < 10 ? "0" + mm: mm;

    return (dd + '/' + mm + '/' + yyyy);
}

export var time = function () {
    var today = new Date();
    var hh = today.getHours();
    var mm = today.getMinutes();
    var ss = today.getSeconds();

    return ((((hh == 0 ? 24 : hh) - 1) % 12 + 1) + ":" +
            (mm < 10 ? "0" + mm: mm) + ":" +
            (ss < 10 ? "0" + ss: ss) + " " +
            ((hh == 0 || hh > 12) ? "PM" : "AM"));
}

export var consoleLog = function (x) {
    document.getElementById('console').innerHTML = x;
}

export var consoleError = function (x) {
    document.getElementById('console').innerHTML = "<span class='red'>" + x +"</span>"
}

export var consoleSuccess = function (x) {
    document.getElementById('console').innerHTML = "<span class='green'>" + x +"</span>"
}

export var listMediaDevices = function () {
    if (!navigator.mediaDevices || !navigator.mediaDevices.enumerateDevices) {
        console.log("enumerateDevices() not supported.");
        return;
    }
    // List cameras and microphones.
    navigator.mediaDevices.enumerateDevices().then(function(devices) {
        devices.forEach(function(device) {
            //console.log(device.kind + ": " + device.label + " id = " + device.deviceId);
            if(device.kind == "videoinput") {
                document.getElementById('camera-input').innerHTML = document.getElementById('camera-input').innerHTML + "<option>" + device.label + "</option>";
            }
        });
    })
    
    .catch(function(err) {
        console.log(err.name + ": " + err.message);
    });
}

export var listSerialDevices = function () {
    SerialPort.list(function(err ,ports) {
    ports.forEach(function(port) {
        document.getElementById('com-port').innerHTML = document.getElementById('com-port').innerHTML + "<option>" + port.comName + "</option>";
        console.log(port.comName);
        console.log(port.pnpId);
        console.log(port.manufacturer);
            });
        });
}

function toHexString(byteArray) {
  return byteArray.map(function(byte) {
    return ('0' + (byte & 0xFF).toString(16)).slice(-2);
  }).join('')
}

export var connectRoutine = function () {
    /**
     * Connect Routine
     */

        document.getElementById('camera').hidden = false;
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
        
        if(selCOMport == "COM port"){
            consoleError('No COM port selected');
            return;
        }

        //Connect to camera
        consoleLog('Connecting to camera...');
        navigator.mediaDevices.enumerateDevices().then(function(devices) {
            devices.forEach(function(device) {
                
                if (device.label == selCam) {
                    var constraints = { deviceId: { exact: device.deviceId } };
                        var stream = navigator.webkitGetUserMedia({video: constraints},
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

        selBaudrate = parseInt(selBaudrate);

        var port = new SerialPort(selCOMport, { baudRate: selBaudrate,
                                                parser: SerialPort.parsers.byteDelimiter([171,205])}, function (err) {
            if (err) {
                return console.log('Error:', err.message);
            }
        });

        port.on('data', function (data) {
            data = toHexString(data);
            //var buf = new Buffer(data, 'hex');
            //payload.parse(buf);
            //console.log(data);
            var buf = new Buffer('117109a40002041e0000182f010329e68feddb694951', 'hex');
            console.log(payload.parse(buf));


        });

        document.getElementById('connect-btn').src = "assets/unlink.png";
        document.getElementsByClassName('right')[0].className = "lessright";
        document.getElementsByClassName('lessright')[0].innerHTML = "Disconnect";

        tabSwitch("home");
        tabSwitch("telemetry");
        tabSwitch("graphing");
        tabSwitch("settings");
}


var tabSwitch = function (y, currentActive) {
    document.getElementById(y).addEventListener('click', function(c){
        var x = document.getElementById(y);
        console.log(x.classList.length);
        for(var i=0; i<x.classList.length; i++){
                console.log(x.classList[i]);
                if(x.classList[i] == "active"){
                    break;
                }
            }
        if(x.classList[i] == "active"){}
            
        else{
            resetTabs();
            
            x.className = x.className + " active";
            switch(x.id) {
                case "home":
                     document.getElementById('Home').hidden = false;
                     break;
                 case "telemetry":
                     document.getElementById('Telemetry').hidden = false;
                     break;   
                case "graphing":
                     document.getElementById('Graphing').hidden = false;
                     break;
                case "settings":
                     document.getElementById('Settings').hidden = false;
                     break;
            }
        }
    });
}

function resetTabs() {
    document.getElementById('home').className = "tab-element";
    document.getElementById('telemetry').className = "tab-element";
    document.getElementById('graphing').className = "tab-element";
    document.getElementById('settings').className = "tab-element";

    document.getElementById('Home').hidden = true;
    document.getElementById('Telemetry').hidden = true;
    document.getElementById('Graphing').hidden = true;
    //document.getElementById('Settings').hidden = true;
}


