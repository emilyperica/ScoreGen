const { app, BrowserWindow, ipcMain, Menu } = require('electron');
const path = require('path');

let mainWindow;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200,
        height: 800,
        frame: false, // Removes the default frame
        transparent: false,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            nodeIntegration: false,
            
        },
        titleBarStyle: 'hidden',
        ...(process.platform !== 'darwin' ? { titleBarOverlay: { color: '#2f3241', symbolColor: '#ffffff', height: 60 } } : {})
    });

    mainWindow.loadFile('index.html'); // Load the home screen initially

    // Open DevTools
    mainWindow.webContents.openDevTools();
    
    // Remove the default menu
    Menu.setApplicationMenu(null);

    // Handle navigation from home to Upload
    ipcMain.on('navigate-to-upload', () => {
        mainWindow.loadFile('upload.html'); // Ensure upload.html exists
    });

    // Handle navigation from home to Record
    ipcMain.on('navigate-to-record', () => {
        mainWindow.loadFile('record.html'); // Ensure record.html exists
    });

    // Handle navigation to Home
    ipcMain.on('navigate-to-home', () => {
        mainWindow.loadFile('index.html');
    });

    // Optional: Emit events when window is maximized/unmaximized
    mainWindow.on('maximize', () => {
        mainWindow.webContents.send('window-maximized');
    });

    mainWindow.on('unmaximize', () => {
        mainWindow.webContents.send('window-unmaximized');
    });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
    // On macOS, it's common for applications to stay open until the user explicitly quits
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', () => {
    // On macOS, recreate a window when the dock icon is clicked and there are no other windows open
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
    }
});