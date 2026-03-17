// SPDX-FileCopyrightText: 2025 Blended Authors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Blended WebAssembly module loader and web shell glue.
// Handles: browser feature detection, WASM module loading with progress,
// canvas setup, file drag-and-drop, and virtual filesystem initialization.

(function () {
  "use strict";

  // ── DOM references ──────────────────────────────────────────────

  const canvas = document.getElementById("canvas");
  const overlay = document.getElementById("loading-overlay");
  const progressBar = document.getElementById("progress-bar");
  const progressText = document.getElementById("progress-text");
  const featureChecks = document.getElementById("feature-checks");
  const errorMessage = document.getElementById("error-message");
  const dropOverlay = document.getElementById("drop-overlay");

  // ── Browser feature detection ───────────────────────────────────

  function checkFeatures() {
    const features = [
      {
        name: "WebAssembly",
        ok: typeof WebAssembly === "object" && typeof WebAssembly.instantiate === "function",
      },
      {
        name: "WebGL2",
        ok: (function () {
          try {
            return !!document.createElement("canvas").getContext("webgl2");
          } catch (e) {
            return false;
          }
        })(),
      },
      {
        name: "SharedArrayBuffer",
        ok: typeof SharedArrayBuffer === "function",
      },
      {
        name: "Web Workers",
        ok: typeof Worker === "function",
      },
    ];

    let allOk = true;
    features.forEach(function (f) {
      const el = document.createElement("span");
      el.className = "feature-check " + (f.ok ? "ok" : "fail");
      el.textContent = (f.ok ? "\u2713 " : "\u2717 ") + f.name;
      featureChecks.appendChild(el);
      if (!f.ok) allOk = false;
    });

    return { allOk, features };
  }

  function showError(msg) {
    errorMessage.style.display = "block";
    errorMessage.textContent = msg;
  }

  // ── Canvas sizing ───────────────────────────────────────────────

  function resizeCanvas() {
    canvas.width = window.innerWidth * (window.devicePixelRatio || 1);
    canvas.height = window.innerHeight * (window.devicePixelRatio || 1);
    canvas.style.width = window.innerWidth + "px";
    canvas.style.height = window.innerHeight + "px";
  }

  window.addEventListener("resize", resizeCanvas);
  resizeCanvas();

  // ── Drag-and-drop for .blend files ──────────────────────────────

  document.addEventListener("dragenter", function (e) {
    e.preventDefault();
    dropOverlay.style.display = "flex";
  });

  dropOverlay.addEventListener("dragleave", function (e) {
    e.preventDefault();
    dropOverlay.style.display = "none";
  });

  dropOverlay.addEventListener("dragover", function (e) {
    e.preventDefault();
  });

  dropOverlay.addEventListener("drop", function (e) {
    e.preventDefault();
    dropOverlay.style.display = "none";

    var files = e.dataTransfer.files;
    for (var i = 0; i < files.length; i++) {
      var file = files[i];
      if (file.name.endsWith(".blend") || file.name.endsWith(".obj") || file.name.endsWith(".stl")) {
        loadFileIntoFS(file);
      }
    }
  });

  function loadFileIntoFS(file) {
    var reader = new FileReader();
    reader.onload = function (e) {
      if (typeof Module !== "undefined" && Module.FS) {
        var data = new Uint8Array(e.target.result);
        var path = "/tmp/" + file.name;
        Module.FS.writeFile(path, data);
        console.log("Wrote " + file.name + " to virtual FS at " + path);
        // TODO: Trigger Blended to open the file via WM operator
      }
    };
    reader.readAsArrayBuffer(file);
  }

  // ── Progress tracking ───────────────────────────────────────────

  function updateProgress(ratio, label) {
    var pct = Math.min(100, Math.round(ratio * 100));
    progressBar.style.width = pct + "%";
    progressText.textContent = label || "Loading... " + pct + "%";
  }

  function hideOverlay() {
    overlay.classList.add("fade-out");
    setTimeout(function () {
      overlay.style.display = "none";
    }, 600);
    canvas.focus();
  }

  // ── Module configuration & launch ───────────────────────────────

  function launchBlended() {
    updateProgress(0.05, "Loading WebAssembly module...");

    // The Emscripten-generated blended.js will define BlendedModule
    // as a factory function (due to MODULARIZE=1).
    if (typeof BlendedModule !== "function") {
      showError(
        "BlendedModule not found. The WASM build may not have completed successfully. " +
        "Check the build output for errors."
      );
      return;
    }

    var moduleConfig = {
      canvas: canvas,

      // Print stdout/stderr to the browser console.
      print: function (text) {
        console.log("[Blended]", text);
      },
      printErr: function (text) {
        console.warn("[Blended]", text);
      },

      // Track download progress.
      setStatus: function (text) {
        if (!text) {
          hideOverlay();
          return;
        }

        // Emscripten status strings look like:
        //   "Downloading data... (3/10)"
        //   "Running..."
        var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
        if (m) {
          var loaded = parseFloat(m[2]);
          var total = parseFloat(m[4]);
          updateProgress(loaded / total, m[1].trim());
        } else {
          progressText.textContent = text;
        }
      },

      // Called when the runtime is initialized.
      onRuntimeInitialized: function () {
        updateProgress(1.0, "Starting Blended...");
        // Short delay so the user sees "Starting..." before the UI takes over.
        setTimeout(hideOverlay, 200);
      },

      // Filesystem pre-run: create directories Blender expects.
      preRun: [
        function (mod) {
          // Create standard Blender directories in the virtual filesystem.
          var dirs = ["/tmp", "/home", "/home/web_user", "/home/web_user/.config"];
          dirs.forEach(function (d) {
            try {
              mod.FS.mkdir(d);
            } catch (e) {
              // Directory may already exist.
            }
          });

          // Set HOME so Blender finds its config directory.
          mod.ENV = mod.ENV || {};
          mod.ENV["HOME"] = "/home/web_user";
        },
      ],
    };

    BlendedModule(moduleConfig).catch(function (err) {
      showError("Failed to start Blended: " + (err.message || err));
      console.error(err);
    });
  }

  // ── Cross-origin isolation helper ──────────────────────────────
  // The coi-serviceworker registers on first visit and reloads the page
  // to enable SharedArrayBuffer.  If the SW is still activating we wait
  // briefly instead of immediately declaring the feature missing.

  function waitForCrossOriginIsolation(callback) {
    // Already isolated — proceed immediately.
    if (window.crossOriginIsolated) {
      callback();
      return;
    }

    // No service worker support — can't fix it, proceed with check.
    if (!("serviceWorker" in navigator)) {
      callback();
      return;
    }

    // Give the service worker up to 3 seconds to activate and reload.
    var waited = 0;
    var interval = 200;      // ms between checks
    var maxWait = 3000;       // ms total
    progressText.textContent = "Enabling cross-origin isolation...";

    var timer = setInterval(function () {
      waited += interval;
      if (window.crossOriginIsolated || waited >= maxWait) {
        clearInterval(timer);
        callback();
      }
    }, interval);
  }

  // ── Main entry point ────────────────────────────────────────────

  waitForCrossOriginIsolation(function () {
    var result = checkFeatures();

    if (!result.allOk) {
      var missing = result.features
        .filter(function (f) { return !f.ok; })
        .map(function (f) { return f.name; })
        .join(", ");

      showError(
        "Your browser is missing required features: " + missing + ". " +
        "Blended requires a modern browser with WebAssembly, WebGL2, " +
        "SharedArrayBuffer, and Web Worker support. " +
        "Please try the latest version of Chrome, Firefox, or Edge."
      );
    } else {
      launchBlended();
    }
  });
})();
