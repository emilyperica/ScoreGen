<img src="src/frontend/assets/SG.png" 
        alt="ScoreGen logo" 
        width="200" 
        height="200" 
        style="display: block; margin: 0 auto" />

> An audio-to-sheet music generator. 

## About
ScoreGen is an innovative desktop application that converts live monophonic audio recordings into professional sheet music. Built as a capstone project, it features:

- Real-time monophonic audio processing
- Natively developed signal processing engine for accurate note detection
- Intuitive user interface for recording and editing
- Multiple export formats (WAV, MusicXML, PDF)
- Built-in sheet music viewer and library management

The application handles the complete pipeline from audio capture through signal processing to final sheet music rendering, making music transcription accessible to musicians of all levels.

**Developer Names**: Emily Perica, Ian Algenio, Jackson Lippert, Mark Kogan  
**Date of project start**: Sept 16/2024 



## Folder structure
The subfolders and files of this project are structured as follows:

    ├── .github      // DevOps and project management.
    ├── docs         // Project documentation.
    ├── include      // C++ project headers.
    ├── libs         // External project dependencies.
    ├── refs         // Reference material used for the project, including papers. 
    ├── scripts      // Run and build scripts.
    ├── src          // Source code.
        ├── backend  // C++ backend logic source code.
        ├── frontend // JavaScript frontend.
    ├── test         // Test cases


## Development
Windows 11 is the suggested development environment; a non-Windows environment may be used but may require slightly different setup than is documented here.

### Preqrequisites
ScoreGen uses [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview), [CMake](https://cmake.org/download/), and npm for managing dependencies. Ensure these are installed before getting started.

To install dependencies:

```
# backend dependencies
.\scripts\installDependencies.sh

# frontend dependencies
npm install
```

### Build and Run
```
# compile and build backend dependencies using CMake
.\scripts\cleanBuild.sh 
# start up the desktop app
npm run start

# alternatively: package the desktop app and generate a distributable
npm run make 
```

## Contribute
See the contribution guide.

## License
This project is distributed under the MIT license.
