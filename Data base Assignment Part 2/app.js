document.addEventListener("DOMContentLoaded", function() {
  fetch('data.php')
    .then(response => response.json())
    .then(data => {
      const labels = data.map(item => item.time_received);
      const temps = data.map(item => parseFloat(item.temperature));

      const ctx = document.getElementById('myChart').getContext('2d');
      new Chart(ctx, {
        type: 'bar', // Change this to 'line' for line graph
        data: {
          labels: labels,
          datasets: [{
            label: 'Temperature (°C)',
            data: temps,
            backgroundColor: 'rgba(0, 200, 0, 0.5)',  // Green
            borderColor: 'rgba(0, 128, 0, 1)',
            borderWidth: 1
          }]
        },
        options: {
          responsive: true,
          scales: {
            x: { title: { display: true, text: 'Time' } },
            y: { title: { display: true, text: 'Temperature (°C)' } }
          }
        }
      });
    });
});
