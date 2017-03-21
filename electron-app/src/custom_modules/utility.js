var SerialPort = require("serialport");
let struct = require('cstruct');
var jquery = require('jquery');

const Sensors = struct `
    int16_t humidity, temperature;
    int32_t pressure;
    uint16_t altitude, particulate_matter, ozone_concentration, wind_speed;
    float hmc_x, hmc_y, hmc_z;
`

const vc = struct `
    uint16_t voltage_3v3, voltage_5v, current, voltage_cell1, voltage_cell2, voltage_cell3, voltage_batt;
`

const Control = struct `
    uint16_t generic_cmd, pan_pos, tilt_pos;
`

const payload = struct `
    uint16_t sanity_check1;
    uint32_t type;
    int16_t humidity, temperature;
    int32_t pressure;
    uint16_t altitude, particulate_matter, ozone_concentration, wind_speed;
    float hmc_x, hmc_y, hmc_z;
    uint16_t voltage_3v3, voltage_5v, current, voltage_cell1, voltage_cell2, voltage_cell3, voltage_batt;
    uint16_t generic_cmd, pan_pos, tilt_pos;
    uint16_t crc, sanity_check2;
`

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
        /*
        if(selCOMport == "COM port"){
            consoleError('No COM port selected');
            return;
        }*/

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
        /*
        //Connect to serialport
        var port = new SerialPort(selCOMport, { baudRate: selBaudrate,
                                                parser: SerialPort.parsers.byteDelimiter([190,186])}, function (err) {
            if (err) {
                return console.log('Error:', err.message);
            }
        });

        let thing = { sanity_check1:0xcafe, sanity_check2:0xbabe, crc:0xd00d, type:1 };
        console.log(payload.writeLE(thing));
        console.log(typeof payload.writeLE(thing));

        port.on('data', function (data) {
            console.log(toHexString(data)+"\n");
            //let raw = new Buffer(data, 'binary');
            //let procdata = payload.readLE(raw);
            //console.log(procdata);
        });*/

        document.getElementById('connect-btn').src = "assets/unlink.png";
        document.getElementsByClassName('right')[0].className = "lessright";
        document.getElementsByClassName('lessright')[0].innerHTML = "Disconnect";
}
