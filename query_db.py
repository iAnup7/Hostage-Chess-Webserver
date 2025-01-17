import sqlite3

def query_database():
    conn = sqlite3.connect('hostage_chess.db')
    cursor = conn.cursor()

    # Query to list tables
    cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
    tables = cursor.fetchall()
    print("Tables in the database:")
    for table in tables:
        print(table[0])

    # Query games table
    print("\nGames Table:")
    cursor.execute("SELECT * FROM games")
    games = cursor.fetchall()
    for row in games:
        print(row)

    # Query boards table
    print("\nBoards Table:")
    cursor.execute("SELECT * FROM boards")
    boards = cursor.fetchall()
    for row in boards:
        print(row)

    conn.close()

if __name__ == '__main__':
    query_database()