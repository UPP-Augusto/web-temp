const char CHARTS_HTML[] PROGMEM = R"=====(
<!doctype html>
<html lang="es">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <title>Monitor de Temperatura</title>
    <link href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { padding-top: 5rem; background-color: #f5f5f5; }
        .chart-container { 
            position: relative; 
            margin: auto; 
            height: 60vh; 
            width: 90vw;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
    </style>
</head>
<body>
    <nav class="navbar navbar-dark fixed-top bg-dark flex-md-nowrap p-0 shadow">
        <a class="navbar-brand col-sm-3 col-md-2 mr-0" href="#">Monitor SP2</a>
    </nav>

    <div class="container text-center">
        <div class="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pt-3 pb-2 mb-3 border-bottom">
            <h1 class="h2">Monitor en Tiempo Real</h1>
            <button type="button" class="btn btn-outline-danger" onclick="clearData()">Borrar Historial</button>
        </div>

        <div class="chart-container">
            <canvas id="myChart"></canvas>
        </div>
        <p class="mt-3 text-muted">Actualización automática cada 10 segundos</p>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
        let myChartInstance = null;

        async function loadData() {
            try {
                // nocache para evitar que el navegador use datos viejos
                const response = await fetch('/api/data?nocache=' + new Date().getTime());
                const data = await response.text();
                
                // Procesar CSV
                const rows = data.split('\n').filter(row => row.trim() !== '' && !row.startsWith('timestamp')); 
                
                const labels = [];
                const temps = [];
                const hums = [];

                rows.forEach(row => {
                    const cols = row.split(',');
                    if(cols.length >= 3) {
                        // Calcular minutos
                        const mins = Math.floor(cols[0] / 1000 / 60); 
                        labels.push(mins + 'm'); 
                        temps.push(parseFloat(cols[1]));
                        hums.push(parseFloat(cols[2]));
                    }
                });

                if (myChartInstance) {
                    // Actualizar datos existentes
                    myChartInstance.data.labels = labels;
                    myChartInstance.data.datasets[0].data = temps;
                    myChartInstance.data.datasets[1].data = hums;
                    myChartInstance.update();
                } else {
                    // Crear nueva
                    renderChart(labels, temps, hums);
                }

            } catch (error) {
                console.error("Error:", error);
            }
        }

        function renderChart(labels, temps, hums) {
            var ctx = document.getElementById('myChart').getContext('2d');
            myChartInstance = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: labels,
                    datasets: [{
                        label: 'Temperatura (°C)',
                        data: temps,
                        borderColor: 'rgb(255, 99, 132)',
                        backgroundColor: 'rgba(255, 99, 132, 0.1)',
                        fill: true,
                        tension: 0.3
                    }, {
                        label: 'Humedad (%)',
                        data: hums,
                        borderColor: 'rgb(54, 162, 235)',
                        backgroundColor: 'rgba(54, 162, 235, 0.1)',
                        fill: true,
                        tension: 0.3
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: { duration: 0 },
                    scales: { y: { beginAtZero: true } }
                }
            });
        }

        async function clearData() {
            if(confirm('¿Seguro que deseas borrar el historial?')) {
                await fetch('/api/clear');
                loadData();
            }
        }
        
        loadData();
        setInterval(loadData, 10000);
    </script>
</body>
</html>
)=====";
