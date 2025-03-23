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
            sandbox:false,
            
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
    // 1) Spawn the C++ executable
    const executablePath = path.join('build/Debug/ScoreGen.exe');
    console.log(`Spawning C++ backend`);
    childProc = spawn(executablePath, [], { stdio: ['pipe', 'pipe', 'pipe'] });
  
    // 2) Debug logs from the C++ process
    childProc.stdout.on('data', (data) => {
      console.log(`Backend stdout: ${data.toString()}`);
    });
    childProc.stderr.on('data', (data) => {
      console.error(`Backend stderr: ${data.toString()}`);
    });
    childProc.on('error', (err) => {
      console.error('Failed to start subprocess:', err);
    });
    childProc.on('close', (code) => {
      console.log(`Child process exited with code ${code}`);
    });
  
    // 3) Handle "process-audio" via a Promise (so the renderer can await it)
    ipcMain.handle('process-audio', async () => {
        console.log('[Main] ipcMain.handle("process-audio") triggered');
      
        return new Promise((resolve, reject) => {
          if (!childProc || !childProc.stdin.writable) {
            return reject(new Error('C++ process is not available.'));
          }
          let resolved = false;
      
          const timeout = setTimeout(() => {
            if (!resolved) {
              resolved = true;
              childProc.stdout.off('data', onData);
              reject(new Error('Timed out waiting for backend response.'));
            }
          }, 100000);
      
          const onData = (data) => {
            const text = data.toString();
            console.log(`[Main] Child stdout: ${text}`);
      
            if (text.includes('MusicXML file generated successfully.')) {
              clearTimeout(timeout);
              childProc.stdout.off('data', onData);
              resolved = true;
              resolve();
            } else if (text.includes('Failed to generate MusicXML file.')) {
              clearTimeout(timeout);
              childProc.stdout.off('data', onData);
              resolved = true;
              reject(new Error('C++ process reported failure.'));
            }
          };
      
          // Attach the listener before sending the command
          childProc.stdout.on('data', onData);
          childProc.stdin.write('processAudio\n');
        });
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