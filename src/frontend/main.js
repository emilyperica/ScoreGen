const { app, BrowserWindow, ipcMain, Menu } = require('electron');
const path = require('path');
const { spawn } = require('child_process');

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

    mainWindow.loadFile('src/frontend/index.html'); // Load the home screen initially

    // Open DevTools
    mainWindow.webContents.openDevTools();
    
    // Remove the default menu
    Menu.setApplicationMenu(null);

    // Handle navigation from home to Upload
    ipcMain.on('navigate-to-upload', () => {
        mainWindow.loadFile('src/frontend/upload.html'); // Ensure upload.html exists
    });

    // Handle navigation from home to Record
    ipcMain.on('navigate-to-record', () => {
        mainWindow.loadFile('src/frontend/record.html'); // Ensure record.html exists
    });

    // Handle navigation to Home
    ipcMain.on('navigate-to-home', () => {
        mainWindow.loadFile('src/frontend/index.html');
    });

    // Optional: Emit events when window is maximized/unmaximized
    mainWindow.on('maximize', () => {
        mainWindow.webContents.send('window-maximized');
    });

    mainWindow.on('unmaximize', () => {
        mainWindow.webContents.send('window-unmaximized');
    });
}

app.whenReady().then(() => {
    // Adjust the path to your built C++ executable as needed.
    const executablePath = path.join('build/Debug/ScoreGen.exe');
    console.log(`Spawning C++ backend at: ${executablePath}`);
  
    // Spawn the C++ process with a pipe for stdin.
    childProc = spawn(executablePath, [], { stdio: ['pipe', 'pipe', 'pipe'] });
  
    // Listen to stdout and stderr from the C++ process.
    childProc.stdout.on('data', (data) => {
      console.log(`Backend stdout: ${data.toString()}`);
    });
    childProc.stderr.on('data', (data) => {
      console.error(`Backend stderr: ${data.toString()}`);
    });
  
    // Listen for the "process-audio" IPC message from the renderer.
    ipcMain.on('process-audio', (event) => {
      console.log('Received "process-audio" event');
      if (childProc && childProc.stdin.writable) {
        // Write the command followed by a newline so that the C++ process reads it.
        childProc.stdin.write('processAudio\n');
      }
    });
  
    createWindow();
  });

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