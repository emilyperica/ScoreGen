document.addEventListener('DOMContentLoaded', () => {
    const homeNavBtn = document.getElementById('home-nav-btn');
    const uploadNavBtn = document.getElementById('upload-nav-btn');
    const recordNavBtn = document.getElementById('record-nav-btn');
    const uploadBtn = document.getElementById('upload-btn');
    const recordBtn = document.getElementById('record-btn');
    const pdfBtn = document.getElementById('pdf-btn');
    document.getElementById('sheet-music-button').addEventListener('click', () => {
    window.electronAPI.navigateToSheetMusic();
});

    // Function to remove 'active' class from all nav buttons
    function resetActiveNav() {
        const navButtons = document.querySelectorAll('.btn-nav, .btn-nav-logo');
        navButtons.forEach(btn => btn.classList.remove('active'));
    }

    if (homeNavBtn) {
        homeNavBtn.addEventListener('click', () => {
            resetActiveNav();
            homeNavBtn.classList.add('active');
            window.electron.send('navigate-to-home');
        });
    } else {
        console.error("Home navigation button not found");
    }

    if (uploadNavBtn) {
        uploadNavBtn.addEventListener('click', () => {
            resetActiveNav();
            uploadNavBtn.classList.add('active');
            window.electron.send('navigate-to-upload');
        });
    } else {
        console.error("Upload navigation button not found");
    }

    if (recordNavBtn) {
        recordNavBtn.addEventListener('click', () => {
            resetActiveNav();
            recordNavBtn.classList.add('active');
            window.electron.send('navigate-to-record');
        });
    } else {
        console.error("Record navigation button not found");
    }

    if (uploadBtn) {
        uploadBtn.addEventListener('click', () => {
            resetActiveNav();
            uploadNavBtn.classList.add('active');
            window.electron.send('navigate-to-upload');
        });
    }
    
    if (recordBtn) {
        recordBtn.addEventListener('click', () => {
            resetActiveNav();
            recordNavBtn.classList.add('active');
            window.electron.send('navigate-to-record');
        });
    }

    if (pdfBtn) {
        pdfBtn.addEventListener('click', () => {
            resetActiveNav();
            pdfBtn.classList.add('active');
            window.electronAPI.navigateToSheetMusic();
        });
    }

    // Set active button based on the current page
    const currentPage = window.location.pathname.split('/').pop();
    if (currentPage === 'index.html') {
        resetActiveNav();
        homeNavBtn.classList.add('active');
    } else if (currentPage === 'upload.html') {
        resetActiveNav();
        uploadNavBtn.classList.add('active');
    } else if (currentPage === 'record.html') {
        resetActiveNav();
        recordNavBtn.classList.add('active');
    }
});