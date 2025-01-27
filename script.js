var timeControl = document.getElementById("time-control").value;
var timePerMove = document.getElementById("time-per-move").value;
var playingAs = document.getElementById("playing-as").value;

var whiteTime, blackTime, increment, timer;
var isClockRunning = false;

const engineWorker = new Worker("engine.worker.js");

let gameHasEnded = false;

function initializeClock() {
    clearInterval(timer);
    switch (timeControl) {
        case "bullet": // 1 minute + 5 second increment
            whiteTime = 60;
            blackTime = 60;
            increment = 5;
            break;
        case "blitz": // 5 minutes + 10 second increment
            whiteTime = 300;
            blackTime = 300;
            increment = 10;
            break;
        case "rapid": // 10 minutes + 15 second increment
            whiteTime = 600;
            blackTime = 600;
            increment = 15;
            break;
        case "classical": // 30 minutes
            whiteTime = 1800;
            blackTime = 1800;
            break;
    }
    updateClock();
}

function updateClock() {
    document.getElementById("white-time").innerHTML = formatTime(whiteTime);
    document.getElementById("black-time").innerHTML = formatTime(blackTime);
}

function formatTime(time) {
    let minutes = Math.floor(time / 60);
    let seconds = time % 60;
    return `${minutes}:${seconds.toString().padStart(2, "0")}`;
}

function startClock() {
    if (!isClockRunning) {
        timer = setInterval(() => {
            if (chess.turn() == "w") {
                whiteTime--;
            } else {
                blackTime--;
            }

            if (whiteTime <= 0 || blackTime <= 0) {
                gameHasEnded = true;
                clearInterval(timer);
                gameOver();
            }

            updateClock();
        }, 1000);
        isClockRunning = true;
    }
}

var chessboard = new Chessboard("board", {
    position: "start",
    orientation: playingAs,
    draggable: true,
    onDragStart: onDragStart,
    onDrop: onDrop,
    onSnapEnd: onSnapEnd
});
var chess = new Chess();

function onDragStart(source, piece, position, orientation) {
    if (gameHasEnded) return false;
    if (chess.game_over()) return false;
    if (playingAs=="white") {
        if (piece.search(/^b/) !== -1) return false;
    } else {
        if (piece.search(/^w/) !== -1) return false;
    }
}

function checkIfGameOverAndUpdateClock() {
    // Update the clock
    if (increment) {
        if (playingAs == "white") {
            blackTime = parseInt(blackTime) + parseInt(increment);
        } else {
            whiteTime = parseInt(whiteTime) + parseInt(increment);
        }
        updateClock();
    } 

    // Game over
    if (chess.game_over()) {
        gameOver();
        return;
    };
}

function makeMove() {    
    var possibleMoves = chess.moves();

    // Game over
    if (possibleMoves.length === 0) {
        gameOver();
        return;
    };

    // Get the FEN string of current position 
    var fen = chess.fen();

    // Calculate time parameters 
    let moveTime = parseInt(timePerMove) * 1000;
    let timeLeft, inc;
    if (playingAs == "white") {
        timeLeft = parseInt(blackTime) * 1000;
    } else {
        timeLeft = parseInt(whiteTime) * 1000;
    }
    inc = parseInt(increment) * 1000;

    engineWorker.postMessage({
        fen: fen,
        bookPath: "/komodo.bin",
        timeLeft: timeLeft,
        inc: inc,
        moveTime: moveTime
    });

    engineWorker.onmessage = (e) => {
        const { action, result } = e.data;
        if (action==="getBookMove" && result) {
            const bookMove = result;
            const from = bookMove.substring(0, 2);
            const to = bookMove.substring(2, 4);
            const promotion = bookMove.length > 4 ? bookMove[4] : undefined;
            chess.move({
                from: from,
                to: to,
                promotion: promotion
            });
            chessboard.position(chess.fen());
            checkIfGameOverAndUpdateClock();
        } else if (action==="getBestMove") {
            const bestMove = result;
            const from = bestMove.substring(0, 2);
            const to = bestMove.substring(2, 4);
            const promotion = bestMove.length > 4 ? bestMove[4] : undefined;
            chess.move({
                from: from,
                to: to,
                promotion: promotion
            });
            chessboard.position(chess.fen());
            checkIfGameOverAndUpdateClock();
        }
    }
}

function onDrop (source, target) {
    // Check if move is legal
    var move = chess.move({
        from: source,
        to: target,
        promotion: 'q' // Default promotion to queen
    });

    // Illegal move
    if (move === null) return 'snapback';

    // Game over 
    if (chess.game_over()) {
        clearInterval(timer);
        gameOver();
        return;
    };

    // Update the clock
    if (increment) {
        if (playingAs == "white") {
            whiteTime = parseInt(whiteTime) + parseInt(increment);
        } else {
            blackTime = parseInt(blackTime) + parseInt(increment);
        }
        updateClock();
    }

    // Make engine move
    window.setTimeout(makeMove, 250);
}

function onSnapEnd () {
    chessboard.position(chess.fen());
}

async function makeGameVisible() {
    document.getElementById("engine").classList.remove("hidden");

    initializeClock();

    if (playingAs=="black") {
        chessboard.orientation("black");
        window.setTimeout(makeMove, 250);
    }

    startClock();
}

document.getElementById("start-match").addEventListener("click", function() {
    // Get the form data 
    timeControl = document.getElementById("time-control").value;
    timePerMove = document.getElementById("time-per-move").value;
    playingAs = document.getElementById("playing-as").value;

    if (playingAs=="random") {
        playingAs = Math.random() < 0.5 ? "white" : "black";
    }

    isClockRunning = false;
    clearInterval(timer);

    // Hide the form
    document.getElementById("options").classList.add("hidden");

    // Show the chessboard
    makeGameVisible();
});

document.getElementById("export-pgn").addEventListener("click", function() {
    navigator.clipboard.writeText(chess.pgn());
});

document.getElementById("undo-move").addEventListener("click", function() {
    if (chess.game_over()) return;
    if (chess.history().length < 2) return;
    if (playingAs=="black" && chess.turn() == "w") return;
    if (playingAs=="white" && chess.turn() == "b") return;

    chess.undo();
    chess.undo();
    chessboard.position(chess.fen());
});

function gameOver() {
    document.getElementById("game-result").classList.remove("hidden");
    let gameOverReason = document.getElementById("game-over-reason");
    let gameWinner = document.getElementById("winner");
    if (chess.in_checkmate()) {
        gameOverReason.innerHTML = "Checkmate";
        if (chess.turn()=="w") {
            gameWinner.innerHTML = "Black wins!";
        } else {
            gameWinner.innerHTML = "White wins!";
        }
    } else if (chess.in_draw()) {
        gameOverReason.innerHTML = "Draw";
        gameWinner.innerHTML = "The game is drawn!";
    } else if (chess.in_stalemate()) {
        gameOverReason.innerHTML = "Stalemate";
        gameWinner.innerHTML = "It's stalemate!";
    } else if (whiteTime <= 0) {
        gameOverReason.innerHTML = "Time out";
        gameWinner.innerHTML = "Black wins on time!";
    } else if (blackTime <= 0) {
        gameOverReason.innerHTML = "Time out";
        gameWinner.innerHTML = "White wins on time!";
    }
}

document.getElementById("close-popup").addEventListener("click", function() {
    document.getElementById("game-result").classList.add("hidden");
});