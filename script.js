const ctx = document.getElementById('gaugeCanvas').getContext('2d');
const tankEl = document.getElementById('tankValue');
const homeEl = document.getElementById('homeValue');
const alertBanner = document.getElementById('alertBanner');

let tankLitres = 0;
let homeLitres = 0;
let allowedLimit = 100;
let interval;

function drawGauge(value) {
  const max = 100;
  ctx.clearRect(0, 0, 300, 300);

  // Outer Circle
  ctx.beginPath();
  ctx.arc(150, 150, 100, 0.75 * Math.PI, 0.25 * Math.PI, false);
  ctx.lineWidth = 20;
  ctx.strokeStyle = "#ddd";
  ctx.stroke();

  // Progress arc
  const angle = 0.75 * Math.PI + (value / max) * 1.5 * Math.PI;
  ctx.beginPath();
  ctx.arc(150, 150, 100, 0.75 * Math.PI, angle, false);
  ctx.lineWidth = 20;
  ctx.strokeStyle = value > 80 ? "#ff4d4d" : "#00e676";
  ctx.stroke();

  // Text
  ctx.fillStyle = "white";
  ctx.font = "30px sans-serif";
  ctx.textAlign = "center";
  ctx.fillText(`${value} L`, 150, 160);
  ctx.font = "18px sans-serif";
  ctx.fillText("Water Usage", 150, 190);
}

function updateData() {
  const tankIncrement = Math.floor(Math.random() * 5) + 5;
  const homeIncrement = Math.random() < 0.9 ? tankIncrement : 0;

  tankLitres += tankIncrement;
  homeLitres += homeIncrement;

  tankEl.innerText = tankLitres;
  homeEl.innerText = homeLitres;

  drawGauge(homeLitres);

  const leakage = tankLitres - homeLitres;

  if (leakage > 20 || homeLitres > allowedLimit) {
    alertBanner.style.display = "block";
  } else {
    alertBanner.style.display = "none";
  }
}

// Start simulation
drawGauge(0);
interval = setInterval(updateData, 2000);

        const adapter = new STM32DataAdapter({ simulationMode: true });
        
        // Make adapter available globally for debug
        window.waterSystemAdapter = adapter;
        
        // Expose connect function for UI
        window.connectToSTM32 = (url) => {
            if (adapter.isConnected) {
                adapter.disconnect();
            }
            adapter.connect(url || 'ws://localhost:8080');
        };
    });
}

// Start simulation
drawGauge(0);
interval = setInterval(updateData, 2000);
