// commands.js
const { spawn } = require('child_process');
const path = require('path');

// Adjust the path to match where your built executable is located.
const executablePath = path.join(__dirname, '..', '..', 'build', 'Debug', 'ScoreGen.exe');
console.log(`Spawning executable at: ${executablePath}`);

// Spawn the C++ executable.
const child = spawn(executablePath, [], { stdio: ['pipe', 'pipe', 'pipe'] });

// Listen for data from the C++ process's stdout.
child.stdout.on('data', (data) => {
  console.log(`stdout: ${data.toString()}`);
});

// Listen for error output from stderr.
child.stderr.on('data', (data) => {
  console.error(`stderr: ${data.toString()}`);
});

// Handle any errors that occur during spawning.
child.on('error', (err) => {
  console.error('Failed to start subprocess:', err);
});

// Log when the child process exits.
child.on('close', (code) => {
  console.log(`Child process exited with code ${code}`);
});

// Function to send a command to the C++ backend.
function sendCommand(command) {
  console.log(`Sending command: ${command}`);
  child.stdin.write(command + '\n');
}

// Example: Send the "processAudio" command to trigger the processing.
sendCommand('processAudio');
