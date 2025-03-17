document.addEventListener('DOMContentLoaded', async () => {
    // Load PDFs when page loads
    const pdfList = document.getElementById('pdf-list');
    try {
        const pdfs = await window.electronAPI.getPDFList();
        
        // Create PDF items
        pdfs.forEach(pdf => {
            const pdfItem = document.createElement('div');
            pdfItem.className = 'pdf-item';
            
            // Add PDF preview/icon
            const preview = document.createElement('div');
            preview.className = 'pdf-preview';
            preview.innerHTML = `
                <embed src="file://${pdf.path}#toolbar=0&navpanes=0&scrollbar=0" 
                       type="application/pdf" 
                       width="100%" 
                       height="150px">
                       <div class="click-overlay"></div>
            `;
            
            // Add PDF name
            const name = document.createElement('div');
            name.className = 'pdf-name';
            name.textContent = pdf.name;
            
            pdfItem.appendChild(preview);
            pdfItem.appendChild(name);
            
            // Add click handler
            pdfItem.addEventListener('click', () => viewPDF(pdf.path));
            pdfList.appendChild(pdfItem);
        });
    } catch (error) {
        console.error('Failed to load PDFs:', error);
    }
    
    // Click-off functionality
    document.querySelector('.viewer-overlay').addEventListener('click', () => {
        document.getElementById('pdf-viewer').style.display = 'none';
    });
    
    document.getElementById('close-viewer').addEventListener('click', () => {
        document.getElementById('pdf-viewer').style.display = 'none';
    });
});

function viewPDF(pdfPath) {
    const viewer = document.getElementById('pdf-viewer');
    const iframe = document.getElementById('pdf-frame');
    const pdfUrl = `file://${pdfPath}`;
    iframe.src = pdfUrl;
    iframe.setAttribute('data-path', pdfPath);
    viewer.style.display = 'flex';
}