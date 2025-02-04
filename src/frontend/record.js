document.addEventListener('DOMContentLoaded', function() {
  /* === Recording Variables & Elements === */
  let mediaRecorder;
  let audioChunks = [];
  let recordedAudioBlob = null;
  let recordingStartTime = null;
  let recordingInterval = null;

  const inputDeviceSelect = document.getElementById('input-device-select');
  const recordingLengthDisplay = document.getElementById('recording-length');
  const startRecordingBtn = document.getElementById('start-recording');
  const pauseRecordingBtn = document.getElementById('pause-recording');
  const stopRecordingBtn = document.getElementById('stop-recording');
  const exportMusicxmlBtn = document.getElementById('export-musicxml');
  const saveRecordingBtn = document.getElementById('save-recording');
  const processButton = document.getElementById('processButton')
  
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
  
  /* --- Helper Functions for WAV conversion --- */
  
  // Converts a Blob (from MediaRecorder) to a WAV Blob.
  function convertBlobToWav(blob) {
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = function() {
        const arrayBuffer = reader.result;
        // Create an AudioContext to decode the audio data.
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        audioContext.decodeAudioData(arrayBuffer, function(audioBuffer) {
          // Convert the AudioBuffer into a WAV ArrayBuffer.
          const wavBuffer = audioBufferToWav(audioBuffer);
          const wavBlob = new Blob([wavBuffer], { type: 'audio/wav' });
          resolve(wavBlob);
        }, reject);
      };
      reader.onerror = reject;
      reader.readAsArrayBuffer(blob);
    });
  }
  
  // Converts an AudioBuffer to a WAV ArrayBuffer.
  function audioBufferToWav(buffer, opt) {
    opt = opt || {};
  
    var numChannels = buffer.numberOfChannels;
    var sampleRate = buffer.sampleRate;
    var format = opt.float32 ? 3 : 1; // 1 = PCM, 3 = IEEE Float
    var bitDepth = format === 3 ? 32 : 16;
  
    var result;
    if (numChannels === 2) {
      result = interleave(buffer.getChannelData(0), buffer.getChannelData(1));
    } else {
      result = buffer.getChannelData(0);
    }
    return encodeWAV(result, numChannels, sampleRate, bitDepth);
  }
  
  function interleave(inputL, inputR) {
    var length = inputL.length + inputR.length;
    var result = new Float32Array(length);
    var index = 0;
    var inputIndex = 0;
  
    while (index < length) {
      result[index++] = inputL[inputIndex];
      result[index++] = inputR[inputIndex];
      inputIndex++;
    }
    return result;
  }
  
  function encodeWAV(samples, numChannels, sampleRate, bitDepth) {
    var bytesPerSample = bitDepth / 8;
    var blockAlign = numChannels * bytesPerSample;
    var buffer = new ArrayBuffer(44 + samples.length * bytesPerSample);
    var view = new DataView(buffer);
  
    /* RIFF identifier */
    writeString(view, 0, 'RIFF');
    /* file length */
    view.setUint32(4, 36 + samples.length * bytesPerSample, true);
    /* RIFF type */
    writeString(view, 8, 'WAVE');
    /* format chunk identifier */
    writeString(view, 12, 'fmt ');
    /* format chunk length */
    view.setUint32(16, 16, true);
    /* sample format (PCM) */
    view.setUint16(20, 1, true);
    /* channel count */
    view.setUint16(22, numChannels, true);
    /* sample rate */
    view.setUint32(24, sampleRate, true);
    /* byte rate (sample rate * block align) */
    view.setUint32(28, sampleRate * blockAlign, true);
    /* block align (channel count * bytes per sample) */
    view.setUint16(32, blockAlign, true);
    /* bits per sample */
    view.setUint16(34, bitDepth, true);
    /* data chunk identifier */
    writeString(view, 36, 'data');
    /* data chunk length */
    view.setUint32(40, samples.length * bytesPerSample, true);
  
    if (bitDepth === 16) {
      floatTo16BitPCM(view, 44, samples);
    } else {
      writeFloat32(view, 44, samples);
    }
    return buffer;
  }
  
  function writeString(view, offset, string) {
    for (var i = 0; i < string.length; i++){
      view.setUint8(offset + i, string.charCodeAt(i));
    }
  }
  
  function floatTo16BitPCM(output, offset, input) {
    for (var i = 0; i < input.length; i++, offset += 2) {
      var s = Math.max(-1, Math.min(1, input[i]));
      output.setInt16(offset, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
    }
  }
  
  function writeFloat32(output, offset, input) {
    for (var i = 0; i < input.length; i++, offset += 4) {
      output.setFloat32(offset, input[i], true);
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
    // Set the uploaded file as the source for the audio element.
    audio.src = URL.createObjectURL(file);
    audio.load();
    audio.addEventListener('loadedmetadata', function() {
      totalDurationDisplay.textContent = formatTime(audio.duration);
    }, { once: true });
    playPauseButton.disabled = false;
    // Enable the save and export buttons.
    saveRecordingBtn.disabled = false;
    exportMusicxmlBtn.disabled = false;
    // Store the file as the recordedAudioBlob so that it can be used by other features.
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
      
      // Update button states.
      startRecordingBtn.disabled = true;
      pauseRecordingBtn.disabled = false;
      stopRecordingBtn.disabled = false;
      exportMusicxmlBtn.disabled = true;  // Disable export until recording is complete
      saveRecordingBtn.disabled = true;     // Disable save until recording is complete
      
      // Start recording timer.
      recordingStartTime = Date.now();
      recordingInterval = setInterval(updateRecordingTimer, 1000);
      
      // Collect audio data chunks.
      mediaRecorder.addEventListener('dataavailable', event => {
        console.log("Data available:", event.data.size);
        audioChunks.push(event.data);
      });
      
      // When recording stops, create the Blob and load it into the audio player.
      mediaRecorder.addEventListener('stop', () => {
        clearInterval(recordingInterval);
        updateRecordingTimer(); // Final update.
        recordedAudioBlob = new Blob(audioChunks, { type: 'audio/ogg; codecs=opus' });
        console.log("Recorded Blob size:", recordedAudioBlob.size);
        
        // Set the recorded audio as the source for the playback element.
        audio.src = URL.createObjectURL(recordedAudioBlob);
        audio.load();
        
        // When metadata is loaded, update the total duration display.
        audio.addEventListener('loadedmetadata', function() {
          let duration = audio.duration;
          if (!isFinite(duration)) {
            duration = (Date.now() - recordingStartTime) / 1000;
          }
          totalDurationDisplay.textContent = formatTime(duration);
        }, { once: true });
        
        playPauseButton.disabled = false;
        // Enable export and save buttons now that a recording exists.
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
      // Note: exportMusicxmlBtn and saveRecordingBtn will be enabled in the 'stop' handler.
    }
  });
  
  /* --- Save Recording as WAV --- */
  saveRecordingBtn.addEventListener('click', () => {
    if (!recordedAudioBlob) {
      alert("No recording available to save!");
      return;
    }
    // Convert the recorded audio Blob (or uploaded file) to a WAV Blob and trigger download.
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

  processButton.addEventListener('click', async () => {
    // Replace with an actual file path or use a file dialog.
    const filePath = "C:\\path\\to\\your\\audio.wav";
    try {
      const result = await window.api.processAudio(filePath);
      console.log("Processing result:", result);
    } catch (err) {
      console.error("Error:", err);
    }
  });
  
  /* --- Export to MusicXML --- */
  exportMusicxmlBtn.addEventListener('click', () => {
    if (!recordedAudioBlob) {
      alert("No recording available to export!");
      return;
    }
    transcribeAudioToMusicXML(recordedAudioBlob)
      .then(musicXML => {
        const xmlBlob = new Blob([musicXML], { type: 'application/xml' });
        const url = URL.createObjectURL(xmlBlob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'score.musicxml';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
      })
      .catch(err => {
        console.error("Error during MusicXML export:", err);
        alert("Failed to export audio to MusicXML.");
      });
  });
  
  // Dummy transcription function (simulate converting audio to MusicXML).
  function transcribeAudioToMusicXML(audioBlob) {
    return new Promise((resolve, reject) => {
      setTimeout(() => {
        const dummyMusicXML = `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE score-partwise PUBLIC
    "-//Recordare//DTD MusicXML 3.1 Partwise//EN"
    "http://www.musicxml.org/dtds/partwise.dtd">
<score-partwise version="3.1">
  <part-list>
    <score-part id="P1">
      <part-name>Music</part-name>
    </score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>1</divisions>
        <key>
          <fifths>0</fifths>
        </key>
        <time>
          <beats>4</beats>
          <beat-type>4</beat-type>
        </time>
        <clef>
          <sign>G</sign>
          <line>2</line>
        </clef>
      </attributes>
      <note>
        <pitch>
          <step>C</step>
          <octave>4</octave>
        </pitch>
        <duration>1</duration>
        <type>quarter</type>
      </note>
      <note>
        <rest/>
        <duration>1</duration>
        <type>quarter</type>
      </note>
      <note>
        <pitch>
          <step>E</step>
          <octave>4</octave>
        </pitch>
        <duration>1</duration>
        <type>quarter</type>
      </note>
      <note>
        <pitch>
          <step>G</step>
          <octave>4</octave>
        </pitch>
        <duration>1</duration>
        <type>quarter</type>
      </note>
    </measure>
  </part>
</score-partwise>`;
        resolve(dummyMusicXML);
      }, 1000); // Simulated delay.
    });
  }
  
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
});
