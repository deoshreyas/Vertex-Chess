import pygame 
from pygame.locals import * 
import pygame.freetype
import chess
import ctypes
import threading

pygame.init()

PLAYING_AS = 'white'
FIXED_TIME_FOR_MOVE = 5 # in seconds (prioritised over internal time management logic)
time_control = [900, 900, 10] # in seconds (white, black, increment)
FLIP_BOARD = PLAYING_AS == 'white'
FEN = None

# Load the DLL
chess_lib = ctypes.windll.LoadLibrary(r"./chess_engine.dll")

# Define argument and return types for the function
chess_lib.getBestMove.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
chess_lib.getBestMove.restype = ctypes.c_char_p
chess_lib.getBookMove.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
chess_lib.getBookMove.restype = ctypes.c_char_p

# WINDOW SETTINGS
WIDTH, HEIGHT = 500, 500
WINDOW_WIDTH, WINDOW_HEIGHT = 550, 650
WIN = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("The Vertex Chess Engine")
WIN_ALPHA = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)

# COLORS
WHITE = (248, 231, 187)
BLACK = (102, 0, 0)
BACKGROUND = (25, 25, 25)

# FONTS 
pygame.font.init()
font = pygame.font.SysFont(None, 24)

# LEFT AND TOP GAP FOR THE BOARD
# LEFT_GAP = WINDOW_WIDTH//2 - WIDTH//2
LEFT_GAP = 25
TOP_GAP = WINDOW_HEIGHT//2 - HEIGHT//2

# Create mapping of FEN characters to image file names for the pieces 
SQ_WIDTH = WIDTH // 8
SQ_HEIGHT = HEIGHT // 8
piece_images = {}
for piece in 'prnbqkPRNBQK':
    prefix = 'w' if piece.isupper() else 'b'
    filename = f'assets\\{prefix+piece.upper()}.png'
    img = pygame.image.load(filename)
    piece_images[piece] = img

# ENGINE 
book_path = "komodo.bin"
def getEngineMove(board):
    fen = board.fen()

    # try getting a book move first 
    book_move = chess_lib.getBookMove(book_path.encode('utf-8'), fen.encode('utf-8')).decode('utf-8')
    if book_move:
        return chess.Move.from_uci(book_move)
    
    # get time control parameters
    time_left = time_control[1] if board.turn == chess.BLACK else time_control[0]
    time_left = int(time_left * 1000)
    increment = time_control[2] * 1000 
    moves_to_go = 40 

    best_move = chess_lib.getBestMove(fen.encode('utf-8'), time_left, increment, moves_to_go, FIXED_TIME_FOR_MOVE*1000).decode('utf-8')
    return chess.Move.from_uci(best_move)

def engineMoveThread(board):
    global engine_thinking, engineToMove 
    engine_move = getEngineMove(board)
    if board.turn == chess.WHITE:
        time_control[0] += time_control[2]
    else:
        time_control[1] += time_control[2]
    board.push(engine_move)
    engineToMove = False
    engine_thinking = False

