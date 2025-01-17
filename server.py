from hclib import newboard, stringboard, apply_move, moves, move_t
import sys
import sqlite3
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
from urllib.parse import urlparse, parse_qs
from datetime import datetime

# Default FEN string
DEFAULT_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

# Global game time in seconds
GAME_TIME = 300

# Database setup function
def initialize_database():
    conn = sqlite3.connect('hostage_chess.db')
    cursor = conn.cursor()

    # Create games table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS games (
            GAME_NO INTEGER PRIMARY KEY AUTOINCREMENT,
            WHITE_HANDLE TEXT NOT NULL,
            BLACK_HANDLE TEXT,
            RESULT TEXT
        )
    ''')

    # Create boards table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS boards (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            GAME_NO INTEGER NOT NULL,
            TURN_NO INTEGER NOT NULL,
            TURN TEXT NOT NULL,
            BOARD TEXT NOT NULL,
            REAL_TIME TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            WHITE_TIME INTEGER NOT NULL,
            BLACK_TIME INTEGER NOT NULL,
            FOREIGN KEY (GAME_NO) REFERENCES games(GAME_NO)
        )
    ''')
    
    conn.commit()
    conn.close()

# HTTP Request Handler
class MyHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        parsed_path = urlparse(self.path)
        query_params = parse_qs(parsed_path.query)

        if parsed_path.path == '/index.html' or parsed_path.path == '/':
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(b"""
                <html>
                <head><title>Hostage Chess</title></head>
                <body>
                    <h1>Welcome to Hostage Chess</h1>
                    <form action="/login.html" method="get">
                        <input type="text" name="handle" placeholder="Enter your handle" required>
                        <button type="submit">Start Game</button>
                    </form>
                    <a href="/history.html">Game History</a>
                </body>
                </html>
            """)

        elif parsed_path.path == '/login.html':
            handle = query_params.get('handle', [None])[0]

            if handle:
                conn = sqlite3.connect('hostage_chess.db')
                cursor = conn.cursor()

                # Check if there's a game with an empty BLACK_HANDLE
                cursor.execute("SELECT GAME_NO FROM games WHERE BLACK_HANDLE IS NULL")
                row = cursor.fetchone()

                if row:
                    # Join an existing game
                    game_no = row[0]
                    cursor.execute("UPDATE games SET BLACK_HANDLE = ? WHERE GAME_NO = ?", (handle, game_no))
                    cursor.execute("""
                        INSERT INTO boards (GAME_NO, TURN_NO, TURN, BOARD, WHITE_TIME, BLACK_TIME)
                        VALUES (?, 1, 'White', ?, ?, ?)
                    """, (game_no, DEFAULT_FEN, GAME_TIME, GAME_TIME))
                    conn.commit()
                    conn.close()

                    # Redirect to /player.html
                    self.send_response(200)
                    self.send_header("Content-Type", "text/html")
                    self.end_headers()
                    self.wfile.write(f"""
                        <html>
                        <head>
                            <title>Hostage Chess</title>
                            <script>
                                const gameNo = {game_no};
                                const turnNo = 1;
                                window.location.href = `/player.html?game_no=${{gameNo}}&turn_no=${{turnNo}}`;
                            </script>
                        </head>
                        <body>
                            <h1>You have joined Game #{game_no}. Redirecting...</h1>
                        </body>
                        </html>
                    """.encode())
                else:
                    # Create a new game
                    cursor.execute("SELECT COALESCE(MAX(GAME_NO), 0) + 1 FROM games")
                    game_no = cursor.fetchone()[0]
                    cursor.execute("INSERT INTO games (GAME_NO, WHITE_HANDLE) VALUES (?, ?)", (game_no, handle))
                    conn.commit()
                    conn.close()

                    # Wait for opponent
                    self.send_response(200)
                    self.send_header("Content-Type", "text/html")
                    self.end_headers()
                    self.wfile.write(f"""
                        <html>
                        <head>
                            <title>Hostage Chess</title>
                            <script>
                                const gameNo = {game_no};
                                function checkOpponent() {{
                                    fetch(`/check_opponent.html?game_no=${{gameNo}}`)
                                        .then(response => response.json())
                                        .then(data => {{
                                            if (data.opponentFound) {{
                                                window.location.href = `/player.html?game_no=${{gameNo}}&turn_no=1`;
                                            }}
                                        }});
                                }}
                                setInterval(checkOpponent, 1000);
                            </script>
                        </head>
                        <body>
                            <h1>Waiting for an opponent to join Game #{game_no}</h1>
                        </body>
                        </html>
                    """.encode())
        elif parsed_path.path == '/history.html':
            # Fetch game history
            conn = sqlite3.connect('hostage_chess.db')
            cursor = conn.cursor()
            cursor.execute("SELECT GAME_NO, WHITE_HANDLE, BLACK_HANDLE, RESULT FROM games")
            games = cursor.fetchall()
            conn.close()

            # Generate HTML for game history
            games_html = "<table border='1' style='width: 100%; text-align: left;'>"
            games_html += """
                <tr>
                    <th>Game No</th>
                    <th>White Handle</th>
                    <th>Black Handle</th>
                    <th>Result</th>
                </tr>
            """
            for game_no, white_handle, black_handle, result in games:
                games_html += f"""
                    <tr>
                        <td><a href="/gamelog.html?game_no={game_no}">Game #{game_no}</a></td>
                        <td>{white_handle}</td>
                        <td>{black_handle}</td>
                        <td>{result if result else "In Progress"}</td>
                    </tr>
                """
            games_html += "</table>"

            # Send response
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(f"""
                <html>
                <head><title>Game History</title></head>
                <body>
                    <h1>Game History</h1>
                    {games_html}
                    <br>
                    <a href="/index.html">Back to Home</a>
                </body>
                </html>
            """.encode())
        
        elif parsed_path.path == '/gamelog.html':
            game_no = query_params.get('game_no', [None])[0]

            if game_no:
                conn = sqlite3.connect('hostage_chess.db')
                cursor = conn.cursor()

                # Fetch all boards for the given game_no
                cursor.execute("""
                    SELECT TURN_NO, TURN, BOARD
                    FROM boards
                    WHERE GAME_NO = ?
                    ORDER BY TURN_NO
                """, (game_no,))
                boards = cursor.fetchall()

                # Fetch the result of the game
                cursor.execute("""
                    SELECT RESULT FROM games WHERE GAME_NO = ?
                """, (game_no,))
                result = cursor.fetchone()
                conn.close()

                if boards:
                    self.send_response(200)
                    self.send_header("Content-Type", "text/html")
                    self.end_headers()

                    # Generate the HTML content
                    boards_html = ""
                    for turn_no, turn, board in boards:
                        boards_html += f"""
                            <div style="margin-bottom: 20px;">
                                <h3>Turn {turn_no} - {turn}'s Move</h3>
                                <div id="board-{turn_no}" style="width: 400px;"></div>
                                <script>
                                    var board = Chessboard('board-{turn_no}', {{
                                        position: '{board}',
                                        draggable: false,
                                        pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{{piece}}.png'
                                    }});
                                </script>
                            </div>
                        """

                    # Add the game result and navigation links
                    result_html = f"<h2>Result: {result[0] if result else 'Pending'}</h2>"
                    navigation_links = """
                        <a href="/history.html">Back to Game History</a>
                        <br>
                        <a href="/index.html">Back to Home</a>
                    """

                    # Combine everything into the final HTML response
                    self.wfile.write(f"""
                        <html>
                        <head>
                            <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
                            <link rel="stylesheet" href="https://unpkg.com/@chrisoakman/chessboardjs@1.0.0/dist/chessboard-1.0.0.min.css">
                            <script src="https://unpkg.com/@chrisoakman/chessboardjs@1.0.0/dist/chessboard-1.0.0.min.js"></script>
                        </head>
                        <body>
                            <h1>Game #{game_no} - Log</h1>
                            {boards_html}
                            {result_html}
                            {navigation_links}
                        </body>
                        </html>
                    """.encode())
                else:
                    self.send_error(404, "No logs found for this game.")
            else:
                self.send_error(400, "Missing game_no parameter.")


        elif parsed_path.path == '/player.html':
            game_no = query_params.get('game_no', [None])[0]
            turn_no = query_params.get('turn_no', [None])[0]

            if game_no and turn_no:
                conn = sqlite3.connect('hostage_chess.db')
                cursor = conn.cursor()

                cursor.execute("""
                    SELECT TURN, BOARD, WHITE_TIME, BLACK_TIME
                    FROM boards
                    WHERE GAME_NO = ? AND TURN_NO = ?
                """, (game_no, turn_no))
                row = cursor.fetchone()
                conn.close()

                if row:
                    turn, board, white_time, black_time = row
                    self.send_response(200)
                    self.send_header("Content-Type", "text/html")
                    self.end_headers()
                    content = f"""
                        <!DOCTYPE html>
                        <html>
                        <head>
                            <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
                            <link rel="stylesheet" href="https://unpkg.com/@chrisoakman/chessboardjs@1.0.0/dist/chessboard-1.0.0.min.css">
                            <script src="https://unpkg.com/@chrisoakman/chessboardjs@1.0.0/dist/chessboard-1.0.0.min.js"></script>
                        </head>
                        <body>
                            <h1>Game #{game_no} - Turn {turn}</h1>
                            <div id="board" style="width: 400px;"></div>
                            <form action="/opponent.html" method="post">
                                <input type="hidden" name="game_no" value="{game_no}">
                                <input type="hidden" name="turn_no" value="{turn_no}">
                                <textarea name="board">{board}</textarea>
                                <button type="submit">Done</button>
                            </form>
                            <script>
                                var board = Chessboard('board', {{
                                    position: '{board}',
                                    draggable: true,
                                    pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{{piece}}.png'
                                }});
                                document.querySelector('form').onsubmit = function() {{
                                    document.querySelector('textarea[name="board"]').value = board.fen();
                                }};
                            </script>
                        </body>
                        </html>
                    """
                    self.wfile.write(content.encode('utf-8'))
                else:
                    self.send_error(404, "Turn not found")
            else:
                self.send_error(400, "Missing game_no or turn_no")
        
        

    def do_POST(self):
        parsed_path = urlparse(self.path)

        if parsed_path.path == '/opponent.html':
            try:
                content_length = int(self.headers['Content-Length'])
                post_data = self.rfile.read(content_length)
                params = parse_qs(post_data.decode('utf-8'))

                game_no = params.get('game_no', [None])[0]
                turn_no = params.get('turn_no', [None])[0]
                board = params.get('board', [None])[0]

                if not (game_no and turn_no and board):
                    raise ValueError("Missing required parameters in POST request to /opponent.html")

                conn = sqlite3.connect('hostage_chess.db')
                cursor = conn.cursor()

                cursor.execute("""
                    SELECT TURN, REAL_TIME, WHITE_TIME, BLACK_TIME
                    FROM boards
                    WHERE GAME_NO = ? AND TURN_NO = ?
                """, (game_no, turn_no))
                row = cursor.fetchone()

                if not row:
                    raise ValueError(f"No board data found for game_no={game_no}, turn_no={turn_no}")

                current_turn, real_time, white_time, black_time = row

                current_time = datetime.now()
                real_time = datetime.strptime(real_time, '%Y-%m-%d %H:%M:%S')
                elapsed_time = (current_time - real_time).seconds

                if current_turn == 'White':
                    white_time -= elapsed_time
                else:
                    black_time -= elapsed_time

                # if white_time <= 0 or black_time <= 0:
                #     result = "White wins" if black_time <= 0 else "Black wins"
                #     cursor.execute("UPDATE games SET RESULT = ? WHERE GAME_NO = ?", (result, game_no))
                #     conn.commit()
                #     conn.close()

                #     self.send_response(200)
                #     self.send_header("Content-Type", "text/html")
                #     self.end_headers()
                #     self.wfile.write(f"""
                #         <html>
                #         <head><title>Game Over</title></head>
                #         <body>
                #             <h1>{result}</h1>
                #             <a href="/history.html">View Game History</a>
                #             <a href="/index.html">Start New Game</a>
                #         </body>
                #         </html>
                #     """.encode())
                #     return

                new_turn = 'Black' if current_turn == 'White' else 'White'
                new_turn_no = int(turn_no) + 1
                cursor.execute("""
                    INSERT INTO boards (GAME_NO, TURN_NO, TURN, BOARD, REAL_TIME, WHITE_TIME, BLACK_TIME)
                    VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP, ?, ?)
                """, (game_no, new_turn_no, new_turn, board, white_time, black_time))
                conn.commit()
                conn.close()

                self.send_response(200)
                self.send_header("Content-Type", "text/html")
                self.end_headers()
                self.wfile.write(f"""
                    <html>
                    <head>
                        <script>
                            window.location.href = "/player.html?game_no={game_no}&turn_no={new_turn_no}";
                        </script>
                    </head>
                    <body>
                        <h1>Updating game state...</h1>
                    </body>
                    </html>
                """.encode())

            except Exception as e:
                print(f"Error in /opponent.html: {e}")
                self.send_response(500)
                self.send_header("Content-Type", "text/plain")
                self.end_headers()
                self.wfile.write(f"Error: {e}".encode())
        else:
            self.send_error(501, "Unsupported method")



if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 server.py <port>")
        sys.exit(1)

    port = int(sys.argv[1])
    initialize_database()

    print(f"Starting server on port {port}...")
    httpd = HTTPServer(('0.0.0.0', port), MyHandler)
    httpd.serve_forever()