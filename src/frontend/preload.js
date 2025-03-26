const { contextBridge, ipcRenderer } = require('electron');
const fs = require('fs');
const path = require('path');

contextBridge.exposeInMainWorld('electron', {
    send: (channel, data) => {
        const validChannels = ['navigate-to-app', 'navigate-to-upload', 'navigate-to-record', 'navigate-to-home'];
        if (validChannels.includes(channel)) {
            ipcRenderer.send(channel, data);
        }
    },
    receive: (channel, func) => {
        const validChannels = ['fromMain'];
        if (validChannels.includes(channel)) {
            ipcRenderer.on(channel, (event, ...args) => func(...args));
        }
    }
});

contextBridge.exposeInMainWorld('api', {
    processAudio: (command) => ipcRenderer.invoke('process-audio', command)
  });
  
contextBridge.exposeInMainWorld('nodeAPI', {
    saveTempWavFile: async (buffer) => {
      const tempFilePath = path.join(__dirname, '..', '..', 'temp.wav');
      await fs.promises.writeFile(tempFilePath, Buffer.from(buffer));
      return tempFilePath;
    },
    deleteTempWavFile: async () => {
      const tempFilePath = path.join(__dirname, '..', '..', 'temp.wav');
      await fs.promises.unlink(tempFilePath);
    }
});