# BUTTON CLASS 
class Button:
    def __init__(self, x, y, width, height, color, text, font, font_color, font_size):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.color = color
        self.text = text
        self.font = font
        self.font_color = font_color
        self.font_size = font_size
    def draw(self):
        pygame.draw.rect(WIN, self.color, (self.x, self.y, self.width, self.height))
        font = pygame.font.SysFont(self.font, self.font_size)
        text = font.render(self.text, True, self.font_color)
        WIN.blit(text, (self.x + self.width//2 - text.get_width()//2, self.y + self.height//2 - text.get_height()//2))
    def clicked(self):
        mouseX, mouseY = pygame.mouse.get_pos()
        if self.x <= mouseX <= self.x + self.width and self.y <= mouseY <= self.y + self.height:
            return True
        return False
  
last_update_time = pygame.time.get_ticks()
CLOCK_UPDATE_INTERVAL = 1000
title_text = "The Vertex Chess Engine"
title_subtext = f"Playing as {PLAYING_AS.capitalize()}"
game_over = False 

# FUNCTIONS
def updateTimeControl(board):
    global last_update_time, time_control, title_text, title_subtext, game_over
    if not board.is_game_over():
        current_time = pygame.time.get_ticks()
        elapsed = current_time - last_update_time
        if elapsed >= CLOCK_UPDATE_INTERVAL:
            elapsed_seconds = elapsed // 1000
            if board.turn == chess.WHITE:
                time_control[0] -= elapsed_seconds
                if time_control[0] <= 0:
                    title_text = "Timeout"
                    title_subtext = "Black wins"
                    game_over = True
            else:
                time_control[1] -= elapsed_seconds
                if time_control[1] <= 0:
                    title_text = "Timeout"
                    title_subtext = "White wins"
                    game_over = True
            last_update_time = current_time
    
def drawTimeControl():
    time_white = max(0, int(time_control[0]))
    time_black = max(0, int(time_control[1]))
    time_font = pygame.freetype.SysFont("monsterrat", 25)
    # white 
    minutes, seconds = divmod(time_white, 60)
    text = f"{minutes}:{seconds:02d}"
    pygame.draw.rect(WIN, (255, 255, 255), (LEFT_GAP+WIDTH - time_font.get_rect(text).width - 90, HEIGHT+TOP_GAP+10, 70, 30))
    time_font.render_to(WIN, (LEFT_GAP + WIDTH - time_font.get_rect(text).width - 85, HEIGHT+TOP_GAP+15), text, (0, 0, 0))
    # black
    minutes, seconds = divmod(time_black, 60)
    text = f"{minutes}:{seconds:02d}"
    pygame.draw.rect(WIN, (0, 0, 0), (LEFT_GAP+WIDTH - time_font.get_rect(text).width - 10, HEIGHT+TOP_GAP+10, 70, 30))
    time_font.render_to(WIN, (LEFT_GAP + WIDTH - time_font.get_rect(text).width - 5, HEIGHT+TOP_GAP+15), text, (255, 255, 255))

def drawTitleTop(board):
    global title_text, title_subtext, game_over
    if board.is_game_over():
        game_over = True
        if board.is_checkmate():
            outcome = board.outcome()
            if outcome.winner == chess.WHITE:
                title_text = "Checkmate"
                title_subtext = "White wins"
            else:
                title_text = "Checkmate"
                title_subtext = "Black wins"
        elif board.is_stalemate():
            title_text = "Draw"
            title_subtext = "Stalemate"
        elif board.is_insufficient_material():
            title_text = "Draw"
            title_subtext = "Insufficient material"
        elif board.is_seventyfive_moves():
            title_text = "Draw"
            title_subtext = "75 moves rule"
        elif board.is_fivefold_repetition():
            title_text = "Draw"
            title_subtext = "Fivefold repetition" 
    font1 = pygame.freetype.SysFont("monsterrat", 28)
    font2 = pygame.freetype.SysFont("monsterrat", 18)
    font1.render_to(WIN, ((LEFT_GAP+WIDTH) - WIDTH//2 - font1.get_rect(title_text).width//2, 15), title_text, (255, 255, 255))
    font2.render_to(WIN, ((LEFT_GAP+WIDTH) - WIDTH//2 - font2.get_rect(title_subtext).width//2, 42), title_subtext, (255, 255, 255))

def drawSquaresOverLastMove(board):
    if len(board.move_stack) > 0:
        last_move = board.peek()
        from_row = chess.square_rank(last_move.from_square)
        from_col = chess.square_file(last_move.from_square)
        to_row = chess.square_rank(last_move.to_square)
        to_col = chess.square_file(last_move.to_square)
        if FLIP_BOARD:
            from_row = 7 - from_row
            to_row = 7 - to_row
        else:
            from_col = 7 - from_col 
            to_col = 7 - to_col
        pygame.draw.rect(WIN, (175, 100, 100), (from_col * SQ_WIDTH + LEFT_GAP, from_row * SQ_HEIGHT + TOP_GAP, SQ_WIDTH, SQ_HEIGHT))
        pygame.draw.rect(WIN, (255, 125, 125), (to_col * SQ_WIDTH + LEFT_GAP, to_row * SQ_HEIGHT + TOP_GAP, SQ_WIDTH, SQ_HEIGHT))

def drawLegalMoveCircles(board, selected_square):
    WIN_ALPHA.fill((0, 0, 0, 0))
    legal_moves = board.legal_moves
    for move in legal_moves:
        if move.from_square == chess.parse_square(selected_square):
            row = chess.square_rank(move.to_square)
            col = chess.square_file(move.to_square)
            if FLIP_BOARD:
                row = 7 - row
            else:
                col = 7 - col
            pygame.draw.circle(WIN_ALPHA, (0, 0, 0, 50), (col * SQ_WIDTH + LEFT_GAP + SQ_WIDTH//2, row * SQ_HEIGHT + TOP_GAP + SQ_HEIGHT//2), SQ_HEIGHT//6)
    WIN.blit(WIN_ALPHA, (0, 0))

def checkIfPawnPromotionMove(move, piece):
    # default promotion to queen
    if str(piece) in "pP":
        if chess.square_rank(move.to_square) in [0, 7]:
            move.promotion = chess.QUEEN
    return move

def drawPieces(board, dragged_piece=None, dragged_piece_pos=None):
    fen = board.fen().split()[0]
    board_row = 0
    board_col = 0
    for char in fen:
        if char == '/':
            board_row += 1
            board_col = 0
        elif char.isdigit():
            board_col += int(char)
        else:
            screen_row = 7 - board_row
            screen_col = 7 - board_col 
            if FLIP_BOARD:
                screen_row = board_row
                screen_col = board_col
            compare_col = 7 - screen_col if not FLIP_BOARD else screen_col
            compare_row = 7 - screen_row if FLIP_BOARD else screen_row
            if not (dragged_piece_pos and 
                   compare_row == dragged_piece_pos[0] and 
                   compare_col == dragged_piece_pos[1]):
                img = piece_images[char]
                img = pygame.transform.scale(img, (SQ_WIDTH, SQ_HEIGHT))
                WIN.blit(img, (screen_col * SQ_WIDTH + LEFT_GAP, screen_row * SQ_HEIGHT + TOP_GAP))
            board_col += 1
    
    if dragged_piece:
        col = chess.square_file(chess.parse_square(selected_square))
        row = chess.square_rank(chess.parse_square(selected_square))
        if FLIP_BOARD:
            row = 7 - row
        else:
            col = 7 - col
        pygame.draw.rect(WIN, (125, 255, 125), (col * SQ_WIDTH + LEFT_GAP, row * SQ_HEIGHT + TOP_GAP, SQ_WIDTH, SQ_HEIGHT))
        mouseX, mouseY = pygame.mouse.get_pos()
        img = piece_images[str(dragged_piece)]
        WIN.blit(img, (mouseX - SQ_WIDTH//2, mouseY - SQ_HEIGHT//2))

def drawBoard():
    SQ_WIDTH = WIDTH // 8
    SQ_HEIGHT = HEIGHT // 8
    for row in range(8):
        for col in range(8):
            if (row + col) % 2 == 0:
                pygame.draw.rect(WIN, WHITE, (col * SQ_WIDTH + LEFT_GAP, row * SQ_HEIGHT + TOP_GAP, SQ_WIDTH, SQ_HEIGHT))
            else:
                pygame.draw.rect(WIN, BLACK, (col * SQ_WIDTH + LEFT_GAP, row * SQ_HEIGHT + TOP_GAP, SQ_WIDTH, SQ_HEIGHT))

def getSquareFromCoords(x, y):
    col = (x - LEFT_GAP) // SQ_WIDTH
    row = 7 - (y - TOP_GAP) // SQ_HEIGHT
    if 0 <= row <= 7 and 0 <= col <= 7:
        if FLIP_BOARD:
            row = 7 - row
        else:
            row = row
            col = 7 - col
        rows = '87654321'
        cols = 'abcdefgh'
        return cols[col] + rows[row]
    return None

# GET THE PLAYING_AS TO MOVE
if PLAYING_AS == 'white':
    engineToMove = False 
else:
    engineToMove = True

# BUTTONS 
BTN_GAP = 10
undoMoveButton = Button(LEFT_GAP, HEIGHT+TOP_GAP+BTN_GAP, 70, 30, (100, 200, 100), "Undo", "monsterrat", (255, 255, 255), 30)
printFENButton = Button(LEFT_GAP+80, HEIGHT+TOP_GAP+BTN_GAP, 70, 30, (255, 125, 125), "Fen", "monsterrat", (255, 255, 255), 30)

# CHESS 
board = chess.Board() if not FEN else chess.Board(FEN)
selected_piece = None
selected_square = None
dragging = False
engine_thinking = False
engine_thread = None 

# GAME LOOP
run = True
while run:
    for event in pygame.event.get():
        if event.type == QUIT:
            run = False
        if event.type == MOUSEBUTTONDOWN:
            if undoMoveButton.clicked():
                if len(board.move_stack) > 2 and not engineToMove and not game_over:
                    board.pop()
                    board.pop()
            if printFENButton.clicked():
                fen = board.fen()
                print(fen)
        if event.type == MOUSEBUTTONDOWN and event.button == 1 and not engine_thinking and not game_over:
            mouseX, mouseY = event.pos
            square = getSquareFromCoords(mouseX, mouseY)
            if square:
                piece = board.piece_at(chess.parse_square(square))
                if piece:
                    if (PLAYING_AS=='white' and piece.color == chess.WHITE) or (PLAYING_AS=='black' and piece.color == chess.BLACK):
                        selected_piece = piece
                        selected_square = square
                        dragging = True 
        if event.type == MOUSEBUTTONUP and event.button == 1 and not game_over:
            if dragging:
                WIN_ALPHA.fill((0, 0, 0, 0))
                mouseX, mouseY = event.pos
                target_square = getSquareFromCoords(mouseX, mouseY)
                if target_square and selected_square:
                    if target_square == selected_square:
                        dragging = False
                        continue
                    move = chess.Move.from_uci(selected_square + target_square)
                    move = checkIfPawnPromotionMove(move, selected_piece)
                    if move in board.legal_moves:
                        if board.turn == chess.WHITE:
                            time_control[0] += time_control[2]
                        else:
                            time_control[1] += time_control[2]
                        board.push(move)
                        engineToMove = True
                selected_piece = None
                selected_square = None
                dragging = False

    WIN.fill(BACKGROUND)

    # Draw the time control
    updateTimeControl(board)
    drawTimeControl()

    # Draw the current evaluation 
    drawTitleTop(board)
    
    # Draw the board and pieces
    drawBoard()
    
    # Draw squares over the last move
    drawSquaresOverLastMove(board)

    # Draw the selected piece and legal moves
    if dragging:
        row = chess.square_rank(chess.parse_square(selected_square))
        col = chess.square_file(chess.parse_square(selected_square))
        drawPieces(board, selected_piece, (row, col))
        drawLegalMoveCircles(board, selected_square)
    else:
        drawPieces(board)

    # Engine move
    if engineToMove and not board.is_game_over() and not engine_thinking and not game_over:
        engine_thinking = True 
        engine_thread = threading.Thread(target=engineMoveThread, args=(board,))
        engine_thread.start()
    
    # Draw the buttons
    undoMoveButton.draw()
    printFENButton.draw()

    pygame.display.update()

pygame.quit()
quit()