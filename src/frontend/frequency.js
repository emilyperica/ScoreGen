// Ensure the script runs after the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('frequencyCanvas');
    const canvasCtx = canvas.getContext('2d');
  
    // Check for browser compatibility
    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
      alert('getUserMedia is not supported in your browser.');
      return;
    }
  
    // Request access to the microphone
    navigator.mediaDevices.getUserMedia({ audio: true })
      .then(stream => {
        // Create an AudioContext
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
  
        // Create a source from the microphone stream
        const source = audioContext.createMediaStreamSource(stream);
  
        // Create an AnalyserNode
        const analyser = audioContext.createAnalyser();
        analyser.fftSize = 2048; // Determines the frequency resolution
        const bufferLength = analyser.frequencyBinCount;
        const dataArray = new Uint8Array(bufferLength);
  
        // Connect the nodes
        source.connect(analyser);
        // Optionally connect to the destination (speakers)
        // analyser.connect(audioContext.destination);
  
        // Set up the canvas dimensions
        canvas.width = 700;
        canvas.height = 400;
  
        // Function to draw the frequency data
        function draw() {
          requestAnimationFrame(draw);
  
          analyser.getByteFrequencyData(dataArray);
  
          canvasCtx.fillStyle = '#fff';
          canvasCtx.fillRect(0, 0, canvas.width, canvas.height);
  
          const barWidth = (canvas.width / bufferLength) * 2.5;
          let barHeight;
          let x = 0;
  
          for(let i = 0; i < bufferLength; i++) {
            barHeight = dataArray[i];
  
            // Create a gradient for the bars
            const gradient = canvasCtx.createLinearGradient(0, 0, 0, canvas.height);
            gradient.addColorStop(0, 'red');
            gradient.addColorStop(0.5, 'yellow');
            gradient.addColorStop(1, 'green');
  
            canvasCtx.fillStyle = gradient;
            canvasCtx.fillRect(x, canvas.height - barHeight, barWidth, barHeight);
  
            x += barWidth + 1;
          }
        }
  
        draw(); // Start the visualization
      })
      .catch(err => {
        console.error('Error accessing the microphone', err);
        alert('Could not access the microphone. Please check permissions.');
      });
  });
  