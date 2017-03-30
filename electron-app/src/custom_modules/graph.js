var Chart = require('chart.js')

var myLineChart;
var latestLabel = 0;

export var graphInit = function () {
    var ctx = document.getElementById("myChart");
    var data = {
    labels: [],
    datasets: [
        {
            label: "My First dataset",
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

        myLineChart = new Chart(ctx, {
        type: 'line',
        data: data,
        options: {animationSteps: 15, scales:{xAxes: [{display: false}]}, tooltips: {enabled: false}}
    });

};

export var graphAddDatapoint = function (label, data){
    myLineChart.data.labels.push(latestLabel);
    myLineChart.data.datasets[0].data.push(data);

    if(latestLabel > 100){
        myLineChart.data.labels.splice(0,1);
        myLineChart.data.datasets[0].data.splice(0,1);
    }

    latestLabel++;
    myLineChart.update();
};