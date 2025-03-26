document.addEventListener('DOMContentLoaded', function() {
  /* === Recording Variables & Elements === */
  let mediaRecorder;
  let audioChunks = [];
  let recordedAudioBlob = null;
  let recordingStartTime = null;
  let recordingInterval = null;

  /* === Record Audio Elements === */
  const inputDeviceSelect = document.getElementById('input-device-select');
  const recordingLengthDisplay = document.getElementById('recording-length');
  const startRecordingBtn = document.getElementById('start-recording');
  const pauseRecordingBtn = document.getElementById('pause-recording');
  const stopRecordingBtn = document.getElementById('stop-recording');
  const saveRecordingBtn = document.getElementById('save-recording');

  /* === Export to MusicXML Elements === */
  const exportMusicxmlBtn = document.getElementById('export-musicxml');
  const modal = document.getElementById('musicxmlModal');
  const modalClose = document.getElementById('modalClose');
  const modalCancel = document.getElementById('modalCancel');
  const musicxmlForm = document.getElementById('musicxmlForm');
  
  /* === Playback Variables & Elements === */
  const audio = document.getElementById('audio');
  const playPauseButton = document.getElementById('play-pause-button');
  const prevButton = document.getElementById('prev-button');
  const nextButton = document.getElementById('next-button');
  const progressBar = document.getElementById('progress-bar');
  const currentTimeDisplay = document.getElementById('current-time');
  const totalDurationDisplay = document.getElementById('total-duration');
  
  /* === Upload Elements === */
  const uploadFileButton = document.getElementById('upload-file-button');
  const uploadFileInput = document.getElementById('upload-file');
  

  // Converts a Blob (from getUserMedia, for example) into a 16-bit PCM WAV Blob,
  // rendered at the target sample rate (e.g. 44100 Hz) rather than the default 48000 Hz.
  async function convertBlobToWav(blob) {
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = function() {
        const arrayBuffer = reader.result;
        console.log(`[convertBlobToWav] Read ArrayBuffer of byteLength: ${arrayBuffer.byteLength}`);
        
        // Use a temporary AudioContext to decode.
        const tempAudioCtx = new (window.AudioContext || window.webkitAudioContext)();
        tempAudioCtx.decodeAudioData(arrayBuffer, function(audioBuffer) {
          console.log(`[convertBlobToWav] Decoded AudioBuffer: sampleRate=${audioBuffer.sampleRate}, length=${audioBuffer.length}, channels=${audioBuffer.numberOfChannels}`);
          
          // Instead of forcing 44100 Hz, use the original sampleRate.
          const originalSampleRate = audioBuffer.sampleRate;
          const channels = audioBuffer.numberOfChannels;
          const duration = audioBuffer.length / originalSampleRate;
          const newLength = audioBuffer.length; // Keep the same length in samples.
          console.log(`[convertBlobToWav] Creating OfflineAudioContext: targetSampleRate=${originalSampleRate}, duration=${duration.toFixed(2)}s, newLength=${newLength}`);
          
          const offlineCtx = new OfflineAudioContext(channels, newLength, originalSampleRate);
          const source = offlineCtx.createBufferSource();
          source.buffer = audioBuffer;
          source.connect(offlineCtx.destination);
          source.start();
          
          offlineCtx.startRendering().then((renderedBuffer) => {
            console.log(`[convertBlobToWav] Offline rendering complete: sampleRate=${renderedBuffer.sampleRate}, length=${renderedBuffer.length}`);
            const wavBuffer = audioBufferToWav(renderedBuffer); // still outputs 16-bit PCM
            const wavBlob = new Blob([wavBuffer], { type: 'audio/wav' });
            resolve(wavBlob);
          }).catch(reject);
        }, reject);
      };
      reader.onerror = reject;
      reader.readAsArrayBuffer(blob);
    });
  }
  
  function audioBufferToWav(buffer) {
    const numChannels = buffer.numberOfChannels;
    const sampleRate = buffer.sampleRate; // This will be 44100 if rendered with OfflineAudioContext
    const bitDepth = 16; // Always 16-bit PCM
  
    let result;
    if (numChannels === 2) {
      result = interleave(buffer.getChannelData(0), buffer.getChannelData(1));
    } else {
      result = buffer.getChannelData(0);
    }
    return encodeWAV(result, numChannels, sampleRate, bitDepth);
  }
  
  function interleave(inputL, inputR) {
    const length = inputL.length + inputR.length;
    const result = new Float32Array(length);
    let index = 0;
    let inputIndex = 0;
  
    while (index < length) {
      result[index++] = inputL[inputIndex];
      result[index++] = inputR[inputIndex];
      inputIndex++;
    }
    return result;
  }
  
  function encodeWAV(samples, numChannels, sampleRate, bitDepth) {
    const bytesPerSample = bitDepth / 8;
    const blockAlign = numChannels * bytesPerSample;
    const buffer = new ArrayBuffer(44 + samples.length * bytesPerSample);
    const view = new DataView(buffer);
  
    // RIFF identifier
    writeString(view, 0, 'RIFF');
    // file length
    view.setUint32(4, 36 + samples.length * bytesPerSample, true);
    // RIFF type
    writeString(view, 8, 'WAVE');
    // format chunk identifier
    writeString(view, 12, 'fmt ');
    // format chunk length
    view.setUint32(16, 16, true);
    // sample format (PCM)
    view.setUint16(20, 1, true);
    // channel count
    view.setUint16(22, numChannels, true);
    // sample rate
    view.setUint32(24, sampleRate, true);
    // byte rate (sample rate * block align)
    view.setUint32(28, sampleRate * blockAlign, true);
    // block align (channel count * bytes per sample)
    view.setUint16(32, blockAlign, true);
    // bits per sample
    view.setUint16(34, bitDepth, true);
    // data chunk identifier
    writeString(view, 36, 'data');
    // data chunk length
    view.setUint32(40, samples.length * bytesPerSample, true);
  
    // Convert samples to 16-bit PCM
    floatTo16BitPCM(view, 44, samples);
    return buffer;
  }
  
  function writeString(view, offset, string) {
    for (let i = 0; i < string.length; i++){
      view.setUint8(offset + i, string.charCodeAt(i));
    }
  }
  
  function floatTo16BitPCM(output, offset, input) {
    for (let i = 0; i < input.length; i++, offset += 2) {
      let s = Math.max(-1, Math.min(1, input[i]));
      output.setInt16(offset, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
    }
  }
  
  /* --- Helper Functions for Time Formatting --- */
  function updateRecordingTimer() {
    const elapsedMs = Date.now() - recordingStartTime;
    const seconds = Math.floor((elapsedMs / 1000) % 60);
    const minutes = Math.floor(elapsedMs / 1000 / 60);
    recordingLengthDisplay.textContent = `${minutes}:${seconds < 10 ? '0' + seconds : seconds}`;
  }
  
  function formatTime(seconds) {
    if (!isFinite(seconds)) {
      return "0:00";
    }
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins}:${secs < 10 ? '0' + secs : secs}`;
  }
  
  /* --- Populate Audio Input Devices --- */
  navigator.mediaDevices.enumerateDevices()
    .then(devices => {
      inputDeviceSelect.innerHTML = '';
      devices.forEach(device => {
        if (device.kind === 'audioinput') {
          const option = document.createElement('option');
          option.value = device.deviceId;
          option.textContent = device.label || `Microphone ${inputDeviceSelect.length + 1}`;
          inputDeviceSelect.appendChild(option);
        }
      });
    })
    .catch(err => {
      console.error("Error enumerating devices:", err);
    });
  
  /* --- Upload File Handling --- */
  uploadFileButton.addEventListener('click', () => {
    uploadFileInput.click();
  });
  
  uploadFileInput.addEventListener('change', function() {
    const file = this.files[0];
    if (!file) return;
    if (!file.name.toLowerCase().endsWith('.wav')) {
      alert('Please upload a valid .wav file.');
      return;
    }
    audio.src = URL.createObjectURL(file);
    audio.load();
    audio.addEventListener('loadedmetadata', function() {
      totalDurationDisplay.textContent = formatTime(audio.duration);
      console.log(`Audio metadata: duration=${audio.duration}, src=${audio.src}`);
    }, { once: true });
    playPauseButton.disabled = false;
    saveRecordingBtn.disabled = false;
    exportMusicxmlBtn.disabled = false;
    recordedAudioBlob = file;
  });
  
  /* --- Recording Event Listeners --- */
  startRecordingBtn.addEventListener('click', () => {
    const selectedDeviceId = inputDeviceSelect.value;
    navigator.mediaDevices.getUserMedia({
      audio: { deviceId: selectedDeviceId ? { exact: selectedDeviceId } : undefined }
    })
    .then(stream => {
      mediaRecorder = new MediaRecorder(stream);
      audioChunks = [];
      mediaRecorder.start();
      
      startRecordingBtn.disabled = true;
      pauseRecordingBtn.disabled = false;
      stopRecordingBtn.disabled = false;
      exportMusicxmlBtn.disabled = true;
      saveRecordingBtn.disabled = true;
      
      recordingStartTime = Date.now();
      recordingInterval = setInterval(updateRecordingTimer, 1000);
      
      mediaRecorder.addEventListener('dataavailable', event => {
        console.log("Data available:", event.data.size);
        audioChunks.push(event.data);
      });
      
      mediaRecorder.addEventListener('stop', () => {
        clearInterval(recordingInterval);
        updateRecordingTimer();
        recordedAudioBlob = new Blob(audioChunks, { type: 'audio/ogg; codecs=opus' });
        console.log("Recorded Blob size:", recordedAudioBlob.size);
        
        audio.src = URL.createObjectURL(recordedAudioBlob);
        audio.load();
        
        audio.addEventListener('loadedmetadata', function() {
          let duration = audio.duration;
          if (!isFinite(duration)) {
            duration = (Date.now() - recordingStartTime) / 1000;
          }
          totalDurationDisplay.textContent = formatTime(duration);
        }, { once: true });
        
        playPauseButton.disabled = false;
        exportMusicxmlBtn.disabled = false;
        saveRecordingBtn.disabled = false;
      });
      
    })
    .catch(error => {
      console.error("Error accessing microphone:", error);
    });
  });
  
  pauseRecordingBtn.addEventListener('click', () => {
    if (mediaRecorder && mediaRecorder.state === 'recording') {
      mediaRecorder.pause();
      pauseRecordingBtn.disabled = true;
      startRecordingBtn.disabled = false;
      clearInterval(recordingInterval);
    }
  });
  
  stopRecordingBtn.addEventListener('click', () => {
    if (mediaRecorder && (mediaRecorder.state === 'recording' || mediaRecorder.state === 'paused')) {
      mediaRecorder.stop();
      startRecordingBtn.disabled = false;
      pauseRecordingBtn.disabled = true;
      stopRecordingBtn.disabled = true;
    }
  });
  
  /* --- Save Recording as WAV --- */
  saveRecordingBtn.addEventListener('click', () => {
    if (!recordedAudioBlob) {
      alert("No recording available to save!");
      return;
    }
    convertBlobToWav(recordedAudioBlob)
      .then(wavBlob => {
        const url = URL.createObjectURL(wavBlob);
        const a = document.createElement('a');
        a.style.display = 'none';
        a.href = url;
        a.download = 'recording.wav';
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
        document.body.removeChild(a);
      })
      .catch(err => {
        console.error("Error converting to WAV:", err);
        alert("Failed to convert recording to WAV.");
      });
  });

    exportMusicxmlBtn.addEventListener('click', () => {
      console.log('[Renderer] Export button clicked.');
      modal.style.display = 'block';
    });
  
  /* --- Playback Event Listeners --- */
  playPauseButton.addEventListener('click', () => {
    if (audio.paused) {
      audio.play();
      playPauseButton.innerHTML = '<i class="fas fa-pause"></i>';
      playPauseButton.title = "Pause";
    } else {
      audio.pause();
      playPauseButton.innerHTML = '<i class="fas fa-play"></i>';
      playPauseButton.title = "Play";
    }
  });
  
  audio.addEventListener('timeupdate', () => {
    if (!isNaN(audio.duration) && isFinite(audio.duration)) {
      progressBar.value = (audio.currentTime / audio.duration) * 100;
      currentTimeDisplay.textContent = formatTime(audio.currentTime);
    }
  });
  
  audio.addEventListener('loadedmetadata', () => {
    totalDurationDisplay.textContent = formatTime(audio.duration);
  });
  
  audio.addEventListener('ended', () => {
    playPauseButton.innerHTML = '<i class="fas fa-play"></i>';
    playPauseButton.title = "Play";
  });
  
  progressBar.addEventListener('input', () => {
    if (audio.duration && isFinite(audio.duration)) {
      let seekTime = (progressBar.value / 100) * audio.duration;
      audio.currentTime = seekTime;
    }
  });
  
  prevButton.addEventListener('click', () => {
    audio.currentTime = 0;
  });
  
  nextButton.addEventListener('click', () => {
    audio.currentTime = 0;
    audio.play();
    playPauseButton.innerHTML = '<i class="fas fa-pause"></i>';
    playPauseButton.title = "Pause";
  });

  /* --- MusicXML Form Submission --- */
  modalClose.addEventListener('click', () => {
    console.log('[Renderer] Modal close button clicked.');
    modal.style.display = 'none';
  });

  modalCancel.addEventListener('click', () => {
    console.log('[Renderer] Modal cancel button clicked.');
    modal.style.display = 'none';
  });
  
  musicxmlForm.addEventListener('submit', function (e) {
    console.log('[Renderer] Modal submit button clicked.');
    e.preventDefault();
  
    const formData = {
      workNumber: document.getElementById('workNumber').value,
      workTitle: document.getElementById('workTitle').value,
      movementNumber: document.getElementById('movementNumber').value,
      movementTitle: document.getElementById('movementTitle').value,
      creatorName: document.getElementById('creatorName').value,
      instrument: document.getElementById('instrumentInput').value,
      timeSignature: document.getElementById('timeSignatureInput').value
    };
  
    const command = `processAudio|${formData.workTitle}|${formData.workNumber}|${formData.movementNumber}|${formData.movementTitle}|${formData.creatorName}|${formData.instrument}|${formData.timeSignature}`;
  
    if (!recordedAudioBlob) {
      alert("No recording available to export!");
      return;
    }
  
    const spinner = document.getElementById('spinnerOverlay');
    if (spinner) spinner.style.display = 'flex';
  
    (async function () {
      try {
        const wavBlob = await convertBlobToWav(recordedAudioBlob);
        const buffer = await wavBlob.arrayBuffer();
  
        await window.nodeAPI.saveTempWavFile(buffer);
        await window.api.processAudio(command);
  
        const response = await fetch('../../output.xml');
        if (!response.ok) {
          throw new Error('Failed to fetch output.xml');
        }
  
        const musicxmlBlob = await response.blob();
        const url = URL.createObjectURL(musicxmlBlob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'score.musicxml';
        document.body.appendChild(a);
        a.click();
        a.remove();
        URL.revokeObjectURL(url);
      } catch (err) {
        console.error("Error during export:", err);
        alert("Failed to export MusicXML.");
      } finally {
        if (spinner) spinner.style.display = 'none';
        modal.style.display = 'none';
      }
    })();
  });

});
