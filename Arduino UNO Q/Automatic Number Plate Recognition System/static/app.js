const scanBtn = document.getElementById("rescanButton");
const list = document.getElementById("recentDetections");

scanBtn.onclick = async () => {
    await fetch("/capture", { method: "POST" });
    loadVehicles();
};

async function loadVehicles() {
    const res = await fetch("/vehicles");
    const vehicles = await res.json();

    list.innerHTML = "";

    vehicles.forEach(v => {
        const li = document.createElement("li");
        li.innerHTML = `
            <div class="scan-cell-container">
                <div>
                    <div class="scan-content"><b>${v.plate}</b></div>
                    <div class="scan-header">${v.status}</div>
                </div>
                <div class="scan-content-time">
                    ${v.status === "IN" ? v.entry_time : v.exit_time}
                </div>
            </div>
        `;
        list.appendChild(li);
    });
}

setInterval(loadVehicles, 3000);