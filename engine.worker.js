importScripts("chess_engine.js");

let moduleInitialized = false;
let bookLoaded = false;

Module.onRuntimeInitialized = () => {
    moduleInitialized = true;
};

loadBook();

self.onmessage = async (e) => {
    // Wait for the module to be initialized
    while (!moduleInitialized) {
        await new Promise((resolve) => setTimeout(resolve, 100));
    }

    // Wait for the book to be loaded
    while (!bookLoaded) {
        await new Promise((resolve) => setTimeout(resolve, 100));
    }

    const { fen, bookPath, timeLeft, inc, moveTime } = e.data;

    const bookMove = Module.ccall(
        "getBookMove",
        "string",
        ["string", "string"],
        [bookPath, fen]
    );
    if (bookMove) {
        self.postMessage({ action: "getBookMove", result: bookMove });
    } else {
        const bestMove = Module.ccall(
            "getBestMove",
            "string",
            ["string", "number", "number", "number", "number"],
            [fen, timeLeft, inc, 0, moveTime]
        );
        self.postMessage({ action: "getBestMove", result: bestMove });
    }
};

async function loadBook() {
    const response = await fetch('komodo.bin');
    const bookData = await response.arrayBuffer();
    FS.createDataFile("/", "komodo.bin", new Uint8Array(bookData), true, false);
    bookLoaded = true;
}