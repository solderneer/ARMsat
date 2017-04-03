var Chart = require('chart.js')

var Chart1;
var Chart2;
var chart1Label = 0;
var chart2Label = 0;

export var graphInit = function () {
    var ctx = document.getElementById("Graph1");
    var data = {
    labels: [],
    datasets: [
        {
            label: "Pressure",
            fill: false,
            lineTension: 0,
            backgroundColor: "rgba(75,192,192,0.4)",
            borderColor: "rgba(75,192,192,1)",
            borderCapStyle: 'butt',
            borderDash: [0],
            borderDashOffset: 0.0,
            borderJoinStyle: 'miter',
            pointBorderColor: "rgba(75,192,192,1)",
            pointBackgroundColor: "#fff",
            pointBorderWidth: 1,
            pointHoverRadius: 5,
            pointHoverBackgroundColor: "rgba(75,192,192,1)",
            pointHoverBorderColor: "rgba(220,220,220,1)",
            pointHoverBorderWidth: 2,
            pointRadius: 1,
            pointHitRadius: 10,
            data: [0],
            spanGaps: false,
         }
        ]
    };
        Chart1 = new Chart(ctx, {
        type: 'line',
        data: data,
        options: {animationSteps: 15, scales:{xAxes: [{display: false}]}, tooltips: {enabled: false}}
    });
};

export var graphAddDatapoint = function (data){
    Chart1.data.labels.push(chart1Label);
    Chart1.data.datasets[0].data.push(data);

    if(chart1Label > 100){
        Chart1.data.labels.splice(0,1);
        Chart1.data.datasets[0].data.splice(0,1);
    }

    chart1Label++;
    Chart1.update();
};

export var graphChangeDataType = function() {

};