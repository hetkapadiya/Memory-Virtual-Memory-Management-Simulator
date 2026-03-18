function openTab(evt, tabName) {
    let contents = document.getElementsByClassName("tab-content");
    for (let i = 0; i < contents.length; i++) contents[i].classList.remove("active");
    
    let buttons = document.getElementsByClassName("tab-btn");
    for (let i = 0; i < buttons.length; i++) buttons[i].classList.remove("active");

    document.getElementById(tabName).classList.add("active");
    evt.currentTarget.classList.add("active");
}

async function triggerSimulation() {
    const blocks = document.getElementById('blocks-in').value;
    const procs = document.getElementById('procs-in').value;
    const algo = document.getElementById('algo-in').value;

    if (!blocks || !procs) return alert("Please enter values");

    try {
        const response = await fetch('http://localhost:5000/simulate', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ blocks, procs, algo })
        });
        const data = await response.json();
        renderRAM(data.blocks);
        updateStats(data.blocks);
    } catch (err) {
        alert("Server not running! Run python server.py first.");
    }
}

function renderRAM(blocks) {
    const grid = document.getElementById('ram-grid');
    grid.innerHTML = '';

    blocks.forEach(b => {
        const cell = document.createElement('div');
        cell.className = 'ram-cell';
        if (b.p_id === -1) {
            cell.classList.add('c-free');
            cell.innerHTML = `<span>--</span><small>Free: ${b.size}K</small>`;
        } else {
            const colors = ['c-p1', 'c-p2', 'c-p3'];
            cell.classList.add(colors[(b.p_id - 1) % 3]);
            cell.innerHTML = `<span>P${b.p_id}</span><small>Size: ${b.size - b.frag}K</small>`;
        }
        grid.appendChild(cell);
    });
}

function updateStats(blocks) {
    let internal = 0;
    let external = 0;
    blocks.forEach(b => {
        if (b.p_id !== -1) internal += b.frag;
        else external += b.size;
    });
    document.getElementById('stat-int').innerText = internal;
    document.getElementById('stat-ext').innerText = external;
}

async function runComparison() {

    const blocks = document.getElementById('blocks-in').value;
    const procs = document.getElementById('procs-in').value;

    const res = await fetch('http://localhost:5000/simulate', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({
            blocks,
            procs,
            mode: "compare"
        })
    });

    const data = await res.json();

    console.log("COMPARE DATA:", data);

    if (!data.results) {
        alert("Comparison failed");
        return;
    }

    const body = document.getElementById("comp-body");
    body.innerHTML = "";

    data.results.forEach(r => {
        body.innerHTML += `
            <tr>
                <td>${r.name}</td>
                <td>${r.frag} KB</td>
            </tr>
        `;
    });
}

function runPaging() {

    const refStr = document.getElementById('ref-string').value;
    const frames = document.getElementById('frame-count').value;
    const algorithm = document.getElementById('page-algo').value;

    fetch("http://localhost:5000/simulate", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            ref: refStr,
            frames: frames,
            algorithm: algorithm,
            mode: "paging"
        })
    })
    .then(res => res.json())
    .then(data => {

    console.log("PAGING DATA:", data);

    const container = document.getElementById("paging-table-container");

    let html = "<table><thead><tr><th>Step</th>";

    // Header (steps)
    for (let i = 0; i < data.steps.length; i++) {
        html += `<th>${i + 1}</th>`;
    }

    html += "</tr></thead><tbody>";

    // Number of frames = length of each step array
    const frameCount = data.steps[0].length;

    // Rows = frames
    for (let f = 0; f < frameCount; f++) {
        html += `<tr><td>Frame ${f + 1}</td>`;

        for (let s = 0; s < data.steps.length; s++) {
            const val = data.steps[s][f] || "-";
            const cellClass = val === "-" ? "empty-cell" : "filled-cell";
            html += `<td class="${cellClass}">${val}</td>`;
        }

        html += "</tr>";
    }

    html += "</tbody></table>";

    container.innerHTML = html;

})
.catch(err => {
    console.error(err);
    alert("Paging backend error");
});
}