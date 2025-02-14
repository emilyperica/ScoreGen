const { contextBridge, ipcRenderer } = require('electron');

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
    processAudio: () => ipcRenderer.send('process-audio')
});