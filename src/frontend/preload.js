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

contextBridge.exposeInMainWorld('electronAPI', {
    // Navigation APIs
    navigateToHome: () => ipcRenderer.send('navigate-to-home'),
    navigateToSheetMusic: () => ipcRenderer.send('navigate-to-sheet-music'),
    
    // PDF Management APIs
    getPDFList: () => ipcRenderer.invoke('get-pdf-list'),
    downloadPDF: (pdfPath) => ipcRenderer.invoke('download-pdf', pdfPath),
    deletePDF: (pdfPath) => ipcRenderer.invoke('delete-pdf', pdfPath),

    
    // Existing APIs
    processAudio: (payload) => ipcRenderer.invoke('process-audio', payload),
    generatePDF: (payload) => ipcRenderer.invoke('generate-pdf', payload),
    
    // Window control APIs
    minimizeWindow: () => ipcRenderer.send('minimize-window'),
    maximizeWindow: () => ipcRenderer.send('maximize-window'),
    closeWindow: () => ipcRenderer.send('close-window')
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