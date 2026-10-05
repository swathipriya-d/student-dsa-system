/**
 * Student Record Management System - Frontend App Engine
 * Connects Web Interface with the Python Flask Bridge & Native C DSA Core
 */

document.addEventListener("DOMContentLoaded", () => {
    // State
    let studentsCache = [];
    let undoStackCache = [];
    let requestsCache = [];
    let currentSortMode = "default";

    // Elements - Navigation
    const navItems = document.querySelectorAll(".nav-item");
    const tabPanes = document.querySelectorAll(".tab-pane");
    const pageTitle = document.getElementById("page-title");
    const pageDesc = document.getElementById("page-desc");

    // Elements - Directory
    const studentTbody = document.getElementById("student-tbody");
    const tableRecordCount = document.getElementById("table-record-count");
    const statTotalStudents = document.getElementById("stat-total-students");
    const statStackDepth = document.getElementById("stat-stack-depth");
    const statQueueCount = document.getElementById("stat-queue-count");
    const inputSearch = document.getElementById("input-search");
    const btnHashSearch = document.getElementById("btn-hash-search");
    const searchFeedback = document.getElementById("search-feedback");

    // Sort buttons
    const btnSortCgpaDesc = document.getElementById("btn-sort-cgpa-desc");
    const btnSortCgpaAsc = document.getElementById("btn-sort-cgpa-asc");
    const btnSortName = document.getElementById("btn-sort-name");
    const btnResetOrder = document.getElementById("btn-reset-order");

    // Undo Elements
    const btnQuickUndo = document.getElementById("btn-quick-undo");
    const btnDoUndo = document.getElementById("btn-do-undo");
    const undoResultBox = document.getElementById("undo-result-box");
    const stackVisualizer = document.getElementById("stack-visualizer");

    // Queue Elements
    const formEnqueueRequest = document.getElementById("form-enqueue-request");
    const selectQueueStudent = document.getElementById("select-queue-student");
    const selectQueueCertType = document.getElementById("select-queue-cert-type");
    const btnDequeueRequest = document.getElementById("btn-dequeue-request");
    const queueTrack = document.getElementById("queue-track");

    // BST Elements
    const bstTreeContainer = document.getElementById("bst-tree-container");
    const btnRefreshBst = document.getElementById("btn-refresh-bst");

    // Graph Elements
    const topologicalSequence = document.getElementById("topological-sequence");
    const graphCanvas = document.getElementById("graphCanvas");

    // Modals
    const btnOpenAddModal = document.getElementById("btn-open-add-modal");
    const modalAddStudent = document.getElementById("modal-add-student");
    const btnCloseAddModal = document.getElementById("btn-close-add-modal");
    const btnCancelAddModal = document.getElementById("btn-cancel-add-modal");
    const formAddStudent = document.getElementById("form-add-student");

    const modalEditStudent = document.getElementById("modal-edit-student");
    const btnCloseEditModal = document.getElementById("btn-close-edit-modal");
    const btnCancelEditModal = document.getElementById("btn-cancel-edit-modal");
    const formEditStudent = document.getElementById("form-edit-student");

    // =========================================================================
    // TAB NAVIGATION
    // =========================================================================

    const tabDescriptions = {
        "tab-directory": {
            title: "Student Records Directory",
            desc: "Backed by C Singly Linked List, Hash Table with Separate Chaining, and Merge Sort."
        },
        "tab-undo": {
            title: "Undo Desk (Stack ADT)",
            desc: "LIFO Stack implemented with dynamic pointer nodes in C for instant rollback."
        },
        "tab-queue": {
            title: "Certificate Service Desk (Queue ADT)",
            desc: "FIFO Queue with dynamic front/rear pointers for First-Come, First-Served ticket processing."
        },
        "tab-bst": {
            title: "Department Hierarchy (Binary Search Tree)",
            desc: "Lexicographically ordered BST with in-order traversal grouping students by department."
        },
        "tab-graph": {
            title: "Curriculum Prerequisites (Graph & Topological Sort)",
            desc: "Directed Acyclic Graph (DAG) resolved via Kahn's Algorithm for semester course progression."
        },
        "tab-viva": {
            title: "DSA Architecture & Viva Reference",
            desc: "Deep-dive into the C engine memory layout, algorithms, and console mode instructions."
        }
    };

    navItems.forEach(item => {
        item.addEventListener("click", () => {
            const targetTab = item.dataset.tab;
            navItems.forEach(n => n.classList.remove("active"));
            tabPanes.forEach(p => p.classList.remove("active"));

            item.classList.add("active");
            document.getElementById(targetTab).classList.add("active");

            if (tabDescriptions[targetTab]) {
                pageTitle.textContent = tabDescriptions[targetTab].title;
                pageDesc.textContent = tabDescriptions[targetTab].desc;
            }

            // Lazy-load view specific data
            if (targetTab === "tab-bst") loadBST();
            if (targetTab === "tab-graph") loadGraph();
            if (targetTab === "tab-queue") loadRequests();
        });
    });

    // =========================================================================
    // TOAST NOTIFICATIONS
    // =========================================================================

    function showToast(message, type = "success") {
        const container = document.getElementById("toast-container");
        const toast = document.createElement("div");
        toast.className = `toast toast-${type}`;
        toast.innerHTML = `
            <span>${type === "success" ? "✓" : "⚠"}</span>
            <div>${message}</div>
        `;
        container.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = "0";
            toast.style.transform = "translateX(20px)";
            setTimeout(() => toast.remove(), 300);
        }, 3500);
    }

    // =========================================================================
    // API CALLS: STUDENTS DIRECTORY
    // =========================================================================

    async function loadStudents() {
        try {
            const res = await fetch("/api/students");
            const data = await res.json();
            if (data.status === "success") {
                studentsCache = data.students || [];
                renderStudentTable(studentsCache);
                updateStats();
                populateStudentSelectDropdown();
            } else {
                showToast("Failed to load students: " + data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Connection error with backend bridge", "error");
        }
    }

    function renderStudentTable(students) {
        studentTbody.innerHTML = "";
        tableRecordCount.textContent = `Showing ${students.length} record(s)`;

        if (students.length === 0) {
            studentTbody.innerHTML = `
                <tr>
                    <td colspan="7" class="loading-state">No student records found in C linked list.</td>
                </tr>
            `;
            return;
        }

        students.forEach(s => {
            const tr = document.createElement("tr");

            // CGPA class
            let cgpaClass = "cgpa-mid";
            if (s.cgpa >= 9.0) cgpaClass = "cgpa-high";
            else if (s.cgpa < 7.5) cgpaClass = "cgpa-low";

            tr.innerHTML = `
                <td><span class="roll-badge">${s.roll}</span></td>
                <td class="student-name-cell">${escapeHtml(s.name)}</td>
                <td><span class="dept-badge">${escapeHtml(s.dept)}</span></td>
                <td><span class="cgpa-pill ${cgpaClass}">${s.cgpa.toFixed(2)}</span></td>
                <td>${escapeHtml(s.email || "—")}</td>
                <td>${escapeHtml(s.phone || "—")}</td>
                <td class="table-actions">
                    <button class="action-btn edit" title="Edit record" data-roll="${s.roll}">
                        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M17 3a2.85 2.83 0 1 1 4 4L7.5 20.5 2 22l1.5-5.5Z"/></svg>
                    </button>
                    <button class="action-btn delete" title="Delete record" data-roll="${s.roll}">
                        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M3 6h18"/><path d="M19 6v14c0 1-1 2-2 2H7c-1 0-2-1-2-2V6"/><path d="M8 6V4c0-1 1-2 2-2h4c1 0 2 1 2 2v2"/></svg>
                    </button>
                </td>
            `;

            // Attach event listeners
            tr.querySelector(".action-btn.edit").addEventListener("click", () => openEditModal(s));
            tr.querySelector(".action-btn.delete").addEventListener("click", () => deleteStudentRecord(s.roll, s.name));

            studentTbody.appendChild(tr);
        });
    }

    function updateStats() {
        statTotalStudents.textContent = studentsCache.length;
        statQueueCount.textContent = requestsCache.length;
    }

    function populateStudentSelectDropdown() {
        selectQueueStudent.innerHTML = `<option value="">Select registered student...</option>`;
        studentsCache.forEach(s => {
            const opt = document.createElement("option");
            opt.value = s.roll;
            opt.textContent = `[Roll ${s.roll}] ${s.name} (${s.dept})`;
            selectQueueStudent.appendChild(opt);
        });
    }

    // =========================================================================
    // SEARCH & SORT HANDLERS
    // =========================================================================

    // Linear Search by Name (Substring matching)
    let searchDebounce = null;
    inputSearch.addEventListener("input", (e) => {
        clearTimeout(searchDebounce);
        const query = e.target.value.trim();

        if (!query) {
            searchFeedback.classList.add("d-none");
            renderStudentTable(studentsCache);
            return;
        }

        searchDebounce = setTimeout(async () => {
            // Check if numeric (Roll No search)
            if (/^\d+$/.test(query)) {
                executeHashSearch(parseInt(query));
                return;
            }

            try {
                const res = await fetch(`/api/students/search/name?q=${encodeURIComponent(query)}`);
                const data = await res.json();
                if (data.status === "success") {
                    searchFeedback.classList.remove("d-none");
                    searchFeedback.innerHTML = `<strong>Linear Search O(n) in C:</strong> Found ${data.count} student(s) matching "<em>${escapeHtml(query)}</em>".`;
                    renderStudentTable(data.results);
                }
            } catch (err) {
                console.error(err);
            }
        }, 250);
    });

    // O(1) Hash Table Lookup
    btnHashSearch.addEventListener("click", () => {
        const query = inputSearch.value.trim();
        const roll = parseInt(query);
        if (isNaN(roll)) {
            showToast("Enter a numeric Roll Number for Hash Table search", "error");
            return;
        }
        executeHashSearch(roll);
    });

    async function executeHashSearch(roll) {
        try {
            const res = await fetch(`/api/students/search/roll/${roll}`);
            const data = await res.json();
            searchFeedback.classList.remove("d-none");

            if (data.found && data.student) {
                searchFeedback.innerHTML = `
                    <strong>⚡ Hash Table Hit (O(1) Average Time):</strong> 
                    Resolved slot <code>hash(${roll}) = ${roll % 17}</code> in separate chaining buckets. 
                    Binary search index in sorted array: <code>${data.bSearchIndex}</code>.
                `;
                renderStudentTable([data.student]);
            } else {
                searchFeedback.innerHTML = `<strong>Hash Table &amp; Binary Search:</strong> No record found for Roll Number <code>${roll}</code>.`;
                renderStudentTable([]);
            }
        } catch (err) {
            console.error(err);
        }
    }

    // Sort buttons: Merge Sort in C
    function resetSortChips() {
        btnSortCgpaDesc.classList.remove("active");
        btnSortCgpaAsc.classList.remove("active");
        btnSortName.classList.remove("active");
        btnResetOrder.classList.remove("active");
    }

    btnSortCgpaDesc.addEventListener("click", async () => {
        resetSortChips();
        btnSortCgpaDesc.classList.add("active");
        const res = await fetch("/api/students/sort/cgpa?order=desc");
        const data = await res.json();
        if (data.status === "success") {
            renderStudentTable(data.students);
            showToast("Sorted by CGPA (Merge Sort Descending)", "success");
        }
    });

    btnSortCgpaAsc.addEventListener("click", async () => {
        resetSortChips();
        btnSortCgpaAsc.classList.add("active");
        const res = await fetch("/api/students/sort/cgpa?order=asc");
        const data = await res.json();
        if (data.status === "success") {
            renderStudentTable(data.students);
            showToast("Sorted by CGPA (Merge Sort Ascending)", "success");
        }
    });

    btnSortName.addEventListener("click", async () => {
        resetSortChips();
        btnSortName.classList.add("active");
        const res = await fetch("/api/students/sort/name?order=asc");
        const data = await res.json();
        if (data.status === "success") {
            renderStudentTable(data.students);
            showToast("Sorted by Name Alphabetically (Merge Sort)", "success");
        }
    });

    btnResetOrder.addEventListener("click", () => {
        resetSortChips();
        btnResetOrder.classList.add("active");
        inputSearch.value = "";
        searchFeedback.classList.add("d-none");
        loadStudents();
    });

    // =========================================================================
    // ADD & EDIT MODALS
    // =========================================================================

    btnOpenAddModal.addEventListener("click", () => modalAddStudent.classList.remove("d-none"));
    btnCloseAddModal.addEventListener("click", () => modalAddStudent.classList.add("d-none"));
    btnCancelAddModal.addEventListener("click", () => modalAddStudent.classList.add("d-none"));

    formAddStudent.addEventListener("submit", async (e) => {
        e.preventDefault();
        const payload = {
            roll: parseInt(document.getElementById("add-roll").value),
            name: document.getElementById("add-name").value.trim(),
            dept: document.getElementById("add-dept").value,
            cgpa: parseFloat(document.getElementById("add-cgpa").value),
            email: document.getElementById("add-email").value.trim(),
            phone: document.getElementById("add-phone").value.trim()
        };

        try {
            const res = await fetch("/api/students", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });
            const data = await res.json();
            if (data.status === "success") {
                modalAddStudent.classList.add("d-none");
                formAddStudent.reset();
                showToast(`Student [${payload.roll}] added to Linked List & Hash Table!`, "success");
                loadStudents();
            } else {
                showToast(data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Failed to add student", "error");
        }
    });

    function openEditModal(student) {
        document.getElementById("edit-roll").value = student.roll;
        document.getElementById("edit-name").value = student.name;
        document.getElementById("edit-dept").value = student.dept;
        document.getElementById("edit-cgpa").value = student.cgpa;
        document.getElementById("edit-email").value = student.email || "";
        document.getElementById("edit-phone").value = student.phone || "";
        modalEditStudent.classList.remove("d-none");
    }

    btnCloseEditModal.addEventListener("click", () => modalEditStudent.classList.add("d-none"));
    btnCancelEditModal.addEventListener("click", () => modalEditStudent.classList.add("d-none"));

    formEditStudent.addEventListener("submit", async (e) => {
        e.preventDefault();
        const roll = parseInt(document.getElementById("edit-roll").value);
        const payload = {
            name: document.getElementById("edit-name").value.trim(),
            dept: document.getElementById("edit-dept").value,
            cgpa: parseFloat(document.getElementById("edit-cgpa").value),
            email: document.getElementById("edit-email").value.trim(),
            phone: document.getElementById("edit-phone").value.trim()
        };

        try {
            const res = await fetch(`/api/students/${roll}`, {
                method: "PUT",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });
            const data = await res.json();
            if (data.status === "success") {
                modalEditStudent.classList.add("d-none");
                showToast(`Record updated! Old state saved to Undo Stack.`, "success");
                recordUndoAction({
                    type: "UPDATE",
                    title: `Update Student #${roll}`,
                    desc: `${payload.name} (CGPA: ${payload.cgpa.toFixed(2)})`
                });
                loadStudents();
            } else {
                showToast(data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Failed to update student", "error");
        }
    });

    async function deleteStudentRecord(roll, name) {
        if (!confirm(`Are you sure you want to delete student [${roll}] ${name}? This action can be undone via the Stack.`)) {
            return;
        }

        try {
            const res = await fetch(`/api/students/${roll}`, { method: "DELETE" });
            const data = await res.json();
            if (data.status === "success") {
                showToast(`Student [${roll}] deleted. State pushed to Undo Stack.`, "success");
                recordUndoAction({
                    type: "DELETE",
                    title: `Delete Student #${roll}`,
                    desc: `${name}`
                });
                loadStudents();
            } else {
                showToast(data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Failed to delete student", "error");
        }
    }

    // =========================================================================
    // STACK ADT: UNDO FUNCTIONALITY
    // =========================================================================

    function recordUndoAction(action) {
        undoStackCache.push(action);
        renderStackVisualizer();
        statStackDepth.textContent = undoStackCache.length;
    }

    function renderStackVisualizer() {
        stackVisualizer.innerHTML = `<div class="stack-bottom-line"><span>BOTTOM OF STACK (NULL)</span></div>`;
        if (undoStackCache.length === 0) {
            const emptyNotice = document.createElement("div");
            emptyNotice.className = "empty-state";
            emptyNotice.textContent = "Undo Stack is currently empty (No update/delete operations pending rollback).";
            stackVisualizer.prepend(emptyNotice);
            return;
        }

        undoStackCache.forEach((item, idx) => {
            const isTop = idx === undoStackCache.length - 1;
            const node = document.createElement("div");
            node.className = `stack-node-item ${isTop ? "top-node" : ""}`;
            node.innerHTML = `
                <div>
                    <div class="stack-node-title">${item.title}</div>
                    <div class="stack-node-desc">${item.desc}</div>
                </div>
                ${isTop ? `<span class="stack-top-badge">▲ TOP</span>` : ""}
            `;
            stackVisualizer.prepend(node);
        });
    }

    async function executeUndo() {
        try {
            const res = await fetch("/api/undo", { method: "POST" });
            const data = await res.json();
            if (data.status === "success") {
                showToast(`Stack POP: Restored student [${data.restored.roll}] ${data.restored.name}!`, "success");
                undoResultBox.classList.remove("d-none");
                undoResultBox.innerHTML = `
                    <div style="color: #6ee7b7; font-weight: 700; margin-bottom: 6px;">✓ Action Restored Successfully via C Stack POP</div>
                    <div>Roll No: <strong>${data.restored.roll}</strong> | Name: <strong>${data.restored.name}</strong> | CGPA: <strong>${data.restored.cgpa.toFixed(2)}</strong></div>
                    <div style="font-size: 0.75rem; color: var(--text-muted); margin-top: 4px;">Remaining Stack Depth in C: ${data.stackRemaining}</div>
                `;
                if (undoStackCache.length > 0) undoStackCache.pop();
                renderStackVisualizer();
                statStackDepth.textContent = data.stackRemaining;
                loadStudents();
            } else {
                showToast(data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Failed to perform undo operation", "error");
        }
    }

    btnQuickUndo.addEventListener("click", executeUndo);
    btnDoUndo.addEventListener("click", executeUndo);

    // =========================================================================
    // QUEUE ADT: CERTIFICATE REQUESTS
    // =========================================================================

    async function loadRequests() {
        try {
            const res = await fetch("/api/requests");
            const data = await res.json();
            if (data.status === "success") {
                requestsCache = data.requests || [];
                renderQueuePipeline(requestsCache);
                statQueueCount.textContent = requestsCache.length;
            }
        } catch (err) {
            console.error(err);
        }
    }

    function renderQueuePipeline(requests) {
        queueTrack.innerHTML = "";
        if (requests.length === 0) {
            queueTrack.innerHTML = `<div class="empty-state">No pending requests in FIFO Queue.</div>`;
            return;
        }

        requests.forEach((req, idx) => {
            const isFront = idx === 0;
            const card = document.createElement("div");
            card.className = `queue-ticket-card ${isFront ? "front-ticket" : ""}`;
            card.innerHTML = `
                <div class="ticket-header">
                    <span>#${req.reqId}</span>
                    <span style="color: ${isFront ? '#34d399' : '#9ca3af'}; font-weight: 700;">${isFront ? 'FRONT' : `Slot #${idx + 1}`}</span>
                </div>
                <div class="ticket-type">${escapeHtml(req.type)}</div>
                <div class="ticket-student">${escapeHtml(req.name)} (Roll ${req.roll})</div>
            `;
            queueTrack.appendChild(card);
        });
    }

    formEnqueueRequest.addEventListener("submit", async (e) => {
        e.preventDefault();
        const roll = parseInt(selectQueueStudent.value);
        const certType = selectQueueCertType.value;
        if (!roll) {
            showToast("Please select a student", "error");
            return;
        }

        try {
            const res = await fetch("/api/requests", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ roll, type: certType })
            });
            const data = await res.json();
            if (data.status === "success") {
                showToast(`Ticket #${data.request.reqId} enqueued at REAR!`, "success");
                loadRequests();
            } else {
                showToast(data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Failed to enqueue request", "error");
        }
    });

    btnDequeueRequest.addEventListener("click", async () => {
        try {
            const res = await fetch("/api/requests", { method: "DELETE" });
            const data = await res.json();
            if (data.status === "success") {
                showToast(`Dequeued FRONT ticket #${data.processed.reqId}: ${data.processed.type} for ${data.processed.name}!`, "success");
                loadRequests();
            } else {
                showToast(data.message, "error");
            }
        } catch (err) {
            console.error(err);
            showToast("Failed to process request", "error");
        }
    });

    // =========================================================================
    // BST: DEPARTMENT HIERARCHY
    // =========================================================================

    async function loadBST() {
        try {
            const res = await fetch("/api/departments");
            const data = await res.json();
            if (data.status === "success") {
                renderBST(data.departments);
            }
        } catch (err) {
            console.error(err);
        }
    }

    function renderBST(departments) {
        bstTreeContainer.innerHTML = "";
        if (!departments || departments.length === 0) {
            bstTreeContainer.innerHTML = `<div class="empty-state">No departments in BST.</div>`;
            return;
        }

        departments.forEach(dept => {
            const card = document.createElement("div");
            card.className = "bst-dept-card";

            let studentsListHtml = "";
            (dept.students || []).forEach(s => {
                studentsListHtml += `
                    <div class="bst-student-item">
                        <span><strong>[${s.roll}]</strong> ${escapeHtml(s.name)}</span>
                        <span style="color: #34d399; font-weight: 700;">${s.cgpa.toFixed(2)}</span>
                    </div>
                `;
            });

            card.innerHTML = `
                <div class="bst-dept-title">
                    <span>${escapeHtml(dept.dept)}</span>
                    <span class="bst-dept-count">${dept.studentCount} enrolled</span>
                </div>
                <div class="bst-students-list">
                    ${studentsListHtml || '<div class="empty-state" style="padding: 10px;">No students</div>'}
                </div>
            `;
            bstTreeContainer.appendChild(card);
        });
    }

    btnRefreshBst.addEventListener("click", loadBST);

    // =========================================================================
    // GRAPH: CURRICULUM PREREQUISITES & TOPOLOGICAL SORT
    // =========================================================================

    async function loadGraph() {
        try {
            const res = await fetch("/api/prerequisites");
            const data = await res.json();
            if (data.status === "success") {
                renderTopologicalOrder(data.nodes, data.topologicalOrder);
                drawGraphCanvas(data.nodes, data.edges);
            }
        } catch (err) {
            console.error(err);
        }
    }

    function renderTopologicalOrder(nodes, order) {
        topologicalSequence.innerHTML = "";
        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        order.forEach((id, idx) => {
            const node = nodeMap[id];
            if (!node) return;

            const seqNode = document.createElement("div");
            seqNode.className = "seq-node";
            seqNode.innerHTML = `
                <span class="seq-num">${idx + 1}</span>
                <div>
                    <div class="seq-title">${node.code}: ${escapeHtml(node.title)}</div>
                    <div style="font-size: 0.72rem; color: var(--text-secondary); font-family: var(--font-mono);">Sem ${node.semester} • ${node.credits} Credits</div>
                </div>
            `;
            topologicalSequence.appendChild(seqNode);

            if (idx < order.length - 1) {
                const arrow = document.createElement("span");
                arrow.className = "seq-arrow";
                arrow.textContent = "➔";
                topologicalSequence.appendChild(arrow);
            }
        });
    }

    function drawGraphCanvas(nodes, edges) {
        const canvas = graphCanvas;
        const ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Compute layout positions organized by semester
        const semColumns = { 1: 100, 2: 280, 3: 460, 4: 640, 5: 800 };
        const semCounts = { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 };
        const positions = {};

        nodes.forEach(n => {
            const colX = semColumns[n.semester] || 450;
            const rowIdx = semCounts[n.semester]++;
            const rowY = 100 + rowIdx * 120;
            positions[n.id] = { x: colX, y: rowY };
        });

        // Draw directed edges (prerequisite arrows)
        edges.forEach(edge => {
            const from = positions[edge.source];
            const to = positions[edge.target];
            if (!from || !to) return;

            ctx.beginPath();
            ctx.moveTo(from.x, from.y);
            ctx.lineTo(to.x, to.y);
            ctx.strokeStyle = "rgba(99, 102, 241, 0.4)";
            ctx.lineWidth = 2.5;
            ctx.stroke();

            // Draw arrowhead
            const angle = Math.atan2(to.y - from.y, to.x - from.x);
            const radius = 34; // offset from destination node center
            const arrowX = to.x - radius * Math.cos(angle);
            const arrowY = to.y - radius * Math.sin(angle);

            ctx.beginPath();
            ctx.moveTo(arrowX, arrowY);
            ctx.lineTo(arrowX - 10 * Math.cos(angle - Math.PI / 6), arrowY - 10 * Math.sin(angle - Math.PI / 6));
            ctx.lineTo(arrowX - 10 * Math.cos(angle + Math.PI / 6), arrowY - 10 * Math.sin(angle + Math.PI / 6));
            ctx.fillStyle = "#818cf8";
            ctx.fill();
        });

        // Draw course nodes
        nodes.forEach(n => {
            const pos = positions[n.id];
            if (!pos) return;

            // Outer ring
            ctx.beginPath();
            ctx.arc(pos.x, pos.y, 34, 0, Math.PI * 2);
            ctx.fillStyle = "#1e1b4b";
            ctx.fill();
            ctx.strokeStyle = "#6366f1";
            ctx.lineWidth = 2.5;
            ctx.stroke();

            // Course code
            ctx.font = "bold 11px 'JetBrains Mono', monospace";
            ctx.fillStyle = "#ffffff";
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.fillText(n.code, pos.x, pos.y - 7);

            // Semester text
            ctx.font = "9px 'Plus Jakarta Sans', sans-serif";
            ctx.fillStyle = "#a5b4fc";
            ctx.fillText(`Sem ${n.semester}`, pos.x, pos.y + 9);

            // Course title below node
            ctx.font = "11px 'Plus Jakarta Sans', sans-serif";
            ctx.fillStyle = "#cbd5e1";
            ctx.fillText(n.title, pos.x, pos.y + 48);
        });
    }

    // =========================================================================
    // UTILITIES
    // =========================================================================

    function escapeHtml(str) {
        if (!str) return "";
        return str.replace(/[&<>'"]/g,
            tag => ({
                '&': '&amp;',
                '<': '&lt;',
                '>': '&gt;',
                "'": '&#39;',
                '"': '&quot;'
            }[tag] || tag)
        );
    }

    // Initial load
    loadStudents();
    loadRequests();
    renderStackVisualizer();
});
