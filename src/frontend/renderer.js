document.addEventListener("DOMContentLoaded", () => {
    console.log("DOM fully loaded and parsed");

    // Initialize Verovio Toolkit
    verovio.module.onRuntimeInitialized = function () {
        console.log("Verovio toolkit initialized");

        const tk = new verovio.toolkit();

        let zoom = 30; // Initialize zoom with saved value
        let currentPage = 1;
        let totalPages = 1; // Initialize total pages
        let isPlaying = false; // State flag to track playback

        const MIN_ZOOM = 10; // Minimum zoom level
        const MAX_ZOOM = 100; // Maximum zoom level
        const ZOOM_STEP = 5; // Zoom increment/decrement step

        const options = {
            scale: zoom
        };
        tk.setOptions(options);

        // Handle file uploads
        const fileInput = document.getElementById("musicxmlFile");
        const toggleBtn = document.getElementById('toggle-btn');

        const playMIDIHandler = function () {
            let base64midi = tk.renderToMIDI();
            let midiString = 'data:audio/midi;base64,' + base64midi;
            MIDIjs.play(midiString);
            console.log("MIDI playback started.");
        };

        // Handler to stop MIDI playback
        const stopMIDIHandler = function () {
            MIDIjs.stop();
            console.log("MIDI playback stopped.");
        };

        const midiHighlightingHandler = function (event) {
            let playingNotes = document.querySelectorAll('g.note.playing');
            for (let playingNote of playingNotes) playingNote.classList.remove("playing");

            let currentElements = tk.getElementsAtTime(event.time * 1000);

            if (currentElements.page == 0) return;

            if (currentElements.page != currentPage) {
                currentPage = currentElements.page;
                renderCurrentPage();
                
            }

            for (let note of currentElements.notes) {
                let noteElement = document.getElementById(note);
                if (noteElement) noteElement.classList.add("playing");
            }
        };

        MIDIjs.player_callback = midiHighlightingHandler;

        // Function to toggle playback state
        function togglePlayback(toggleBtn) {
            const icon = toggleBtn.querySelector('i');
            
            if (!icon) {
                console.error('Icon element not found within toggleBtn.');
                return;
            }

            if (!isPlaying) {
                // Start playback
                isPlaying = true;
                playMIDIHandler(); // Assuming this starts playback
                toggleBtn.classList.remove('btn-enter');
                toggleBtn.classList.add('btn-stop');
                icon.classList.remove('fa-play');
                icon.classList.add('fa-stop');
                console.log('Playback started.');
            } else {
                // Stop playback
                isPlaying = false;
                stopMIDIHandler();
                toggleBtn.classList.remove('btn-stop');
                toggleBtn.classList.add('btn-enter');
                icon.classList.remove('fa-stop');
                icon.classList.add('fa-play');
                console.log('Playback stopped.');
            }
        }

        if (toggleBtn) {
            toggleBtn.addEventListener('click', () => {
                togglePlayback(toggleBtn);
            });
        } else {
            console.error('Toggle button not found.');
        }

        // Function to render the current page
        const renderCurrentPage = () => {
            const svg = tk.renderToSVG(currentPage);
            document.getElementById("notation").innerHTML = svg;
            updatePageIndicator();
            updateNavigationButtons();
        };

        // Function to handle file uploads
        fileInput.addEventListener("change", (e) => {
            const file = e.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = (event) => {
                const content = event.target.result;
                try {
                    const loadResult = tk.loadData(content); // Use the file content for MEI/MusicXML
                    if (loadResult > 0) { // loadData returns the number of elements loaded
                        currentPage = 1;
                        totalPages = tk.getPageCount(); // Get total number of pages
                        renderCurrentPage();
                        updateNavigationButtons();
                        console.log("MusicXML file loaded and rendered successfully.");
                    } else {
                        console.error("Failed to load MusicXML data.");
                        document.getElementById("notation").innerHTML = '<p>Failed to load MusicXML data.</p>';
                        currentPage = 0;
                        totalPages = 0;
                        updatePageIndicator();
                        updateNavigationButtons();
                    }
                } catch (error) {
                    console.error("Error loading MusicXML file:", error);
                    document.getElementById("notation").innerHTML = '<p>Error rendering notation.</p>';
                    currentPage = 0;
                    totalPages = 0;
                    updatePageIndicator();
                    updateNavigationButtons();
                }
            };
            reader.readAsText(file);
        });

        // Zoom In Handler
        document.getElementById("zoomIn").addEventListener("click", () => {
            if (zoom < MAX_ZOOM) {
                zoom += ZOOM_STEP;
                tk.setOptions({ scale: zoom });
                renderCurrentPage();
                updateButtonStates();
            }
        });

        // Zoom Out Handler
        document.getElementById("zoomOut").addEventListener("click", () => {
            if (zoom > MIN_ZOOM) {
                zoom -= ZOOM_STEP;
                tk.setOptions({ scale: zoom });
                renderCurrentPage();
                updateButtonStates();
            }
        });

        // Navigation Buttons
        const prevPageBtn = document.getElementById("prevPage");
        const nextPageBtn = document.getElementById("nextPage");
        const pageIndicator = document.getElementById("pageIndicator");

        if (prevPageBtn && nextPageBtn && pageIndicator) {
            // Previous Page Handler
            prevPageBtn.addEventListener("click", () => {
                if (currentPage > 1) {
                    currentPage--;
                    renderCurrentPage();
                }
            });

            // Next Page Handler
            nextPageBtn.addEventListener("click", () => {
                if (currentPage < totalPages) {
                    currentPage++;
                    renderCurrentPage();
                }
            });
        } else {
            console.error("Navigation buttons or page indicator not found");
        }
    
        // Function to update the state (enabled/disabled) of zoom buttons
        const updateButtonStates = () => {
            const zoomInButton = document.getElementById("zoomIn");
            const zoomOutButton = document.getElementById("zoomOut");

            // Disable Zoom In if at maximum zoom
            if (zoom >= MAX_ZOOM) {
                zoomInButton.disabled = true;
                zoomInButton.classList.add('btn-disabled');
            } else {
                zoomInButton.disabled = false;
                zoomInButton.classList.remove('btn-disabled');
            }

            // Disable Zoom Out if at minimum zoom
            if (zoom <= MIN_ZOOM) {
                zoomOutButton.disabled = true;
                zoomOutButton.classList.add('btn-disabled');
            } else {
                zoomOutButton.disabled = false;
                zoomOutButton.classList.remove('btn-disabled');
            }
        };

        // Function to update the page indicator
        const updatePageIndicator = () => {
            if (pageIndicator) {
                if (totalPages > 0) {
                    pageIndicator.textContent = `Page ${currentPage} of ${totalPages}`;
                } else {
                    pageIndicator.textContent = `Page 0 of 0`;
                }
            }
        };

        // Function to update navigation buttons' enabled/disabled state
        const updateNavigationButtons = () => {
            if (prevPageBtn && nextPageBtn) {
                // Disable Previous button if on first page
                if (currentPage <= 1) {
                    prevPageBtn.disabled = true;
                    prevPageBtn.classList.add('btn-disabled');
                } else {
                    prevPageBtn.disabled = false;
                    prevPageBtn.classList.remove('btn-disabled');
                }

                // Disable Next button if on last page
                if (currentPage >= totalPages) {
                    nextPageBtn.disabled = true;
                    nextPageBtn.classList.add('btn-disabled');
                } else {
                    nextPageBtn.disabled = false;
                    nextPageBtn.classList.remove('btn-disabled');
                }
            }
        };

        // Initialize Zoom Display and Button States
        updateButtonStates();
        updatePageIndicator();
    };
});
